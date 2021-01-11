#include "obj_core.h"

static obj_conn_queue_item_t *obj_conn_queue_item_freelist;
static pthread_mutex_t obj_conn_queue_item_freelist_lock;
static pthread_mutex_t obj_conn_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile obj_bool_t obj_conn_allow_new_conns = true;     /* allow new connection flag */
static struct event obj_conn_maxconnsevent;

/* globals */
int obj_conn_max_fds;                       /* maximum fds */
obj_conn_t **obj_conn_conns;                /* connection array */
obj_conn_t *obj_conn_listen_conn = NULL;

void obj_conn_queue_item_freelist_init() {
    obj_conn_queue_item_freelist = NULL;
    pthread_mutex_init(&obj_conn_queue_item_freelist_lock, NULL);
}

/* initializes a connection queue */
void obj_conn_queue_init(obj_conn_queue_t *cq) {
    pthread_mutex_init(&cq->lock, NULL);
    cq->head = NULL;
    cq->tail = NULL;
}

/* pop an item in a connection queue */
obj_conn_queue_item_t *obj_conn_queue_pop(obj_conn_queue_t *cq) {
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
void obj_conn_queue_free_item(obj_conn_queue_item_t *item) {
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

/* drive the state machine */
static void obj_drive_machine(obj_conn_t *c) {

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


void obj_conn_dispatch_conn_new(int sfd, obj_conn_state_t init_state, int event_flags) {
    obj_conn_queue_item_t *item = obj_conn_queue_new_item();
    char buf[1];
    if (item == NULL) {
        close(sfd);
        fprintf(stderr, "failed to allocate memory for connection object\n");
        return;
    }
    /* round robin */
    int tid = (obj_thread_last_thread + 1) % obj_settings.num_threads;
    obj_thread_libevent_thread_t *thread = obj_thread_threads + tid;
    obj_thread_last_thread = tid;
    /* init item */
    item->sfd = sfd;
    item->init_state = init_state;
    item->event_flags = event_flags;

    obj_conn_queue_push(thread->new_conn_queue, item);
    buf[0] = 'c';
    /* nofify worker thread */
    if (write(thread->notify_send_fd, buf, 1) != 1) {
        fprintf(stderr, "writing to worker thread notify pipe error\n");
    }
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
        if (obj_settings.verbose > 0) {
            fprintf(stderr, "event fd doesn't match conn fd\n");
        }
        obj_conn_close(c);
        return;
    }
    obj_drive_machine(c);
    return;
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
        obj_drive_machine(c);
    }
    
}

void obj_conn_close(obj_conn_t *c) {
    obj_assert(c != NULL);
    /* delete the event, the socket and the conn */
    event_del(&c->event);
    if (obj_settings.verbose > 1) {
        fprintf(stderr, "<%d connection closed\n", c->sfd);
    }
    /* TODO force release of read buffer */
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


