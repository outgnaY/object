#include "obj_core.h"


/* ********** index bounds methods ********** */

/* translate expr to index bounds */
void obj_index_bounds_translate(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_assert(out->intervals.size == 0);
    /* must be compare expression */
    obj_assert(expr->type >= OBJ_EXPR_TYPE_EQ && expr->type <= OBJ_EXPR_TYPE_GTE);
    obj_expr_compare_expr_t *compare_expr = (obj_expr_compare_expr_t *)expr;
    out->name = key;
    obj_interval_t interval;
    obj_bson_value_t *expr_value = &compare_expr->value;
    obj_interval_make_interval(expr_value, &interval, expr->type);
    /* append to out */
    obj_array_push_back(&out->intervals, &interval);
}

/* translate expr and intersect with current intervals */
void obj_index_bounds_translate_and_intersect(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_ordered_interval_list_t oil;
    obj_index_bounds_translate(expr, key, value, index_entry, &oil);
    /* do intersect */
    obj_ordered_interval_list_intersect(out, &oil);
    out->name = oil.name;
}

/* translate expr and union with current intervals */
void obj_index_bounds_translate_and_union(obj_expr_base_expr_t *expr, char *key, obj_bson_value_t *value, obj_query_index_entry_t *index_entry, obj_ordered_interval_list_t *out) {
    obj_ordered_interval_list_t oil;
    obj_index_bounds_translate(expr, key, value, index_entry, &oil);
    /* do union */
    obj_ordered_interval_list_union(out, &oil);
    out->name = oil.name;
}

void obj_index_bounds_align_bounds(obj_index_bounds_t *bounds, obj_bson_t *key_pattern) {
    obj_bson_iter_t iter;
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_init(&iter, key_pattern);
    int index = 0;
    int direction;
    obj_bson_value_t *value = NULL;
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

/* ********** index bounds checker methods ********** */

/* init index bounds checker */
void obj_index_bounds_checker_init(obj_index_bounds_checker_t *checker, obj_index_bounds_t *bounds, obj_bson_t *key_pattern) {
    checker->bounds = bounds;
    obj_array_init(&checker->cur_interval, sizeof(int));
    obj_array_init(&checker->expected_direction, sizeof(int));
    obj_array_resize(&checker->cur_interval, bounds->fields.size);
    int i;
    int initial_interval_index = 0;
    for (i = 0; i < bounds->fields.size; i++) {
        obj_array_set_index(&checker->cur_interval, i, &initial_interval_index);
    }
    obj_bson_iter_t iter;
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_init(&iter, key_pattern);
    obj_bson_value_t *value = NULL;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        obj_assert(bson_type == OBJ_BSON_TYPE_INT32);
        value = obj_bson_iter_value(&iter);
        obj_array_push_back(&checker->expected_direction, &value->value.v_int32);
    }
}


/* get start index seek point */
obj_bool_t obj_index_bounds_checker_get_start_seek_point(obj_index_bounds_checker_t *checker, obj_index_seek_point_t *out) {
    out->key_prefix = NULL;
    out->prefix_len = 0;
    out->prefix_exclusive = false;
    obj_array_resize(&out->key_suffix, checker->bounds->fields.size);
    obj_array_resize(&out->suffix_inclusive, checker->bounds->fields.size);
    int i;
    obj_ordered_interval_list_t *oil = NULL;
    obj_interval_t *interval;
    obj_bson_value_t *start = NULL;
    obj_bool_t start_inclusive;
    for (i = 0; i < checker->bounds->fields.size; i++) {
        oil = (obj_ordered_interval_list_t *)obj_array_get_index(&checker->bounds->fields, i);
        if (oil->intervals.size == 0) {
            return false;
        }
        /* get first interval */
        interval = (obj_interval_t *)obj_array_get_index(&oil->intervals, 0);
        start = &interval->start;
        start_inclusive = interval->start_inclusive;
        obj_array_set_index(&out->key_suffix, i, &start);
        obj_array_set_index(&out->suffix_inclusive, i, &start_inclusive);
    }
    return true;
}


/* find interval */
obj_interval_location_t obj_index_bounds_checker_find_interval_for_field(obj_bson_value_t *value, obj_ordered_interval_list_t *oil, int expected_direction, int *new_interval_index) {
    int i;
    /* binary search, find lower bound */
    int lo = 0;
    int hi = oil->intervals.size - 1;
    int mid;
    obj_interval_t *interval = NULL;
    /* find left-most BEHIND/WITHIN */
    while (lo < hi) {
        mid = (lo + hi) / 2;
        interval = (obj_interval_t *)obj_array_get_index(&oil->intervals, mid);
        if (!obj_interval_is_key_element_ahead_of_interval(interval, value, expected_direction)) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    obj_assert(lo == hi);
    obj_interval_location_t location = obj_interval_compare_with_key_element(interval, value, expected_direction);
    /* ahead of all intervals */
    if (location == OBJ_INTERVAL_LOCATION_AHEAD) {
        return OBJ_INTERVAL_LOCATION_AHEAD;
    }
    *new_interval_index = lo;
    return location;
}


/* check key */
obj_index_key_state_t obj_index_bounds_checker_check_key(obj_index_bounds_checker_t *checker, obj_bson_t *index_key, obj_index_seek_point_t *out) {
    obj_array_resize(&out->key_suffix, checker->cur_interval.size);
    obj_array_resize(&out->suffix_inclusive, checker->cur_interval.size);
    obj_array_t values;
    obj_array_init(&values, sizeof(obj_bson_value_t));
    obj_bson_iter_t iter;
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_init(&iter, index_key);
    obj_bson_value_t *value = NULL;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        value = obj_bson_iter_value(&iter);
        obj_array_push_back(&values, value);
    }
    obj_assert(values.size == checker->cur_interval.size);
    int first_non_contained_field;
    obj_interval_location_t location;
    int initial_interval_index = 0;
    if (!obj_index_bounds_checker_find_left_most(checker, &values, &first_non_contained_field, &location)) {
        /* all within current interval */
        return OBJ_INDEX_KEY_STATE_VALID;
    }
    if (location == OBJ_INTERVAL_LOCATION_BEHIND) {
        int i;
        for (i = first_non_contained_field; i < checker->cur_interval.size; i++) {
            obj_array_set_index(&checker->cur_interval, i, &initial_interval_index);
        }
        if (!obj_index_bounds_checker_find_left_most(checker, &values, &first_non_contained_field, &location)) {
            /* all within current interval */
            return OBJ_INDEX_KEY_STATE_VALID;
        }
    }
    if (location == OBJ_INTERVAL_LOCATION_BEHIND) {
        out->key_prefix = index_key;
        out->prefix_len = first_non_contained_field;
        out->prefix_exclusive = false;
        int j;
        obj_bson_value_t *start = NULL;
        obj_interval_t *interval;
        int interval_index;
        obj_ordered_interval_list_t *oil = NULL;
        for (j = first_non_contained_field; j < checker->cur_interval.size; j++) {
            oil = (obj_ordered_interval_list_t *)obj_array_get_index(&checker->bounds->fields, j);
            interval_index = obj_array_get_index_value(&checker->cur_interval, j, int);
            interval = (obj_interval_t *)obj_array_get_index(&oil->intervals, interval_index);
            start = &interval->start;
            obj_array_set_index(&out->key_suffix, j, &start);
            obj_array_set_index(&out->suffix_inclusive, j, &interval->start_inclusive);
        }
        return OBJ_INDEX_KEY_STATE_MUST_ADVANCE;
    }
    obj_assert(location == OBJ_INTERVAL_LOCATION_AHEAD);
    while (first_non_contained_field < checker->cur_interval.size) {
        int new_interval_index;
        obj_bson_value_t *key_value = (obj_bson_value_t *)obj_array_get_index(&values, first_non_contained_field);
        obj_ordered_interval_list_t *oil = (obj_ordered_interval_list_t *)obj_array_get_index(&checker->bounds->fields, first_non_contained_field);
        int direction = obj_array_get_index_value(&checker->expected_direction, first_non_contained_field, int);
        obj_interval_location_t where = obj_index_bounds_checker_find_interval_for_field(key_value, oil, direction, &new_interval_index);
        if (where == OBJ_INTERVAL_LOCATION_WITHIN) {
            obj_array_set_index(&checker->cur_interval, first_non_contained_field, &new_interval_index);
            ++first_non_contained_field;
        } else if (where == OBJ_INTERVAL_LOCATION_BEHIND) {
            obj_array_set_index(&checker->cur_interval, first_non_contained_field, &new_interval_index);
            int i;
            for (i = first_non_contained_field + 1; i < checker->cur_interval.size; i++) {
                obj_array_set_index(&checker->cur_interval, i, &initial_interval_index);
            }
            out->key_prefix = index_key;
            out->prefix_len = first_non_contained_field;
            out->prefix_exclusive = false;
            obj_interval_t *interval = NULL;
            obj_bson_value_t *start = NULL;
            int interval_index;
            for (i = first_non_contained_field; i < checker->cur_interval.size; i++) {
                oil = (obj_ordered_interval_list_t *)obj_array_get_index(&checker->bounds->fields, i);
                interval_index = obj_array_get_index_value(&checker->cur_interval, i, int);
                interval = (obj_interval_t *)obj_array_get_index(&oil->intervals, interval_index);
                start = &interval->start;
                obj_array_set_index(&out->key_suffix, i, &start);
                obj_array_set_index(&out->suffix_inclusive, i, &interval->start_inclusive);
            }

            return OBJ_INDEX_KEY_STATE_MUST_ADVANCE;
        } else {
            /* can't advance */
            if (!obj_index_bounds_checker_space_left_to_advance(checker, first_non_contained_field, &values)) {
                return OBJ_INDEX_KEY_STATE_DONE;
            }
            out->key_prefix = index_key;
            out->prefix_len = first_non_contained_field;
            /* !! */
            out->prefix_exclusive = true;
            int i;
            int initial_interval_index = 0;
            for (i = first_non_contained_field; i < checker->cur_interval.size; i++) {
                obj_array_set_index(&checker->cur_interval, i, &initial_interval_index);
            }
            return OBJ_INDEX_KEY_STATE_MUST_ADVANCE;
        }
    }
    return OBJ_INDEX_KEY_STATE_VALID;
}

/* if we can advance any of the first fields_to_check fields and still be in index bounds */
obj_bool_t obj_index_bounds_checker_space_left_to_advance(obj_index_bounds_checker_t *checker, int fields_to_check, obj_array_t *values) {
    int i;
    int interval_index;
    obj_ordered_interval_list_t *oil = NULL;
    obj_interval_t *interval = NULL;
    obj_bson_value_t *value = NULL;
    int direction;
    for (i = 0; i < fields_to_check; i++) {
        interval_index = obj_array_get_index_value(&checker->cur_interval, i, int);
        oil = (obj_ordered_interval_list_t *)obj_array_get_index(&checker->bounds->fields, i);
        /* not the last interval */
        if (interval_index != oil->intervals.size - 1) {
            return true;
        }
        interval = (obj_interval_t *)obj_array_get_index(&oil->intervals, interval_index);
        if (!interval->end_inclusive) {
            return true;
        }
        direction = obj_array_get_index_value(&checker->expected_direction, i, int);
        value = (obj_bson_value_t *)obj_array_get_index(values, i);
        /* not reach the end */
        if (-direction == obj_sgn(obj_interval_value_compare(value, &interval->end))) {
            return true;
        }
    }
    return false;
}

obj_bool_t obj_index_bounds_checker_find_left_most(obj_index_bounds_checker_t *checker, obj_array_t *values, int *where, obj_interval_location_t *what) {
    int i;
    obj_bson_value_t *value = NULL;
    obj_interval_location_t location;
    obj_ordered_interval_list_t *oil = NULL;
    obj_interval_t *interval = NULL;
    int interval_index;
    int direction;
    for (i = 0; i < checker->cur_interval.size; i++) {
        interval_index = obj_array_get_index_value(&checker->cur_interval, i, int);
        direction = obj_array_get_index_value(&checker->expected_direction, i, int);
        value = (obj_bson_value_t *)obj_array_get_index(values, i);
        oil = (obj_ordered_interval_list_t *)obj_array_get_index(&checker->bounds->fields, i);
        interval = (obj_interval_t *)obj_array_get_index(&oil->intervals, interval_index);
        location = obj_interval_compare_with_key_element(interval, value, direction);
        /* first */
        if (location != OBJ_INTERVAL_LOCATION_WITHIN) {
            *where = i;
            *what = location;
            return true;
        }
    }
    return false;
}

