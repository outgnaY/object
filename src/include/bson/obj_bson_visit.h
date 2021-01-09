#ifndef OBJ_BSON_VISIT_H
#define OBJ_BSON_VISIT_H

#include "obj_core.h"

typedef struct obj_bson_visitor_s obj_bson_visitor_t;
typedef struct obj_bson_print_state_s obj_bson_print_state_t;

extern const obj_bson_visitor_t obj_bson_print_visitors;

/* bson visitor */
struct obj_bson_visitor_s {
    obj_bool_t (*visit_before)(const obj_bson_iter_t *iter, const char *key, void *data);
    obj_bool_t (*visit_after)(const obj_bson_iter_t *iter, const char *key, void *data);
    obj_bool_t (*visit_double)(const obj_bson_iter_t *iter, const char *key, double v_double, void *data);
    obj_bool_t (*visit_utf8)(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const char *v_utf8, void *data);
    obj_bool_t (*visit_object)(const obj_bson_iter_t *iter, const char *key, const obj_bson_t *v_object, void *data);
    obj_bool_t (*visit_array)(const obj_bson_iter_t *iter, const char *key, const obj_bson_t *v_array, void *data);
    obj_bool_t (*visit_binary)(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const obj_uint8_t *v_binary, void *data);
    obj_bool_t (*visit_bool)(const obj_bson_iter_t *iter, const char *key, obj_bool_t v_bool, void *data);
    obj_bool_t (*visit_null)(const obj_bson_iter_t *iter, const char *key, void *data);
    obj_bool_t (*visit_int32)(const obj_bson_iter_t *iter, const char *key, obj_int32_t v_int32, void *data);
    obj_bool_t (*visit_int64)(const obj_bson_iter_t *iter, const char *key, obj_int64_t v_int64, void *data);
};

/* for debug */
struct obj_bson_print_state_s {
    int count;                          /* element count */
    int depth;                          /* current depth */
};

obj_bool_t obj_bson_print_visit(const obj_bson_t *bson);
obj_bool_t obj_bson_iter_visit_all(obj_bson_iter_t *iter, const obj_bson_visitor_t *visitor, void *data);


#endif  /* OBJ_BSON_VISIT_H */