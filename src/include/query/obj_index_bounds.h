#ifndef OBJ_INDEX_BOUNDS_H
#define OBJ_INDEX_BOUNDS_H

#include "obj_core.h"

/* forward declaration */
typedef struct obj_query_index_entry_s obj_query_index_entry_t;

typedef enum obj_index_bound_inclusion obj_index_bound_inclusion_t;
typedef struct obj_index_bounds_s obj_index_bounds_t;
typedef struct obj_index_bounds_checker_s obj_index_bounds_checker_t;

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


struct obj_index_bounds_checker_s {
    obj_index_bounds_t *bounds;
    /* current interval */
    obj_array_t cur_interval;
    /* expected scan direction */
    obj_array_t expected_direction;
};


void obj_index_bounds_translate(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out);
void obj_index_bounds_translate_and_intersect(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out);
void obj_index_bounds_translate_and_union(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out);
void obj_index_bounds_align_bounds(obj_index_bounds_t *bounds, obj_bson_t *key_pattern);
void obj_index_bounds_dump(obj_index_bounds_t *bounds);

#endif  /* OBJ_INDEX_BOUNDS_H */