#include "obj_core.h"


/* translate expr to index bounds */
void obj_index_bounds_translate(obj_expr_base_expr_t *expr, const char *key, const obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_assert(out->intervals.size == 0);
    /* must be compare expression */
    obj_assert(expr->type >= OBJ_EXPR_TYPE_EQ && expr->type <= OBJ_EXPR_TYPE_GTE);
    obj_expr_compare_expr_t *compare_expr = (obj_expr_compare_expr_t *)expr;
    out->name = obj_stringdata_create((char *)key);
    obj_interval_t interval;
    obj_bson_value_t *expr_value = &compare_expr->value;
    obj_interval_make_interval(expr_value, &interval, expr->type);
    /* append to out */
    obj_array_push_back(&out->intervals, &interval);
}

/* translate expr and intersect with current intervals */
void obj_index_bounds_translate_and_intersect(obj_expr_base_expr_t *expr, const char *key, const obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_ordered_interval_list_t oil;
    obj_index_bounds_translate(expr, key, value, index_entry, &oil);
    /* do intersect */
    obj_ordered_interval_list_intersect(out, &oil);
    out->name = oil.name;
}

/* translate expr and union with current intervals */
void obj_index_bounds_translate_and_union(obj_expr_base_expr_t *expr, const char *key, const obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_ordered_interval_list_t oil;
    obj_index_bounds_translate(expr, key, value, index_entry, &oil);
    /* do union */
    obj_ordered_interval_list_union(out, &oil);
    out->name = oil.name;
}

void obj_index_bounds_align_bounds(obj_index_bounds_t *bounds, obj_bson_t *key_pattern) {
    obj_bson_iter_t iter;
    const char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_init(&iter, key_pattern);
    int index = 0;
    int direction;
    const obj_bson_value_t *value = NULL;
    obj_ordered_interval_list_t *oil = NULL;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        value = obj_bson_iter_value(&iter);
        direction = (value->value.v_int32 >= 0) ? 1 : -1;
        if (direction == -1) {
            /* reverse fields */
            oil = (obj_ordered_interval_list_t *)obj_array_get_index(&bounds->fields, index);
            obj_ordered_interval_list_reverse(oil);
        }
        ++index;
    }
}

/* dump index bound */
void obj_index_bounds_dump(obj_index_bounds_t *bounds) {
    printf("index bound:\n");
    int i;
    obj_ordered_interval_list_t *oil = NULL;
    for (i = 0; i < bounds->fields.size; i++) {
        oil = (obj_ordered_interval_list_t *)obj_array_get_index(&bounds->fields, i);
        printf("%d th ordered interval list\n", i);
        obj_ordered_interval_list_dump(oil);
    }
}

