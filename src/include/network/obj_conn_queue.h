#ifndef OBJ_CONNECTION_H
#define OBJ_CONNECTION_H

#include "obj_core.h"

#define OBJ_CONN_QUEUE_ITEMS_PER_ALLOC 64

typedef struct obj_conn_queue_item_s obj_conn_queue_item_t;
typedef struct obj_conn_queue_s obj_conn_queue_t;


/* an item in the connection queue */
struct obj_conn_queue_item_s {
    int sfd;
    obj_conn_state_t init_state;
    int event_flags;
    obj_conn_queue_item_t *next;
};

/* connection queue */
struct obj_conn_queue_s {
    obj_conn_queue_item_t *head;
    obj_conn_queue_item_t *tail;
    pthread_mutex_t lock;
};

#endif  /* OBJ_CONNECTION_H */