#ifndef OBJ_INDEX_BOUNDS_H
#define OBJ_INDEX_BOUNDS_H

#include "obj_core.h"

/* forward declaration */

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

/* index bounds */
void obj_index_bounds_translate(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_ordered_interval_list_t *out);
void obj_index_bounds_translate_and_intersect(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_ordered_interval_list_t *out);
void obj_index_bounds_translate_and_union(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_ordered_interval_list_t *out);
void obj_index_bounds_align_bounds(obj_index_bounds_t *bounds, obj_bson_t *key_pattern);
void obj_index_bounds_dump(obj_index_bounds_t *bounds);
obj_bool_t obj_index_bounds_is_single_interval(obj_index_bounds_t *bounds, obj_bson_t **start_key_out, obj_bool_t *start_key_inclusive, obj_bson_t **end_key_out, obj_bool_t *end_key_inclusive);

/* index bounds checker */
obj_index_bounds_checker_t *obj_index_bounds_checker_create(obj_index_bounds_t *bounds, obj_bson_t *key_pattern);
void obj_index_bounds_checker_init(obj_index_bounds_checker_t *checker, obj_index_bounds_t *bounds, obj_bson_t *key_pattern);
obj_bool_t obj_index_bounds_checker_get_start_seek_point(obj_index_bounds_checker_t *checker, obj_index_seek_point_t *out);
obj_index_key_state_t obj_index_bounds_checker_check_key(obj_index_bounds_checker_t *checker, obj_bson_t *index_key, obj_index_seek_point_t *out);



#endif  /* OBJ_INDEX_BOUNDS_H */