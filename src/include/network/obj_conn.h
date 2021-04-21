#ifndef OBJ_CONN_H
#define OBJ_CONN_H

#include "obj_core.h"

/* forward declaration */
typedef struct obj_thread_libevent_thread_s obj_thread_libevent_thread_t;

typedef enum obj_conn_state obj_conn_state_t;
typedef enum obj_conn_read_result obj_conn_read_result_t;
typedef enum obj_conn_write_result obj_conn_write_result_t;
/* connection queue */
typedef struct obj_conn_queue_item_s obj_conn_queue_item_t;
typedef struct obj_conn_queue_s obj_conn_queue_t;
/* connection */
typedef struct obj_conn_context_s obj_conn_context_t;
typedef struct obj_conn_s obj_conn_t;
typedef struct obj_conn_reply_block_s obj_conn_reply_block_t;



enum obj_conn_state {
    OBJ_CONN_LISTENING,                 /* listen for connections */
    OBJ_CONN_WAITING,                   /* waiting for a readable socket */
    OBJ_CONN_NEW_CMD,                   /* prepare connection for next command */
    OBJ_CONN_PARSE_CMD,                 /* parse and process command */
    OBJ_CONN_READ,                      /* read */
    OBJ_CONN_WRITE,                     /* write */
    OBJ_CONN_CLOSING,                   /* closing the connection */
    OBJ_CONN_CLOSED                     /* connection is closed */
};

enum obj_conn_read_result {
    OBJ_CONN_READ_DATA_RECEIVED,
    OBJ_CONN_READ_NO_DATA_RECEIVED,
    OBJ_CONN_READ_ERROR,                /* an error occurred, or connection closed by client */
    OBJ_CONN_READ_MEMORY_ERROR          /* failed to allocate more memory */
};

enum obj_conn_write_result {
    OBJ_CONN_WRITE_COMPLETE,            /* all done writing */
    OBJ_CONN_WRITE_INCOMPLETE,          /* more data remaining to write */
    OBJ_CONN_WRITE_SOFT_ERROR,          /* can't write any more right now */
    OBJ_CONN_WRITE_HARD_ERROR           /* can't write any more right now, close the connection */
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

struct obj_conn_context_s {
    obj_locker_t *locker;
};

/* an established thread */
struct obj_conn_s {
    int sfd;                                    /* socket fd */
    obj_conn_state_t state;                     /* state of the connection */
    struct event event;                         /* libevent event */
    short event_flags;                          /* event flags */
    obj_thread_libevent_thread_t *thread;       /* thread */
    obj_rel_time_t last_cmd_time;               /* time for the last command of this connection */
    short which;                                /* which events were just triggered */
    obj_buffer_t *inbuf;                        /* input buffer */
    obj_buffer_t *outbuf;                       /* output buffer */
    obj_bool_t close_after_write;               /* close the connection after write */
    obj_list_t *reply_list;                     /* reply send to client */
    obj_conn_t *next;                           /* used to generate a list of conn structures */
    obj_conn_context_t *context;                /* context of current connection */
};

/* content of reply list */
struct obj_conn_reply_block_s {
    obj_buffer_t *buf;
};

#define OBJ_CONN_QUEUE_ITEMS_PER_ALLOC 64
#define OBJ_CONN_WRITE_MAX_BYTES_PER_EVENT (1024 * 64)  /* 64KB */

/* export globals */
extern int obj_conn_max_fds;
extern obj_conn_t **obj_conn_conns;             /* connection array */
extern obj_conn_t *obj_conn_listen_conn;

void obj_conn_queue_item_freelist_init();
void obj_conn_queue_init(obj_conn_queue_t *cq);
obj_conn_queue_item_t *obj_conn_queue_pop(obj_conn_queue_t *cq);
void obj_conn_queue_free_item(obj_conn_queue_item_t *item);
void obj_conn_add_reply(obj_conn_t *c, obj_bson_t *reply);
void obj_conn_conns_init();
obj_conn_context_t *obj_conn_context_create();
void obj_conn_context_destroy(obj_conn_context_t *context);
obj_conn_t *obj_conn_new(const int sfd, obj_conn_state_t init_state, const short event_flags, struct event_base *base);
void obj_conn_dispatch_conn_new(int sfd, obj_conn_state_t init_state, int event_flags);
void obj_conn_accept_new_conns(const obj_bool_t do_accept);
void obj_conn_do_accept_new_conns(const obj_bool_t do_accept);
obj_bool_t obj_conn_update_event(obj_conn_t *c, const int new_flags);
void obj_conn_event_handler(const evutil_socket_t fd, const short which, void *arg);
void obj_conn_close_idle(obj_conn_t *c);
void obj_conn_close(obj_conn_t *c);
void obj_conn_close_all();
void obj_conn_set_state(obj_conn_t *c, obj_conn_state_t state);


#endif  /* OBJ_CONN_H */