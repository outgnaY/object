#ifndef OBJ_INTERVAL_H
#define OBJ_INTERVAL_H

#include "obj_core.h"

typedef enum obj_interval_direction obj_interval_direction_t;
typedef enum obj_interval_compare_result obj_interval_compare_result_t;
typedef enum obj_interval_location obj_interval_location_t;
typedef struct obj_interval_s obj_interval_t;
typedef struct obj_ordered_interval_list_s obj_ordered_interval_list_t;


enum obj_interval_direction {
    OBJ_INTERVAL_DIRECTION_NONE,
    OBJ_INTERVAL_DIRECTION_ASCENDING,
    OBJ_INTERVAL_DIRECTION_DESCENDING
};

enum obj_interval_compare_result {
    OBJ_INTERVAL_COMPARE_RESULT_EQUALS,
    OBJ_INTERVAL_COMAPRE_RESULT_CONTAINS,
    OBJ_INTERVAL_COMAPRE_RESULT_WITHIN,
    OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_BEFORE,
    OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_AFTER,
    OBJ_INTERVAL_COMPARE_RESULT_PRECEDES,
    OBJ_INTERVAL_COMPARE_RESULT_PRECEDES_COULD_UNION,
    OBJ_INTERVAL_COMPARE_RESULT_SUCCEEDS,
    OBJ_INTERVAL_COMPARE_RESULT_UNKNOWN
};

/* relative position of an index key element to an interval */
enum obj_interval_location {
    OBJ_INTERVAL_LOCATION_BEHIND = -1,
    OBJ_INTERVAL_LOCATION_WITHIN = 0,
    OBJ_INTERVAL_LOCATION_AHEAD = 1
};

/* a range of values for one field */
struct obj_interval_s {
    /* obj_bson_t *interval_data; */
    obj_bson_value_t start;
    obj_bson_value_t end;
    obj_bool_t start_inclusive;
    obj_bool_t end_inclusive;
    obj_bool_t empty;
};

/* an ordered list of intervals for one field */
struct obj_ordered_interval_list_s {
    obj_array_t intervals;
    char *name;
};

extern obj_bson_value_t g_interval_value_min;
extern obj_bson_value_t g_interval_value_max;

/* interval methods */
int obj_interval_value_compare(obj_bson_value_t *value1, obj_bson_value_t *value2);;
void obj_interval_init(obj_interval_t *interval, obj_bson_t *base, obj_bool_t si, obj_bool_t ei);
obj_bool_t obj_interval_is_empty(obj_interval_t *interval);
obj_bool_t obj_interval_is_point(obj_interval_t *interval);
obj_bool_t obj_interval_is_null(obj_interval_t *interval);
obj_interval_direction_t obj_interval_get_direction(obj_interval_t *interval);
obj_bool_t obj_interval_equals(obj_interval_t *interval1, obj_interval_t *interval2);
obj_interval_compare_result_t obj_interval_compare(obj_interval_t *interval1, obj_interval_t *interval2);
void obj_interval_intersect(obj_interval_t *interval1, obj_interval_t *interval2, obj_interval_compare_result_t cmp);
void obj_interval_dump(obj_interval_t *interval);
void obj_interval_make_interval(obj_bson_value_t *value, obj_interval_t *interval, obj_expr_type_t expr_type);
void obj_interval_reverse(obj_interval_t *interval);
obj_interval_location_t obj_interval_compare_with_key_element(obj_interval_t *interval, obj_bson_value_t *value, int expected_direction);
obj_bool_t obj_interval_is_key_element_ahead_of_interval(obj_interval_t *interval, obj_bson_value_t *value, int expected_direction);
/*  ordered interval list methods*/
void obj_ordered_interval_list_intersect(obj_ordered_interval_list_t *oil1, obj_ordered_interval_list_t *oil2);
void obj_ordered_interval_list_union(obj_ordered_interval_list_t *oil1, obj_ordered_interval_list_t *oil2);
void obj_ordered_interval_list_init(obj_ordered_interval_list_t *oil);
void obj_ordered_interval_list_destroy_static(obj_ordered_interval_list_t *oil);
void obj_ordered_interval_list_reverse(obj_ordered_interval_list_t *oil);
void obj_ordered_interval_list_dump(obj_ordered_interval_list_t *oil);

#endif  /* OBJ_INTERVAL_H */