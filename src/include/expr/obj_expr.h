#ifndef OBJ_EXPR_H
#define OBJ_EXPR_H

#include "obj_core.h"

typedef enum obj_expr_type obj_expr_type_t;
typedef struct obj_expr_base_s obj_expr_base_t;
typedef struct obj_expr_logical_s obj_expr_logical_t;
typedef struct obj_expr_compare_s obj_expr_compare_t;

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
    OBJ_EXPR_NOR
};

struct obj_expr_base_s {
    obj_expr_type_t type;
};

/* AND OR */
struct obj_expr_logical_s {
    obj_expr_base_t type;
    obj_expr_base_t **list;
    int len;
};

/* EQ LTE LT GT GTE */
struct obj_expr_compare_s {
    obj_expr_base_t type;
    char *path;
    obj_bson_value_t value;
};

#endif  /* OBJ_EXPR_H */