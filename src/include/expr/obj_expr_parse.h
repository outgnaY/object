#ifndef OBJ_EXPR_PARSE_H
#define OBJ_EXPR_PARSE_H

#include "obj_core.h"

typedef enum obj_expr_parse_level obj_expr_parse_level_t;
typedef struct obj_expr_parse_fn_pair_s obj_expr_parse_fn_pair_t;
typedef struct obj_expr_parse_type_pair_s obj_expr_parse_type_pair_t;
typedef obj_status_with_t (*obj_expr_parse_fn)(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level);

enum obj_expr_parse_level {
    OBJ_EXPR_PARSE_LEVEL_PREDICATE_TOP_LEVEL,
    OBJ_EXPR_PARSE_LEVEL_USER_OBJ_TOP_LEVEL,
    OBJ_EXPR_PARSE_LEVEL_USER_SUB_OBJ
};

/* parse function */
struct obj_expr_parse_fn_pair_s {
    const char *name;
    obj_expr_parse_fn parser;
};

/* parse keywords */
struct obj_expr_parse_type_pair_s {
    const char *name;
    obj_expr_type_t type;
};

obj_status_with_t obj_expr_parse(const obj_bson_t *bson);
obj_status_with_t obj_expr_parse_all(const obj_bson_t *bson, obj_expr_parse_level_t current_level);


#endif  /* OBJ_EXPR_PARSE_H */