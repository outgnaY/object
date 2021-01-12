#include "obj_core.h"

#define OBJ_CONN_PER_SLICE 100
#define OBJ_CONN_TIMEOUT_MSG_SIZE (1 + sizeof(int))

static volatile obj_bool_t obj_thread_run_conn_timeout_thread;          /* timeout thread running flag */
static pthread_t obj_thread_conn_timeout_tid;                           /* timeout thread id */
/* lock to cause worker threads to hang up after being woken */
static pthread_mutex_t obj_thread_worker_hang_lock;
/* thread init */
static int obj_thread_init_count = 0;
static pthread_mutex_t obj_thread_init_lock;
static pthread_cond_t obj_thread_init_cond;

int obj_thread_last_thread = -1;
/* worker threads */
obj_thread_libevent_thread_t *obj_thread_threads;

static void obj_thread_wait_for_thread_registration(int nthreads);
static void obj_thread_register_thread_initialized();
static void obj_thread_setup_thread(obj_thread_libevent_thread_t *this);
static void obj_thread_create_worker(void *(*func)(void *), void *arg);
static void *obj_thread_worker_mainloop(void *arg);
static void obj_thread_libevent_process(evutil_socket_t fd, short which, void *arg);
static void *obj_thread_conn_timeout_thread(void *arg);




/* wait for thread registration */
static void obj_thread_wait_for_thread_registration(int nthreads) {
    while (obj_thread_init_count < nthreads) {
        pthread_cond_wait(&obj_thread_init_cond, &obj_thread_init_lock);
    }
    fprintf(stderr, "wait ok\n");
}

static void obj_thread_register_thread_initialized() {
    pthread_mutex_lock(&obj_thread_init_lock);
    obj_thread_init_count++;
    pthread_cond_signal(&obj_thread_init_cond);
    pthread_mutex_unlock(&obj_thread_init_lock);
    /* force worker threads to pile up */
    pthread_mutex_lock(&obj_thread_worker_hang_lock);
    pthread_mutex_unlock(&obj_thread_worker_hang_lock);
}

/* setup a worker thread */
static void obj_thread_setup_thread(obj_thread_libevent_thread_t *this) {
#if defined(LIBEVENT_VERSION_NUMBER) && LIBEVENT_VERSION_NUMBER >= 0x2000101
    struct event_config *ev_config;
    ev_config = event_config_new();
    event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);
    this->base = event_base_new_with_config(ev_config);
    event_config_free(ev_config);
#else
    this->base = event_init();
#endif
    if (!this->base) {
        fprintf(stderr, "can't allocate event base\n");
        exit(EXIT_FAILURE);
    }
    /* listen for notifications from other threads */
    event_set(&this->notify_event, this->notify_receive_fd, EV_READ | EV_PERSIST, obj_thread_libevent_process, this);
    event_base_set(this->base, &this->notify_event);
    if (event_add(&this->notify_event, 0) == -1) {
        fprintf(stderr, "can't monitor libevent notify pipe\n");
        exit(EXIT_FAILURE);
    }
    this->new_conn_queue = obj_alloc(sizeof(obj_conn_queue_t));
    if (this->new_conn_queue == NULL) {
        fprintf(stderr, "failed to allocate memory for connection queue\n");
        exit(EXIT_FAILURE);
    }
    obj_conn_queue_init(this->new_conn_queue);
}

/* create a worker thread */
static void obj_thread_create_worker(void *(*func)(void *), void *arg) {
    pthread_attr_t attr;
    int ret;
    pthread_attr_init(&attr);
    if ((ret = pthread_create(&((obj_thread_libevent_thread_t *)arg)->thread_id, &attr, func, arg)) != 0) {
        fprintf(stderr, "can't create thread: %s\n", OBJ_STRERROR(ret));
        exit(1);
    }
}

/* worker thread mainloop */
static void *obj_thread_worker_mainloop(void *arg) {
    obj_thread_libevent_thread_t *this = (obj_thread_libevent_thread_t *)arg;
    obj_thread_register_thread_initialized();
    event_base_loop(this->base, 0);
    obj_thread_register_thread_initialized();
    event_base_free(this->base);
    return NULL;
}

static void obj_thread_libevent_process(evutil_socket_t fd, short which, void *arg) {
    obj_thread_libevent_thread_t *this = (obj_thread_libevent_thread_t *)arg;
    obj_conn_queue_item_t *item;
    char buf[1];
    obj_conn_t *c;
    unsigned int fd_from_pipe;
    if (read(fd, buf, 1) != 1) {
        if (obj_settings.verbose > 0) {
            fprintf(stderr, "can't read from libevent pipe\n");
        }
        return;
    }
    switch (buf[0]) {
        case 'c':
            item = obj_conn_queue_pop(this->new_conn_queue);
            if (item == NULL) {
                break;
            }
            c = obj_conn_new(item->sfd, item->init_state, item->event_flags, this->base);
            if (c == NULL) {
                if (obj_settings.verbose > 0) {
                    fprintf(stderr, "can't listen for events on fd %d\n", item->sfd);
                }
                close(item->sfd);
            } else {
                c->thread = this;
            }
            /* free the item */
            obj_conn_queue_free_item(item);
            break;
        case 't':
            if (read(fd, &fd_from_pipe, sizeof(fd_from_pipe)) != sizeof(fd_from_pipe)) {
                if (obj_settings.verbose > 0) {
                    fprintf(stderr, "can't read timeout fd from pipe\n");
                    return;
                }
            }
            obj_conn_close_idle(obj_conn_conns[fd_from_pipe]);
            break;
        case 's':
            event_base_loopexit(this->base, NULL);
            break;
        default:
            break;
    }
}

/* kick out idle threads */
static void *obj_thread_conn_timeout_thread(void *arg) {
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
    while (obj_thread_run_conn_timeout_thread) {
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
            if (c->state != OBJ_CONN_NEW_CMD && c->state != OBJ_CONN_READ) {
                continue;
            }
            if ((obj_rel_current_time - c->last_cmd_time) > obj_settings.idle_timeout) {
                buf[0] = 't';
                obj_memcpy(&buf[1], &i, sizeof(int));
                if (write(c->thread->notify_send_fd, buf, OBJ_CONN_TIMEOUT_MSG_SIZE) != OBJ_CONN_TIMEOUT_MSG_SIZE) {
                    perror("failed to write timeout message to notify pipe");
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
        /* TODO might could cause long time sleep!!! */
        /* fprintf(stderr, "sleep time = %d us\n", sleep_time * 1000000); */
        usleep((useconds_t) sleep_time * 1000000);
    }
    return NULL;
}

obj_bool_t obj_thread_start_conn_timeout_thread() {
    int ret;
    if (obj_settings.idle_timeout == 0) {
        return false;
    }
    obj_thread_run_conn_timeout_thread = true;
    if ((ret = pthread_create(&obj_thread_conn_timeout_tid, NULL, obj_thread_conn_timeout_thread, NULL)) != 0) {
        fprintf(stderr, "can't create idle connection timeout thread: %s\n", OBJ_STRERROR(ret));
        return false;
    }
    return true;
}

obj_bool_t obj_thread_stop_conn_timeout_thread() {
    if (!obj_thread_run_conn_timeout_thread) {
        return false;
    }
    obj_thread_run_conn_timeout_thread = false;
    pthread_join(obj_thread_conn_timeout_tid, NULL);
    return 0;
}

void obj_thread_stop_threads() {
    char buf[1];
    int i;
    if (obj_settings.verbose > 0) {
        fprintf(stderr, "asking workers to stop\n");
    }
    buf[0] = 's';
    pthread_mutex_lock(&obj_thread_worker_hang_lock);
    pthread_mutex_lock(&obj_thread_init_lock);
    obj_thread_init_count = 0;
    for (i = 0; i < obj_settings.num_threads; i++) {
        if (write(obj_thread_threads[i].notify_send_fd, buf, 1) != 1) {
            perror("failed writing to notify pipe");
        }
    }
    if (obj_settings.verbose > 0) {
        fprintf(stderr, "waiting workers to stop...\n");
    }
    obj_thread_wait_for_thread_registration(obj_settings.num_threads);
    pthread_mutex_unlock(&obj_thread_init_lock);
    /* stop connection timeout thread */
    obj_thread_stop_conn_timeout_thread();
    if (obj_settings.verbose > 0) {
        fprintf(stderr, "stopped idle timeout thread\n");
    }
    if (obj_settings.verbose > 0) {
        fprintf(stderr, "closing connections\n");
    }
    /* close all connections */
    obj_conn_close_all();
    pthread_mutex_unlock(&obj_thread_worker_hang_lock);
    for (i = 0; i < obj_settings.num_threads; i++) {
        pthread_join(obj_thread_threads[i].thread_id, NULL);
    }
    if (obj_settings.verbose > 0) {
        fprintf(stderr, "all background threads stopped\n");
    }
}

/* initialize worker threads */
void obj_thread_init(int nthreads, void *arg) {
    int i;
    pthread_mutex_init(&obj_thread_worker_hang_lock, NULL);
    /* init lock for worker threads */
    pthread_mutex_init(&obj_thread_init_lock, NULL);
    pthread_cond_init(&obj_thread_init_cond, NULL);
    /* init connection queue freelist lock */
    obj_conn_queue_item_freelist_init();
    /* setup worker threads */
    obj_thread_threads = (obj_thread_libevent_thread_t *)obj_alloc(nthreads * sizeof(obj_thread_libevent_thread_t));
    if (obj_thread_threads == NULL) {
        fprintf(stderr, "can't allocate thread descriptors\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < nthreads; i++) {
        int fds[2];
        if (pipe(fds)) {
            perror("can't create notify pipe");
            exit(EXIT_FAILURE);
        }
        obj_thread_threads[i].notify_receive_fd = fds[0];
        obj_thread_threads[i].notify_send_fd = fds[1];
        obj_thread_setup_thread(&obj_thread_threads[i]);
    }
    /* create worker threads */
    for (i = 0; i < nthreads; i++) {
        obj_thread_create_worker(obj_thread_worker_mainloop, &obj_thread_threads[i]);
    }
    /* wait for all threads to set themselves up before returning */
    pthread_mutex_lock(&obj_thread_init_lock);
    obj_thread_wait_for_thread_registration(nthreads);
    pthread_mutex_unlock(&obj_thread_init_lock);
}

