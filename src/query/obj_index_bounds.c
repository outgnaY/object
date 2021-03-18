#include "obj_core.h"


/* translate expr to index bounds */
void obj_index_bounds_translate(obj_expr_base_expr_t *expr, const char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_assert(out->intervals.size == 0);
    /* must be compare expression */
    obj_assert(expr->type >= OBJ_EXPR_TYPE_EQ && expr->type <= OBJ_EXPR_TYPE_GTE);
    obj_expr_compare_expr_t *compare_expr = (obj_expr_compare_expr_t *)expr;
    out->name = obj_stringdata_create(key);
    obj_interval_t interval;
    obj_interval_make_interval(value, &interval, expr->type);
    /* append to out */
    obj_array_push_back(&out->intervals, &interval);
}

/* translate expr and intersect with current intervals */
void obj_index_bounds_translate_and_intersect(obj_expr_base_expr_t *expr, const char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_ordered_interval_list_t oil;
    obj_index_bounds_translate(expr, key, value, index_entry, &oil);
    /* do intersect */
    obj_ordered_interval_list_intersect(out, &oil);
}

/* translate expr and union with current intervals */
void obj_index_bounds_translate_and_union(obj_expr_base_expr_t *expr, const char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_ordered_interval_list_t oil;
    obj_index_bounds_translate(expr, key, value, index_entry, &oil);
    /* do union */
    obj_ordered_interval_list_union(out, &oil);
}

