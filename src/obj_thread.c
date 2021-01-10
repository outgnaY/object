#include "obj_core.h"

/* lock to cause worker threads to hang up after being woken */
static pthread_mutex_t obj_thread_worker_hang_lock;

/* thread init */
static int obj_thread_init_count = 0;
static pthread_mutex_t obj_thread_init_lock;
static pthread_cond_t obj_thread_init_cond;


/* wait for thread registration */
static void obj_thread_wait_for_thread_registration(int nthreads) {
    while (obj_thread_init_count < nthreads) {
        pthread_cond_wait(&obj_thread_init_cond, &obj_thread_init_lock);
    }
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


/* create a worker thread */
static void obj_thread_create_worker(void *(*func)(void *), void *arg) {
    pthread_attr_t attr;
    int ret;
    pthread_attr_init(&attr);
    if ((ret = pthread_create())) {
        fprintf(stderr, "can't create thread: %s\n", );
        exit(1);
    }
}

static void obj_thread_libevent_process(evutil_socket_t fd, short which, void *arg) {
    obj_thread_libevent_thread_t *this = (obj_thread_libevent_thread_t *)arg;
    obj_conn_queue_item_t *item;
    char buf[1];

}

void obj_thread_stop_threads() {
    char buf[1];
    int i;
    buf[0] = 's';
    pthread_mutex_lock(&obj_thread_worker_hang_lock);
    pthread_mutex_lock(&obj_thread_init_lock);
    int count = 0;
    
}
