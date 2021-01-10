#include "obj_core.h"

#define OBJ_CONN_PER_SLICE 100
#define OBJ_CONN_TIMEOUT_MSG_SIZE (1 + sizeof(int))

static obj_conn_queue_item_t *obj_conn_queue_item_freelist;
static pthread_mutex_t obj_conn_queue_item_freelist_lock;

static int obj_conn_max_fds;                                    /* maximum fds */
static volatile obj_bool_t obj_conn_run_conn_timeout_thread;    /* timeout thread running flag */
static pthread_t obj_conn_timeout_tid;                          /* timeout thread id */
static volatile obj_bool_t obj_conn_allow_new_conns = true;     /* allow new connection flag */
static struct event obj_conn_maxconnsevent;

/* globals */
obj_conn_t **obj_conn_conns;             /* connection array */
pthread_mutex_t obj_conn_lock = PTHREAD_MUTEX_INITIALIZER;
obj_conn_t *obj_conn_listen_conn = NULL;

/* initializes a connection queue */
static void obj_conn_queue_init(obj_conn_queue_t *cq) {
    pthread_mutex_init(&cq->lock, NULL);
    cq->head = NULL;
    cq->tail = NULL;
}

/* pop an item in a connection queue */
static obj_conn_queue_item_t *obj_conn_queue_pop(obj_conn_queue_t *cq) {
    obj_conn_queue_item_t *item;
    pthread_mutex_lock(&cq->lock);
    item = cq->head;
    if (item != NULL) {
        cq->head = item->next;
        if (cq->head == NULL) {
            cq->tail = NULL;
        }
    }
    pthread_mutex_unlock(&cq->lock);
    return item;
}

/* push an item to a connection queue */
static void obj_conn_queue_push(obj_conn_queue_t *cq, obj_conn_queue_item_t *item) {
    item->next = NULL;
    pthread_mutex_lock(&cq->lock);
    if (cq->tail == NULL) {
        cq->head = item;
    } else {
        cq->tail->next = item;
    }
    cq->tail = item;
    pthread_mutex_unlock(&cq->lock);
}

/* return a new connection queue item */
static obj_conn_queue_item_t *obj_conn_queue_new_item() {
    obj_conn_queue_item_t *item = NULL;
    pthread_mutex_lock(&obj_conn_queue_item_freelist_lock);
    if (obj_conn_queue_item_freelist) {
        item = obj_conn_queue_item_freelist;
        obj_conn_queue_item_freelist = item->next;
    }
    pthread_mutex_unlock(&obj_conn_queue_item_freelist_lock);
    if (item == NULL) {
        int i;
        item = obj_alloc(sizeof(obj_conn_queue_item_t) * OBJ_CONN_QUEUE_ITEMS_PER_ALLOC);
        if (item == NULL) {
            return NULL;
        }
        /* link all the new allocated items except the first one */
        for (i = 2; i < OBJ_CONN_QUEUE_ITEMS_PER_ALLOC; i++) {
            item[i - 1].next = &item[i];
        }
        pthread_mutex_lock(&obj_conn_queue_item_freelist_lock);
        item[OBJ_CONN_QUEUE_ITEMS_PER_ALLOC - 1].next = obj_conn_queue_item_freelist;
        obj_conn_queue_item_freelist = &item[1];
        pthread_mutex_unlock(&obj_conn_queue_item_freelist_lock);
    }
    return item;
}

/* free a connection queue item */
static void obj_conn_queue_free_item(obj_conn_queue_item_t *item) {
    pthread_mutex_lock(&obj_conn_queue_item_freelist_lock);
    item->next = obj_conn_queue_item_freelist;
    obj_conn_queue_item_freelist = item;
    pthread_mutex_unlock(&obj_conn_queue_item_freelist_lock);
}


/* ---------- connection ---------- */
static void obj_conn_maxconns_handler(const evutil_socket_t fd, const short which, void *arg) {
    struct timeval t = {.tv_sec = 0, .tv_usec = 10000};
    if (fd == -42 || !obj_conn_allow_new_conns) {
        /* reschedule in 10ms if we need to keep polling */
        evtimer_set(&obj_conn_maxconnsevent, obj_conn_maxconns_handler, 0);
        event_base_set(obj_main_base, &obj_conn_maxconnsevent);
        evtimer_add(&obj_conn_maxconnsevent, &t);
    } else {
        evtimer_del(&obj_conn_maxconnsevent);
        obj_conn_accept_new_conns(true);
    }
}

/* do some cleanup jobs */
static void obj_conn_cleanup(obj_conn_t *c) {
    obj_assert(c != NULL);
}

/* free a connection */
static void obj_conn_free(obj_conn_t *c) {
    if (c) {
        obj_conn_conns[c->sfd] = NULL;
        obj_free(c);
    }
}

/* kick out idle threads */
static void *obj_conn_timeout_thread(void *arg) {
    int i;
    obj_conn_t *c;
    char buf[OBJ_CONN_TIMEOUT_MSG_SIZE];
    obj_rel_time_t oldest_last_cmd;
    int sleep_time;
    int sleep_slice = obj_conn_max_fds / OBJ_CONN_PER_SLICE;
    if (sleep_slice == 0) {
        sleep_slice = OBJ_CONN_PER_SLICE;
    }
    useconds_t timeslice = 1000000 / sleep_slice;
    while (obj_conn_run_conn_timeout_thread) {
        oldest_last_cmd = obj_rel_current_time;
        for (i = 0; i < obj_conn_max_fds; i++) {
            /* sleep */
            if ((i % OBJ_CONN_PER_SLICE) == 0) {
                usleep(timeslice);
            }
            if (!obj_conn_conns[i]) {
                continue;
            }
            c = obj_conn_conns[i];
            /* check connection state */
            if (c->state !=) {
                continue;
            }
            if ((obj_rel_current_time - c->last_cmd_time) > obj_settings.idle_timeout) {
                buf[0] = 't';
                obj_memcpy(&buf[1], &i, sizeof(int));
                if (write(c->thread->notify_send_fd, buf, OBJ_CONN_TIMEOUT_MSG_SIZE) != OBJ_CONN_TIMEOUT_MSG_SIZE) {
                    fprintf(stderr, "failed to write timeout message to notify pipe\n");
                }
            } else {
                if (c->last_cmd_time < oldest_last_cmd) {
                    oldest_last_cmd = c->last_cmd_time;
                }
            }
        }
        /* soonest we could have another connection timeout */
        sleep_time = obj_settings.idle_timeout - (obj_rel_current_time - oldest_last_cmd) + 1;
        if (sleep_time <= 0) {
            sleep_time = 1;
        }
        usleep((useconds_t) sleep_time * 1000000);
    }
    return NULL;
}

/* initialize the connection array */
void obj_conn_conns_init() {
    int next_fd = dup(1);
    if (next_fd < 0) {
        fprintf(stderr, "failed to duplicate file descriptor\n");
        exit(1);
    }
    int headroom = 10;              /* extra room for unexpected open fds */
    struct rlimit rl;
    obj_conn_max_fds = obj_settings.maxconns + headroom + next_fd;
    /* get the actual highest fd if possible */
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        obj_conn_max_fds = rl.rlim_max;
    } else {
        fprintf(stderr, "failed to query maximum file descriptor\n");
    }
    close(next_fd);
    if ((obj_conn_conns = calloc(obj_conn_max_fds, sizeof(obj_conn_t *))) == NULL) {
        fprintf(stderr, "failed to allocate connection structures\n");
        exit(1);
    }
}

obj_conn_t *obj_conn_new(const int sfd, obj_conn_state_t init_state, const short event_flags, struct event_base *base) {
    obj_conn_t *c;
    obj_assert(sfd >= 0 && sfd < obj_conn_max_fds);
}

void obj_conn_accept_new_conns(const obj_bool_t do_accept) {
    pthread_mutex_lock(&obj_conn_lock);
    obj_conn_do_accept_new_conns(do_accept);
    pthread_mutex_unlock(&obj_conn_lock);
}

void obj_conn_do_accept_new_conns(const obj_bool_t do_accept) {
    obj_conn_t *next;
    for (next = obj_conn_listen_conn; next; next = next->next) {
        if (do_accept) {
            obj_conn_update_event(next, EV_READ | EV_PERSIST);
            if (listen(next->sfd, obj_settings.backlog) != 0) {
                fprintf(stderr, "listen error %d\n", next->sfd);
            }
        } else {
            obj_conn_update_event(next, 0);
            if (listen(next->sfd, 0) != 0) {
                fprintf(stderr, "listen error %d\n", next->sfd);
            }
        }
    }
    if (do_accept) {

    } else {
        obj_conn_allow_new_conns = false;
        obj_conn_maxconns_handler(-42, 0, 0);
    }
}

obj_bool_t obj_conn_update_event(obj_conn_t *c, const int new_flags) {
    obj_assert(c != NULL);
    struct event_base *base = c->event.ev_base;
    if (c->event_flags == new_flags) {
        return true;
    }
    if (event_del(&c->event) == -1) {
        return false;
    }
    event_set(&c->event, c->sfd, new_flags, obj_conn_event_handler, (void *)c);
    event_base_set(base, &c->event);
    c->event_flags = new_flags;
    if (event_add(&c->event, 0) == -1) {
        return false;
    }
    return true;
}

void obj_conn_event_handler(const evutil_socket_t fd, const short which, void *arg) {
    obj_conn_t *c;
    c = (obj_conn_t *)arg;
    obj_assert(c != NULL);
    c->which = which;
    if (fd != c->sfd) {
        /* obj_conn */
    }
}

/* close idle connection */
void obj_conn_close_idle(obj_conn_t *c) {
    if (obj_settings.idle_timeout > 0 && (obj_rel_current_time - c->last_cmd_time) > obj_settings.idle_timeout) {
        /* a connection timeout */
        if (c->state != ) {
            if (obj_settings.verbose > 1) {
                fprintf(stderr, "fd %d wants to timeout, but isn't in correct state\n", c->sfd);
            }
            return;
        }
        if (obj_settings.verbose > 1) {
            fprintf(stderr, "closing idle fd %d\n", c->sfd);
        }
        obj_conn_set_state(c, OBJ_CONN_CLOSING);
        /* drive the state machine */
        drive_machine(c);
    }
    
}

void obj_conn_close(obj_conn_t *c) {
    obj_assert(c != NULL);
    event_del(&c->event);
    obj_conn_cleanup(c);
    obj_conn_set_state(c, OBJ_CONN_CLOSED);
    close(c->sfd);
    pthread_mutex_lock(&obj_conn_lock);
    obj_conn_allow_new_conns = true;
    pthread_mutex_unlock(&obj_conn_lock);
}

void obj_conn_close_all() {
    int i;
    for (i = 0; i < obj_conn_max_fds; i++) {
        if (obj_conn_conns[i] && obj_conn_conns[i]->state != OBJ_CONN_CLOSED) {
            obj_conn_close(obj_conn_conns[i]);
        }
    }
}

/* TODO set a connection state */
void obj_conn_set_state(obj_conn_t *c, obj_conn_state_t state) {
    obj_assert(c != NULL);
    if (state != c->state) {
        c->state = state;
    }
}

obj_bool_t obj_conn_start_timeout_thread() {
    int ret;
    if (obj_settings.idle_timeout == 0) {
        return false;
    }
    obj_conn_run_conn_timeout_thread = true;
    if ((ret = pthread_create(&obj_conn_timeout_tid, NULL, obj_conn_timeout_thread, NULL)) != 0) {
        fprintf(stderr, "can't create idle connection timeout thread: %s\n", STRERROR(ret));
        return false;
    }
    return true;
}

obj_bool_t obj_conn_stop_timeout_thread() {
    if (!obj_conn_run_conn_timeout_thread) {
        return false;
    }
    obj_conn_run_conn_timeout_thread = false;
    pthread_join(obj_conn_timeout_tid, NULL);
    return 0;
}
