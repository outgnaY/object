#ifndef OBJ_MESSAGE_H
#define OBJ_MESSAGE_H

/* binary communication protocol */

#include "obj_core.h"

typedef enum obj_msg_operation obj_msg_operation_t;
typedef struct obj_msg_header_s obj_msg_header_t;
typedef struct obj_msg_s obj_msg_t;

enum obj_msg_operations {
    OBJ_MSG_OP_UPDATE = 1,
    OBJ_MSG_OP_INSERT = 2,
    OBJ_MSG_OP_QUERY = 3,
    OBJ_MSG_OP_DELETE = 4
};

/* message header */
#pragma pack(1)
struct obj_msg_header_s {
    obj_int32_t len;                /* length of message */
    obj_int32_t opCode;             /* operation type */
};

struct obj_msg_s {
    obj_msg_header_t header;        /* message header */
    char data[];                    /* message contents */
};



#pragma pack()

#endif  /* OBJ_MESSAGE_H */