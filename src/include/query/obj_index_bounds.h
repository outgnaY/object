#ifndef OBJ_INDEX_BOUNDS_H
#define OBJ_INDEX_BOUNDS_H

#include "obj_core.h"

typedef enum obj_index_bound_inclusion obj_index_bound_inclusion_t;
typedef struct obj_index_bounds_s obj_index_bounds_t;

enum obj_index_bound_inclusion {
    OBJ_INDEX_BOUND_EXCLUDE_START_AND_END,
    OBJ_INDEX_BOUND_INCLUDE_START,
    OBJ_INDEX_BOUND_INCLUDE_END,
    OBJ_INDEX_BOUND_INCLUDE_START_AND_END
};

struct obj_index_bounds_s {
    /* [ordered interval list] */
    obj_array_t fields;
};


void obj_index_bounds_translate(obj_expr_base_expr_t *expr, const char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out);
void obj_index_bounds_translate_and_intersect(obj_expr_base_expr_t *expr, const char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out);
void obj_index_bounds_translate_and_union(obj_expr_base_expr_t *expr, const char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out);
void obj_index_bounds_align_bounds(obj_index_bounds_t *bounds, obj_bson_t *key_pattern);

#endif  /* OBJ_INDEX_BOUNDS_H */