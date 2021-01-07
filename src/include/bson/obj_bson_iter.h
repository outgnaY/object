#ifndef OBJ_BSON_ITER_H
#define OBJ_BSON_ITER_H

#include "obj_core.h"
#include "bson/obj_bson.h"

typedef struct obj_bson_iter_s obj_bson_iter_t;
typedef struct obj_bson_visitor_s obj_bson_visitor_t;

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

/* bson visitor */
struct obj_bson_visitor_s {
    obj_bool_t (*visit_double)(const obj_bson_iter_t *iter, const char *key, double v_double, void *data);
    obj_bool_t (*visit_utf8)(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const char *v_utf8, void *data);
    obj_bool_t (*visit_object)(const obj_bson_iter_t *iter, const char *key, void *data);
    obj_bool_t (*visit_array)(const obj_bson_iter_t *iter, const char *key, void *data);
    obj_bool_t (*visit_binary)(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const obj_uint8_t *v_binary, void *data);
    obj_bool_t (*visit_bool)(const obj_bson_iter_t *iter, const char *key, obj_bool_t v_bool, void *data);
    obj_bool_t (*visit_null)(const obj_bson_iter_t *iter, const char *key, void *data);
    obj_bool_t (*visit_int32)(const obj_bson_iter_t *iter, const char *key, obj_int32_t v_int32, void *data);
    obj_bool_t (*visit_int64)(const obj_bson_iter_t *iter, const char *key, obj_int64_t v_int64, void *data);
};




#endif  /* OBJ_BSON_ITER_H */