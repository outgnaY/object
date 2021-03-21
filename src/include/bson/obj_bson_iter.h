#ifndef OBJ_BSON_ITER_H
#define OBJ_BSON_ITER_H

#include "obj_core.h"

typedef struct obj_bson_visit_visitor_s obj_bson_visit_visitor_t;
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

obj_bool_t obj_bson_iter_init(obj_bson_iter_t *iter, obj_bson_t *bson);
obj_bool_t obj_bson_iter_next_internal(obj_bson_iter_t *iter, char **key, obj_bson_type_t *bson_type);
double obj_bson_iter_double(obj_bson_iter_t *iter);
char *obj_bson_iter_utf8(obj_bson_iter_t *iter, obj_int32_t *len);
obj_uint8_t *obj_bson_iter_binary(obj_bson_iter_t *iter, obj_int32_t *len);
obj_uint8_t *obj_bson_iter_object(obj_bson_iter_t *iter, obj_int32_t *len);
obj_uint8_t *obj_bson_iter_array(obj_bson_iter_t *iter, obj_int32_t *len);
obj_bool_t obj_bson_iter_bool(obj_bson_iter_t *iter);
obj_int32_t obj_bson_iter_int32(obj_bson_iter_t *iter);
obj_int64_t obj_bson_iter_int64(obj_bson_iter_t *iter);
obj_bool_t obj_bson_iter_next(obj_bson_iter_t *iter);
obj_bson_value_t *obj_bson_iter_value(obj_bson_iter_t *iter);
obj_bool_t obj_bson_iter_visit_all(obj_bson_iter_t *iter, obj_bson_visit_visitor_t *visitor, void *data);


#endif  /* OBJ_BSON_ITER_H */