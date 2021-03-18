#include "obj_core.h"

static obj_bson_value_t s_interval_value_min = {OBJ_BSON_TYPE_MIN};
static obj_bson_value_t s_interval_value_max = {OBJ_BSON_TYPE_MAX};

static int obj_interval_value_compare(obj_bson_value_t *value1, obj_bson_value_t *value2);
static obj_bool_t obj_interval_intersects(obj_interval_t *interval1, obj_interval_t *interval2);
static obj_bool_t obj_interval_equals(obj_interval_t *interval1, obj_interval_t *interval2);
static obj_bool_t obj_interval_within(obj_interval_t *interval1, obj_interval_t *interval2);
static obj_bool_t obj_interval_precedes(obj_interval_t *interval1, obj_interval_t *interval2);
static int obj_interval_compare_for_sort(const void *a, const void *b);


/* ********** interval ********** */

/**
 * compare two values, for intervals
 * rules: 
 * for double, int32, int64; compare by their values
 * anything(except for MIN) > MIN; MIN == MIN
 * anything(except for MAX) < MAX; MAX == MAX
 */
static int obj_interval_value_compare(obj_bson_value_t *value1, obj_bson_value_t *value2) {
    obj_bson_type_t type1 = value1->type;
    obj_bson_type_t type2 = value2->type;
    /* obj_assert(type1 == type2); */
    if (type1 >= OBJ_BSON_TYPE_MIN) {
        if (type1 ==OBJ_BSON_TYPE_MIN) {
            if (type2 >= OBJ_BSON_TYPE_MIN) {
                /* type2 special */
                if (type2 == OBJ_BSON_TYPE_MIN) {
                    return 0;
                } else {
                    return -1;
                }
            } else {
                /* type2 not special */
                return -1;
            }
        } else {
            if (type2 >= OBJ_BSON_TYPE_MIN) {
                if (type2 == OBJ_BSON_TYPE_MIN) {
                    return 1;
                } else {
                    return 0;
                }
            } else {
                /* type2 not special */
                return 1;
            }
        }
        
    } else if (type2 >= OBJ_BSON_TYPE_MIN) {
        /* type1 not special, type2 special */
        if (type2  == OBJ_BSON_TYPE_MIN) {
            return 1;
        } else {
            return -1;
        }
    }
    /* type 1 not special, type2 not special */
    obj_assert(type1 == OBJ_BSON_TYPE_INT32 || type1 == OBJ_BSON_TYPE_INT64 || type1 == OBJ_BSON_TYPE_UTF8 || type1 == OBJ_BSON_TYPE_BINARY || type1 == OBJ_BSON_TYPE_DOUBLE);
    switch (type1) {
        case OBJ_BSON_TYPE_DOUBLE: {
            if (value1->value.v_double < value2->value.v_double) {
                return -1;
            } else if (value1->value.v_double > value2->value.v_double) {
                return 1;
            } else {
                return 0;
            }
        }
        case OBJ_BSON_TYPE_UTF8: {
            int common, len1, len2;
            len1 = value1->value.v_utf8.len;
            len2 = value2->value.v_utf8.len;
            common = (len1 < len2 ? len1 : len2);
            int res = obj_memcmp(value1->value.v_utf8.str, value2->value.v_utf8.str, common);
            if (res) {
                return res;
            }
            return len1 - len2;
        }
        case OBJ_BSON_TYPE_BINARY: {
            int common, len1, len2;
            len1 = value1->value.v_binary.len;
            len2 = value2->value.v_binary.len;
            common = (len1 < len2 ? len1 : len2);
            int res = obj_memcmp(value1->value.v_binary.data, value2->value.v_binary.data, common);
            if (res) {
                return res;
            }
            return len1 - len2;
        }
        case OBJ_BSON_TYPE_INT32: {
            if (value1->value.v_int32 < value2->value.v_int32) {
                return -1;
            } else if (value1->value.v_int32 > value2->value.v_int32) {
                return 1;
            } else {
                return 0;
            }
        }
        case OBJ_BSON_TYPE_INT64: {
            if (value1->value.v_int64 < value2->value.v_int64) {
                return -1;
            } else if (value1->value.v_int64 > value2->value.v_int64) {
                return 1;
            } else {
                return 0;
            }
        }
        default:
            obj_assert(0);
    }
}

/* init an interval */
void obj_interval_init(obj_interval_t *interval, obj_bson_t *base, obj_bool_t si, obj_bool_t ei) {
    obj_bson_iter_t iter;
    const char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_init(&iter, base);
    const obj_bson_value_t *value = NULL;
    obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
    value = obj_bson_iter_value(&iter);
    interval->start = *value;
    obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
    value = obj_bson_iter_value(&iter);
    interval->end = *value;
    interval->empty = false;
    interval->start_inclusive = si;
    interval->end_inclusive = ei;
}

/* test if this is an empty interval */
obj_bool_t obj_interval_is_empty(obj_interval_t *interval) {
    /*
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, interval->interval_data);
    return !obj_bson_iter_next(&iter);
    */
    return interval->empty == true;
}

/* test if this interval is a point */
obj_bool_t obj_interval_is_point(obj_interval_t *interval) {
    /* must be both start inclusive and end inclusive and equal value */
    return interval->start_inclusive && interval->end_inclusive && obj_interval_value_compare(&interval->start, &interval->end) == 0;
}

/* test if this interval is null */
obj_bool_t obj_interval_is_null(obj_interval_t *interval) {
    return (!interval->start_inclusive || !interval->end_inclusive) && obj_interval_value_compare(&interval->start, &interval->end) == 0;
}

obj_interval_direction_t obj_interval_get_direction(obj_interval_t *interval) {
    if (obj_interval_is_empty(interval) || obj_interval_is_point(interval) || obj_interval_is_null(interval)) {
        return OBJ_INTERVAL_DIRECTION_NONE;
    }
    int res = obj_interval_value_compare(&interval->start, &interval->end);
    obj_assert(res != 0);
    return (res < 0 ? OBJ_INTERVAL_DIRECTION_ASCENDING : OBJ_INTERVAL_DIRECTION_DESCENDING);
}

/* check if two intervals intersects */
static obj_bool_t obj_interval_intersects(obj_interval_t *interval1, obj_interval_t *interval2) {
    int res = obj_interval_value_compare(&interval1->start, &interval2->end);
    /* case: [2][1] */
    if (res > 0) {
        return false;
    }
    if (res == 0 && (!interval1->start_inclusive || !interval2->end_inclusive)) {
        return false;
    }
    res = obj_interval_value_compare(&interval2->start, &interval1->end);
    /* case: [1][2] */
    if (res > 0) {
        return false;
    }
    if (res == 0 && (!interval2->start_inclusive || !interval1->end_inclusive)) {
        return false;
    }
    return true;
}

/* check if two intervals equals */
static obj_bool_t obj_interval_equals(obj_interval_t *interval1, obj_interval_t *interval2) {
    if (interval1->start_inclusive != interval2->start_inclusive) {
        return false;
    }
    if (interval1->end_inclusive != interval2->end_inclusive) {
        return false;
    }
    int res = obj_interval_value_compare(&interval1->start, &interval2->start);
    if (res != 0) {
        return false;
    }
    res = obj_interval_value_compare(&interval1->end, &interval2->end);
    if (res != 0) {
        return false;
    }
    return true;
}

/* check if interval1 is within interval2 */
static obj_bool_t obj_interval_within(obj_interval_t *interval1, obj_interval_t *interval2) {
    int res = obj_interval_value_compare(&interval1->start, &interval2->start);
    if (res < 0) {
        return false;
    } else if (res == 0 && interval1->start_inclusive && !interval2->start_inclusive) {
        return false;
    }
    res = obj_interval_value_compare(&interval1->end, &interval2->end);
    if (res > 0) {
        return false;
    } else if (res == 0 && interval1->end_inclusive && !interval2->end_inclusive) {
        return false;
    }
    return true;
}

static obj_bool_t obj_interval_precedes(obj_interval_t *interval1, obj_interval_t *interval2) {
    int res = obj_interval_value_compare(&interval1->start, &interval2->start);
    if (res < 0) {
        return true;
    } else if (res == 0 && interval1->start_inclusive && !interval2->start_inclusive) {
        return true;
    }
    return false;
}

/* compare two intervals */
obj_interval_compare_result_t obj_interval_compare(obj_interval_t *interval1, obj_interval_t *interval2) {
    /* intersect */
    if (obj_interval_intersects(interval1, interval2)) {
        if (obj_interval_equals(interval1, interval2)) {
            return OBJ_INTERVAL_COMPARE_RESULT_EQUALS;
        }
        if (obj_interval_within(interval1, interval2)) {
            return OBJ_INTERVAL_COMAPRE_RESULT_WITHIN;
        }
        if (obj_interval_within(interval2, interval1)) {
            return OBJ_INTERVAL_COMAPRE_RESULT_CONTAINS;
        }
        if (obj_interval_precedes(interval1, interval2)) {
            return OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_BEFORE;
        }
        return OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_AFTER;
    }
    /* not intersect */
    if (obj_interval_precedes(interval1, interval2)) {
        if ((interval1->end_inclusive || interval2->start_inclusive) && obj_interval_value_compare(&interval1->end, &interval2->start) == 0) {
            /* could union */
            return OBJ_INTERVAL_COMPARE_RESULT_PRECEDES_COULD_UNION;
        }
        return OBJ_INTERVAL_COMPARE_RESULT_PRECEDES;
    }
    return OBJ_INTERVAL_COMPARE_RESULT_SUCCEEDS;
}

void obj_interval_intersect(obj_interval_t *interval1, obj_interval_t *interval2, obj_interval_compare_result_t cmp) {
    /* !may modify interval1 */
    switch (cmp) {
        case OBJ_INTERVAL_COMPARE_RESULT_EQUALS:
        case OBJ_INTERVAL_COMAPRE_RESULT_WITHIN:
            break;
        case OBJ_INTERVAL_COMAPRE_RESULT_CONTAINS:
            /* interval2 */
            obj_memcpy(interval1, interval2, sizeof(obj_interval_t));
            break;
        case OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_BEFORE:
            interval1->start = interval2->start;
            interval1->start_inclusive = interval2->start_inclusive;
            break;
        case OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_AFTER:
            interval1->end = interval2->end;
            interval1->end_inclusive = interval2->end_inclusive;
            break;
        default:
            obj_assert(0);
    }
}

/* sort by start */
static int obj_interval_compare_for_sort(const void *a, const void *b) {
    obj_interval_t *interval1 = (obj_interval_t *)a;
    obj_interval_t *interval2 = (obj_interval_t *)b;
    int res = obj_interval_value_compare(&interval1->start, &interval2->start);
    if (res != 0) {
        return res;
    }
    if (interval1->start_inclusive == interval2->start_inclusive) {
        return -1;
    }
    if (interval1->start_inclusive) {
        return 1;
    }
    return -1;
}

/* dump interval */
void obj_interval_dump(obj_interval_t *interval) {
    printf("start:\n");
    switch (interval->start.type) {
        case OBJ_BSON_TYPE_DOUBLE:
            printf("%lf\n", interval->start.value.v_double);
            break;
        case OBJ_BSON_TYPE_UTF8:
            printf("%s\n", interval->start.value.v_utf8.str);
            break;
        case OBJ_BSON_TYPE_BINARY:
            break;
        case OBJ_BSON_TYPE_INT32:
            printf("%d\n", interval->start.value.v_int32);
            break;
        case OBJ_BSON_TYPE_INT64:
            printf("%ld\n", interval->start.value.v_int64);
            break;
        default:
            obj_assert(0);
    }
    printf("start inclusive: %d\n", interval->start_inclusive);
    
    printf("end\n");
    switch (interval->end.type) {
        case OBJ_BSON_TYPE_DOUBLE:
            printf("%lf\n", interval->end.value.v_double);
            break;
        case OBJ_BSON_TYPE_UTF8:
            printf("%s\n", interval->end.value.v_utf8.str);
            break;
        case OBJ_BSON_TYPE_BINARY:
            break;
        case OBJ_BSON_TYPE_INT32:
            printf("%d\n", interval->end.value.v_int32);
            break;
        case OBJ_BSON_TYPE_INT64:
            printf("%ld\n", interval->end.value.v_int64);
            break;
        default:
            obj_assert(0);
    }
    printf("end inclusive: %d\n", interval->end_inclusive);
}

/* for translate */
void obj_interval_make_interval(obj_bson_value_t *value, obj_interval_t *interval, obj_expr_type_t expr_type) {
    interval->empty = false;
    switch (expr_type) {
        case OBJ_EXPR_TYPE_EQ:
            interval->start_inclusive = true;
            interval->end_inclusive = true;
            interval->start = *value;
            interval->end = *value;
            break;
        case OBJ_EXPR_TYPE_LT:
            /* (MIN, value) */
            interval->start_inclusive = false;
            interval->end_inclusive = false;
            interval->start = s_interval_value_min;
            interval->end = *value;
            break;
        case OBJ_EXPR_TYPE_LTE:
            /* (MIN, value] */
            interval->start_inclusive = false;
            interval->end_inclusive = true;
            interval->start = s_interval_value_min;
            interval->end = *value;
            break;
        case OBJ_EXPR_TYPE_GT:
            /* (value, MAX) */
            interval->start_inclusive = false;
            interval->end_inclusive = false;
            interval->start = *value;
            interval->end = s_interval_value_max;
            break;
        case OBJ_EXPR_TYPE_GTE:
            /* [value, MAX) */
            interval->start_inclusive = true;
            interval->end_inclusive = false;
            interval->start = *value;
            interval->end = s_interval_value_max;
            break;
        default:
            obj_assert(0);
    }
}


/* ********** ordered interval list ********** */

/* 
 * intersect two ordered interval lists oil1 and oil2, store result in oil1 
 * TODO optimize
 */
void obj_ordered_interval_list_intersect(obj_ordered_interval_list_t *oil1, obj_ordered_interval_list_t *oil2) {
    int index1 = 0;
    int index2 = 0;
    obj_array_t *oil1_intervals = &oil1->intervals;
    obj_array_t *oil2_intervals = &oil2->intervals;
    obj_array_t result;
    if (!obj_array_init(&result, sizeof(obj_interval_t))) {
        return;
    }
    while (index1 < oil1_intervals->size && index2 < oil2_intervals->size) {
        obj_interval_t *interval1 = (obj_interval_t *)obj_array_get_index(oil1_intervals, index1);
        obj_interval_t *interval2 = (obj_interval_t *)obj_array_get_index(oil2_intervals, index2);
        obj_interval_compare_result_t cmp = obj_interval_compare(interval1, interval2);
        if (cmp == OBJ_INTERVAL_COMPARE_RESULT_PRECEDES || cmp == OBJ_INTERVAL_COMPARE_RESULT_PRECEDES_COULD_UNION) {
            ++index1;
        } else if (cmp == OBJ_INTERVAL_COMPARE_RESULT_SUCCEEDS) {
            ++index2;
        } else {
            /* intersect */
            obj_interval_intersect(interval1, interval2, cmp);
            /* add intersect to result */
            obj_array_push_back(&result, interval1);
            if (cmp == OBJ_INTERVAL_COMPARE_RESULT_EQUALS) {
                ++index1;
                ++index2;
            } else if (cmp == OBJ_INTERVAL_COMAPRE_RESULT_WITHIN) {
                ++index1;
            } else if (cmp == OBJ_INTERVAL_COMAPRE_RESULT_CONTAINS) {
                ++index2;
            } else if (cmp == OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_BEFORE) {
                ++index1;
            } else if (cmp == OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_AFTER) {
                ++index2;
            } else {
                obj_assert(0);
            }
        }
    }
    /* move result to oil1 */
    obj_free(oil1_intervals->data);
    obj_memcpy(oil1_intervals, &result, sizeof(obj_array_t));
}

/* 
 * union two ordered interval lists oil1 and oil2, store result in oil1 
 * TODO optimize
 */
void obj_ordered_interval_list_union(obj_ordered_interval_list_t *oil1, obj_ordered_interval_list_t *oil2) {
    /* append oil2 to oil1 */
    obj_array_t *oil1_intervals = &oil1->intervals;
    obj_array_t *oil2_intervals = &oil2->intervals;
    if (!obj_array_reserve(oil1_intervals, oil1_intervals->size + oil2_intervals->size)) {
        return;
    }
    int i = 0;
    for (i = 0; i < oil2_intervals->size; i++) {
        obj_interval_t *interval = (obj_interval_t *)obj_array_get_index(oil2_intervals, i);
        obj_array_push_back(oil1_intervals, interval);
    }
    /* sort */
    qsort(oil1_intervals->data, oil1_intervals->size, sizeof(obj_interval_t), obj_interval_compare_for_sort);
    /*
    printf("total size: %d\n", oil1_intervals->size);
    for (i = 0; i < oil1_intervals->size; i++) {
        obj_interval_t *interval = (obj_interval_t *)obj_array_get_index(oil1_intervals, i);
        obj_interval_dump(interval);
    }
    */
    /* merge */
    obj_array_t result;
    if (!obj_array_init(&result, sizeof(obj_interval_t))) {
        return;
    }
    obj_interval_t *cur = NULL;
    obj_interval_t *next = NULL;
    obj_interval_t merge;
    merge.empty = false;
    i = 0;
    while (i < oil1_intervals->size - 1) {
        /* get ith and i+1 th interval */
        if (cur == NULL) {
            cur = (obj_interval_t *)obj_array_get_index(oil1_intervals, i);
        }
        next = (obj_interval_t *)obj_array_get_index(oil1_intervals, i + 1);
        obj_interval_compare_result_t cmp = obj_interval_compare(cur, next);
        if (cmp == OBJ_INTERVAL_COMPARE_RESULT_PRECEDES) {
            obj_array_push_back(&result, cur);
            cur = NULL;
        } else if (cmp == OBJ_INTERVAL_COMPARE_RESULT_EQUALS || cmp == OBJ_INTERVAL_COMAPRE_RESULT_WITHIN) {
            /* current invalid */
            cur = NULL;
        } else if (cmp == OBJ_INTERVAL_COMAPRE_RESULT_CONTAINS) {
            /* next invalid */
        } else if (cmp == OBJ_INTERVAL_COMPARE_RESULT_OVERLAP_BEFORE || cmp == OBJ_INTERVAL_COMPARE_RESULT_PRECEDES_COULD_UNION) {
            /* merge cur and next */
            merge.start_inclusive = cur->start_inclusive;
            merge.end_inclusive = next->end_inclusive;
            merge.start = cur->start;
            merge.end = next->end;
            cur = &merge;
        } else {
            obj_assert(0);
        }
        i++;
    }
    if (cur != NULL) {
        obj_array_push_back(&result, cur);
    }
    /* move result to oil1 */
    obj_free(oil1_intervals->data);
    obj_memcpy(oil1_intervals, &result, sizeof(obj_array_t));
}
/*
obj_bool_t obj_ordered_interval_list_init(obj_ordered_interval_list_t *oil) {
    return obj_array_init(&oil->intervals, sizeof(obj_interval_t));
}
*/