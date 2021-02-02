#ifndef OBJ_EXPR_PARSE_H
#define OBJ_EXPR_PARSE_H

#include "obj_core.h"

typedef struct obj_expr_parse_operator_element_s obj_expr_parse_operator_element_t;

struct obj_expr_parse_operator_element_s {
    const char *name;
    obj_expr_type_t type;
    
};

#endif  /* OBJ_EXPR_PARSE_H */