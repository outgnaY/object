#ifndef OBJ_BSON_ITER_H
#define OBJ_BSON_ITER_H

#include "obj_common.h"
#include "bson/obj_bson.h"

typedef struct obj_bson_iter_s obj_bson_iter_t;

struct obj_bson_iter_s {
    obj_uint8_t *pos;               /* current position */
    obj_uint8_t *end;               /* end position */
    obj_bson_value_t value;         /* current value */
};


#endif  /* OBJ_BSON_ITER_H */