#ifndef OBJ_EXPR_PARSE_H
#define OBJ_EXPR_PARSE_H

#include "obj_core.h"

typedef obj_status_with_t (*obj_expr_parse_fn)(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level);
typedef enum obj_expr_parse_level obj_expr_parse_level_t;
typedef struct obj_expr_parser_s obj_expr_parser_t;

enum obj_expr_parse_level {
    OBJ_EXPR_PARSE_LEVEL_PREDICATE_TOP_LEVEL,
    OBJ_EXPR_PARSE_LEVEL_USER_OBJ_TOP_LEVEL,
    OBJ_EXPR_PARSE_LEVEL_USER_SUB_OBJ
};

/* parser */
struct obj_expr_parser_s {
    const char *name;
    obj_expr_parse_fn parser;
};

#endif  /* OBJ_EXPR_PARSE_H */