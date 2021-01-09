#ifndef OBJ_BSON_VISIT_H
#define OBJ_BSON_VISIT_H

#include "obj_core.h"

typedef struct obj_bson_visit_visitor_s obj_bson_visit_visitor_t;
typedef struct obj_bson_visit_print_state_s obj_bson_visit_print_state_t;
typedef enum obj_bson_visit_validate_flag obj_bson_visit_validate_flag_t;
typedef struct obj_bson_visit_validate_state_s obj_bson_visit_validate_state_t;

extern const obj_bson_visit_visitor_t obj_bson_visit_print_visitors;

/* bson visitor */
struct obj_bson_visit_visitor_s {
    obj_bool_t (*visit_before)(const obj_bson_iter_t *iter, const char *key, void *data);
    obj_bool_t (*visit_double)(const obj_bson_iter_t *iter, double v_double, void *data);
    obj_bool_t (*visit_utf8)(const obj_bson_iter_t *iter, obj_int32_t len, const char *v_utf8, void *data);
    obj_bool_t (*visit_object)(const obj_bson_iter_t *iter, const obj_bson_t *v_object, void *data);
    obj_bool_t (*visit_array)(const obj_bson_iter_t *iter, const obj_bson_t *v_array, void *data);
    obj_bool_t (*visit_binary)(const obj_bson_iter_t *iter, obj_int32_t len, const obj_uint8_t *v_binary, void *data);
    obj_bool_t (*visit_bool)(const obj_bson_iter_t *iter, obj_bool_t v_bool, void *data);
    obj_bool_t (*visit_null)(const obj_bson_iter_t *iter, void *data);
    obj_bool_t (*visit_int32)(const obj_bson_iter_t *iter, obj_int32_t v_int32, void *data);
    obj_bool_t (*visit_int64)(const obj_bson_iter_t *iter, obj_int64_t v_int64, void *data);
};

/* for debug */
struct obj_bson_visit_print_state_s {
    int count;                          /* element count */
    int depth;                          /* current depth */
};

enum obj_bson_visit_validate_flag {
    OBJ_BSON_VALIDATE_FLAG_NONE = 0,
    OBJ_BSON_VALIDATE_FLAG_UTF8_ALLOW_NULL = 1,
    OBJ_BSON_VALIDATE_FLAG_EMPTY_KEYS = 2
};

struct obj_bson_visit_validate_state_s {
    obj_bson_visit_validate_flag_t flag;
    int off_err;
};


/* print message for debug */
obj_bool_t obj_bson_visit_print_visit(const obj_bson_t *bson);
/* validate bson */
obj_bool_t obj_bson_visit_validate_visit(const obj_bson_t *bson, obj_bson_visit_validate_flag_t flag);


#endif  /* OBJ_BSON_VISIT_H */