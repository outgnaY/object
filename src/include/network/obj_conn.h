#ifndef OBJ_CONN_H
#define OBJ_CONN_H

#include "obj_core.h"

#define OBJ_CONN_QUEUE_ITEMS_PER_ALLOC 64

typedef enum obj_conn_state obj_conn_state_t;
/* connection queue */
typedef struct obj_conn_queue_item_s obj_conn_queue_item_t;
typedef struct obj_conn_queue_s obj_conn_queue_t;
/* connection */
typedef struct obj_conn_s obj_conn_t;


/* export globals */


enum obj_conn_state {
    OBJ_CONN_LISTENING,
    OBJ_CONN_NEW_CMD,
    OBJ_CONN_READ,
    OBJ_CONN_CLOSING,
    OBJ_CONN_CLOSED
};

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

/* an established thread */
struct obj_conn_s {
    int sfd;                                /* socket fd */
    obj_conn_state_t state;                 /* state of the connection */
    struct event event;                     /* libevent event */
    short event_flags;                      /* event flags */
    obj_thread_libevent_thread_t *thread;   /* thread */
    obj_rel_time_t last_cmd_time;           /* time for the last command of this connection */
    short which;                            /* which events were just triggered */
    obj_conn_t *next;                       /* used to generate a list of conn structures */
};

extern obj_conn_t **obj_conn_conns;         /* connection array */
extern int obj_conn_max_fds;

void obj_conn_queue_init(obj_conn_queue_t *cq);

#endif  /* OBJ_CONN_H */