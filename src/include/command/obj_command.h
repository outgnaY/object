#ifndef OBJ_COMMAND_H
#define OBJ_COMMAND_H

#include "obj_core.h"

typedef struct obj_command_processor_pair_s obj_command_processor_pair_t;


struct obj_command_processor_pair_s {
    char *name;
    void (*processor)(obj_conn_t *c, obj_bson_t *command_bson);
};


void obj_process_command(obj_conn_t *c, obj_bson_t *command_bson);


#endif  /* OBJ_COMMAND_H */