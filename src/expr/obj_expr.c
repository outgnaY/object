#include "obj_core.h"

/* new tree expression */
obj_expr_base_t *obj_expr_tree_create(obj_expr_type_t type) {
    obj_expr_tree_t *expr;
    expr = obj_alloc(sizeof(obj_expr_tree_t));
    if (expr == NULL) {
        return NULL;
    }
    expr->type.type = type;
    /* children */
    expr->expr_list = obj_array_create(sizeof(obj_expr_base_t *));
    if (expr->expr_list == NULL) {
        obj_free(expr);
        return NULL;
    }
    return (obj_expr_base_t *)expr;
}

void obj_expr_tree_destroy(obj_expr_type_t type) {
    
}

/* add child to tree expression */
obj_bool_t obj_expr_tree_add_child(obj_expr_base_t *expr, obj_expr_base_t *child) {
    obj_assert(expr && (expr->type == OBJ_EXPR_AND || expr->type == OBJ_EXPR_OR || expr->type == OBJ_EXPR_NOR));
    obj_assert(((obj_expr_tree_t *)expr)->expr_list);
    obj_array_t *expr_list = ((obj_expr_tree_t *)expr)->expr_list;
    return obj_array_push_back(expr_list, &child);
}

/* for debug */
void obj_expr_dump(obj_expr_base_t *expr) {

}