#ifndef OBJ_PROTO_H
#define OBJ_PROTO_H

/* binary communication protocol */

#include "obj_core.h"

/* forward declaration */
typedef struct obj_conn_s obj_conn_t;



/* max length of message, 4MB */
#define OBJ_MSG_MAX_LEN (1 << 22)

obj_bool_t obj_proto_read_command(obj_conn_t *c);

#endif  /* OBJ_PROTO_H */