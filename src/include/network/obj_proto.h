#ifndef OBJ_PROTO_H
#define OBJ_PROTO_H

/* binary communication protocol */

#include "obj_core.h"

/* forward declaration */
typedef struct obj_conn_s obj_conn_t;

typedef enum obj_msg_operation obj_msg_operation_t;
typedef struct obj_msg_header_s obj_msg_header_t;
typedef struct obj_msg_reply_s obj_msg_reply_t;
typedef struct obj_msg_update_s obj_msg_update_t;
typedef struct obj_msg_insert_s obj_msg_insert_t;
typedef struct obj_msg_query_s obj_msg_query_t;
typedef struct obj_msg_delete_s obj_msg_delete_t;

enum obj_msg_operation {
    OBJ_MSG_OP_REPLY = 0,                   /* reply */
    OBJ_MSG_OP_UPDATE = 1,                  /* update */
    OBJ_MSG_OP_INSERT = 2,                  /* insert */
    OBJ_MSG_OP_QUERY = 3,                   /* query */
    OBJ_MSG_OP_DELETE = 4                   /* delete */
};


#pragma pack(1)
/* message header */
struct obj_msg_header_s {
    obj_int32_t len;                /* length of message */
    obj_int32_t opCode;             /* operation type */
};

/* reply */
struct obj_msg_reply_s {
    obj_msg_header_t header;
};

/* update */
struct obj_msg_update_s {
    obj_msg_header_t header;
    char *collection_name;              /* full collection name */
    int name_len;
    obj_int32_t flags;                  /* bit 0: upsert; bit 1: multiupdate */
    obj_bson_t *selector;               /* query to select the document */
    obj_bson_t *update;                 /* specification of the update to perform */
};

/* insert */
struct obj_msg_insert_s {
    obj_msg_header_t header;
    char *collection_name;              /* full collection name */
    int name_len;
    obj_int32_t flags;                  
    obj_int32_t num;                    /* object num */
    obj_bson_t **objects;               /* objects to insert */
};

/* query */
struct obj_msg_query_s {
    obj_msg_header_t header;
    char *collection_name;              /* full collection name */
    int name_len;
    obj_int32_t flags;
    obj_int32_t num_return;             /* number of objects to return */
    obj_bson_t *query;                  /* query */
    obj_bson_t *return_fields;          /* fields to return */
};

/* delete */
struct obj_msg_delete_s {
    obj_msg_header_t header;
    char *collection_name;              /* full collection name */
    int name_len;
    obj_int32_t flags;                  /* bit 0: single remove */
    obj_bson_t *selector;               /* query object */
};

#pragma pack()

/* max length of message, 4MB */
#define OBJ_MSG_MAX_LEN (1 << 22)



#endif  /* OBJ_PROTO_H */