#ifndef OBJ_THREAD_H
#define OBJ_THREAD_H

#include "obj_core.h"


typedef struct obj_thread_libevent_thread_s obj_thread_libevent_thread_t;

struct obj_thread_libevent_thread_s {
    pthread_t thread_id;                        /* thread id */
    struct event_base *base;                    /* event base for this thread */
    struct event notify_event;                  
    int notify_receive_fd;                      /* receiving end of notify pipe */
    int notify_send_fd;                         /* sending end of notify pipe */
    obj_conn_queue_t *new_conn_queue;           /* queue of new connections */
};

extern int obj_thread_last_thread;
extern obj_thread_libevent_thread_t *obj_thread_threads;

#endif  /* OBJ_THREAD_H */