#ifndef OBJ_MESSAGE_H
#define OBJ_MESSAGE_H

/* binary communication protocol */

#include "obj_core.h"

typedef enum obj_msg_operation obj_msg_operation_t;
typedef struct obj_msg_header_s obj_msg_header_t;
typedef struct obj_msg_reply_s obj_msg_reply_t;
typedef struct obj_msg_update_s obj_msg_update_t;
typedef struct obj_msg_insert_s obj_msg_insert_t;
typedef struct obj_msg_query_s obj_msg_query_t;
typedef struct obj_msg_delete_s obj_msg_delete_t;

enum obj_msg_operations {
    OBJ_MSG_OP_REPLY = 0,                   /* reply */
    OBJ_MSG_OP_UPDATE = 1,                  /* update */
    OBJ_MSG_OP_INSERT = 2,                  /* insert */
    OBJ_MSG_OP_QUERY = 3,                   /* query */
    OBJ_MSG_OP_DELETE = 4                   /* delete */
};

/* message header */
#pragma pack(1)
struct obj_msg_header_s {
    obj_int32_t len;                /* length of message */
    obj_int32_t opCode;             /* operation type */
};

struct obj_msg_reply_s {
    obj_msg_header_t header;
};

struct obj_msg_update_s {
    obj_msg_header_t header;
    obj_sds_t collection;               /* full collection name */
    obj_int32_t flags;                  /* bit 0: upsert; bit 1: multiupdate */
    obj_bson_t *selector;               /* query to select the document */
    obj_bson_t *update;                 /* specification of the update to perform */
};

struct obj_msg_insert_s {
    obj_msg_header_t header;
    obj_sds_t collection;               /* full collection name */
    obj_int32_t flags;                  
    obj_bson_t **objects;               /* objects to insert */
    obj_int32_t num;                    /* object num */
};

struct obj_msg_query_s {
    obj_msg_header_t header;
    obj_sds_t collection;               /* full collection name */
    obj_int32_t num_return;             /* number of objects to return */
    obj_bson_t *query;                  /* query */
    obj_bson_t *return_fields;          /* fields to return */
};

struct obj_msg_delete_s {
    obj_msg_header_t header;
    obj_sds_t collection;               /* full collection name */
    obj_int32_t flags;                  /* bit 0: single remove */
    obj_bson_t *selector;               /* query object */
};

#pragma pack()

#endif  /* OBJ_MESSAGE_H */