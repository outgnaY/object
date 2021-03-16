#ifndef OBJ_EXPR_H
#define OBJ_EXPR_H

#include "obj_core.h"

typedef enum obj_expr_type obj_expr_type_t;
typedef struct obj_expr_methods_s obj_expr_methods_t;
typedef struct obj_expr_base_expr_s obj_expr_base_expr_t;
typedef struct obj_expr_tree_expr_s obj_expr_tree_expr_t;
typedef struct obj_expr_compare_expr_s obj_expr_compare_expr_t;
typedef struct obj_expr_not_expr_s obj_expr_not_expr_t;

enum obj_expr_type {
    /* logical */
    OBJ_EXPR_TYPE_AND,
    OBJ_EXPR_TYPE_OR,
    /* compare */
    OBJ_EXPR_TYPE_EQ,
    OBJ_EXPR_TYPE_LTE,
    OBJ_EXPR_TYPE_LT,
    OBJ_EXPR_TYPE_GT,
    OBJ_EXPR_TYPE_GTE,
    /* negation */
    OBJ_EXPR_TYPE_NOT,
    OBJ_EXPR_TYPE_NOR,
    OBJ_EXPR_TYPE_NEQ
};

struct obj_expr_methods_s {
    /* child num */
    int (*num_child)(obj_expr_base_expr_t *expr);
    /* get i-th child */
    obj_expr_base_expr_t *(*get_child)(obj_expr_base_expr_t *expr, int i);
    /* destroy */
    void (*destroy)(obj_expr_base_expr_t *expr);
};

struct obj_expr_base_expr_s {
    obj_expr_type_t type;
    obj_expr_methods_t *methods;
};

/* AND OR NOR */
struct obj_expr_tree_expr_s {
    obj_expr_base_expr_t base;
    obj_array_t expr_list;
};

/* EQ LTE LT GT GTE */
struct obj_expr_compare_expr_s {
    obj_expr_base_expr_t base;
    char *path;
    obj_bson_value_t value;
};

/* NOT */
struct obj_expr_not_expr_s {
    obj_expr_base_expr_t base;
    obj_expr_base_expr_t *expr;
};

/* not expression */
obj_expr_base_expr_t *obj_expr_not_expr_create(obj_expr_base_expr_t *child);
/* compare expression */
obj_expr_base_expr_t *obj_expr_compare_expr_create(const char *path, obj_expr_type_t type, const obj_bson_value_t *value);
/* tree expression */
obj_expr_base_expr_t *obj_expr_tree_expr_create(obj_expr_type_t type);
obj_bool_t obj_expr_tree_add_child(obj_expr_base_expr_t *expr, obj_expr_base_expr_t *child);
/* debug */
void obj_expr_dump(obj_expr_base_expr_t *expr);

#endif  /* OBJ_EXPR_H */