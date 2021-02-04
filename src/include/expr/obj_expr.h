#ifndef OBJ_EXPR_H
#define OBJ_EXPR_H

#include "obj_core.h"

typedef enum obj_expr_type obj_expr_type_t;
typedef struct obj_expr_base_s obj_expr_base_t;
typedef struct obj_expr_tree_s obj_expr_tree_t;
typedef struct obj_expr_compare_s obj_expr_compare_t;
typedef struct obj_expr_not_s obj_expr_not_t;

enum obj_expr_type {
    /* logical */
    OBJ_EXPR_AND,
    OBJ_EXPR_OR,
    /* compare */
    OBJ_EXPR_EQ,
    OBJ_EXPR_LTE,
    OBJ_EXPR_LT,
    OBJ_EXPR_GT,
    OBJ_EXPR_GTE,
    /* negation */
    OBJ_EXPR_NOT,
    OBJ_EXPR_NOR,
    OBJ_EXPR_NEQ
};

struct obj_expr_base_s {
    obj_expr_type_t type;
};

/* AND OR NOR */
struct obj_expr_tree_s {
    obj_expr_base_t type;
    obj_array_t *expr_list;
};

/* EQ LTE LT GT GTE */
struct obj_expr_compare_s {
    obj_expr_base_t type;
    char *path;
    obj_bson_value_t value;
};

/* NOT */
struct obj_expr_not_s {
    obj_expr_base_t type;
    obj_expr_base_t *expr;
};

obj_expr_base_t *obj_expr_not_create(obj_expr_base_t *child);
obj_expr_base_t *obj_expr_compare_create(const char *path, obj_expr_type_t type, obj_bson_value_t *value);
void obj_expr_compare_destroy(obj_expr_base_t *expr);
obj_expr_base_t *obj_expr_tree_create(obj_expr_type_t type);
void obj_expr_tree_destroy(obj_expr_base_t *expr);
obj_bool_t obj_expr_tree_add_child(obj_expr_base_t *expr, obj_expr_base_t *child);

#endif  /* OBJ_EXPR_H */