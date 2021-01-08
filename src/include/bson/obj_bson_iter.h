#ifndef OBJ_BSON_ITER_H
#define OBJ_BSON_ITER_H

#include "obj_core.h"

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

obj_bool_t obj_bson_iter_init(obj_bson_iter_t *iter, const obj_bson_t *bson);
obj_bool_t obj_bson_iter_next_internal(obj_bson_iter_t *iter, const char **key, obj_bson_type_t *bson_type);
double obj_bson_iter_double(const obj_bson_iter_t *iter);
const char *obj_bson_iter_utf8(const obj_bson_iter_t *iter, obj_int32_t *len);
const obj_uint8_t *obj_bson_iter_binary(const obj_bson_iter_t *iter, obj_int32_t *len);
const obj_uint8_t *obj_bson_iter_object(const obj_bson_iter_t *iter, obj_int32_t *len);
const obj_uint8_t *obj_bson_iter_array(const obj_bson_iter_t *iter, obj_int32_t *len);
obj_bool_t obj_bson_iter_bool(const obj_bson_iter_t *iter);
obj_int32_t obj_bson_iter_int32(const obj_bson_iter_t *iter);
obj_int64_t obj_bson_iter_int64(const obj_bson_iter_t *iter);
obj_bool_t obj_bson_iter_next(obj_bson_iter_t *iter);
const obj_bson_value_t *obj_bson_iter_value(obj_bson_iter_t *iter);


#endif  /* OBJ_BSON_ITER_H */