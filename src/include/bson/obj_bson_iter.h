#ifndef OBJ_BSON_ITER_H
#define OBJ_BSON_ITER_H

#include "obj_core.h"
#include "bson/obj_bson.h"

typedef struct obj_bson_iter_s obj_bson_iter_t;

/* bson iterator */
struct obj_bson_iter_s {
    obj_uint8_t *buf;               /* buffer being iterated */
    obj_int32_t len;                /* length of buffer */
    int off_type;                   /* offset of the type byte */
    int off_key;                    /* offset of the key */
    int off_d1;                     /* offset of data1 */
    int off_d2;                     /* offset of data2 */
    int off_err;                    /* offset of error position */
    int off;                        /* current offset of the buffer */
    int off_next;                   /* offset of next element */
    obj_bson_value_t value;         /* current value */
};




#endif  /* OBJ_BSON_ITER_H */