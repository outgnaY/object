#include "obj_core.h"


static obj_bool_t obj_bson_print_visit_before(const obj_bson_iter_t *iter, const char *key, void *data);
static obj_bool_t obj_bson_print_visit_double(const obj_bson_iter_t *iter, const char *key, double v_double, void *data);
static obj_bool_t obj_bson_print_visit_utf8(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const char *v_utf8, void *data);
static obj_bool_t obj_bson_print_visit_object(const obj_bson_iter_t *iter, const char *key, const obj_bson_t *v_object, void *data);
static obj_bool_t obj_bson_print_visit_array(const obj_bson_iter_t *iter, const char *key, const obj_bson_t *v_array, void *data);
static obj_bool_t obj_bson_print_visit_binary(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const obj_uint8_t *v_binary, void *data);
static obj_bool_t obj_bson_print_visit_bool(const obj_bson_iter_t *iter, const char *key, obj_bool_t v_bool, void *data);
static obj_bool_t obj_bson_print_visit_null(const obj_bson_iter_t *iter, const char *key, void *data);
static obj_bool_t obj_bson_print_visit_int32(const obj_bson_iter_t *iter, const char *key, obj_int32_t v_int32, void *data);
static obj_bool_t obj_bson_print_visit_int64(const obj_bson_iter_t *iter, const char *key, obj_int64_t v_int64, void *data);
static obj_bool_t obj_bson_print_visit_all(const obj_bson_t *bson);


const obj_bson_visitor_t obj_bson_print_visitors = {
    obj_bson_print_visit_before,
    NULL,
    obj_bson_print_visit_double,
    obj_bson_print_visit_utf8,
    obj_bson_print_visit_object,
    obj_bson_print_visit_array,
    obj_bson_print_visit_binary,
    obj_bson_print_visit_bool,
    obj_bson_print_visit_null,
    obj_bson_print_visit_int32,
    obj_bson_print_visit_int64
};

/* print message for debug */

static obj_bool_t obj_bson_print_visit_before(const obj_bson_iter_t *iter, const char *key, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    if (state->count) {
        printf(",");
    }
    printf("%s:", key);
    state->count++;
}


static obj_bool_t obj_bson_print_visit_double(const obj_bson_iter_t *iter, const char *key, double v_double, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    printf("[double]");
    if (v_double != v_double) {
        printf("NaN");
        
    } else if (v_double * 0 != 0) {
        if (v_double > 0) {
            printf("Inf");
        } else {
            printf("-Inf");
        }
    } else {
        printf("%lf", v_double);
    }
    return true;
}

static obj_bool_t obj_bson_print_visit_utf8(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const char *v_utf8, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    int i;
    printf("[utf8]");
    for (i = 0; i < len; i++) {
        printf("%c", v_utf8[i]);
    }
    return true;
}

static obj_bool_t obj_bson_print_visit_object(const obj_bson_iter_t *iter, const char *key, const obj_bson_t *v_object, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    obj_bson_print_state_t child_state;
    obj_bson_iter_t child;
    printf("[object]");
    if (obj_bson_iter_init(&child, v_object)) {
        printf("{");
        child_state.depth = state->depth + 1;
        child_state.count = 0;
        if (!obj_bson_iter_visit_all(&child, &obj_bson_print_visitors, &child_state)) {
            return false;
        }
        printf("}");
        return true;
    }
    return false;
}

static obj_bool_t obj_bson_print_visit_array(const obj_bson_iter_t *iter, const char *key, const obj_bson_t *v_array, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    obj_bson_print_state_t child_state;
    obj_bson_iter_t child;
    printf("[array]");
    if (obj_bson_iter_init(&child, v_array)) {
        printf("[");
        child_state.depth = state->depth + 1;
        child_state.count = 0;
        if (!obj_bson_iter_visit_all(&child, &obj_bson_print_visitors, &child_state)) {
            return false;
        }
        printf("]");
        return true;
    }
    return false;
}

static obj_bool_t obj_bson_print_visit_binary(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const obj_uint8_t *v_binary, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    int i;
    printf("[binary]");
    for (i = 0; i < len; i++) {
        printf("%02x", v_binary[i]);
    }
    return true;
}

static obj_bool_t obj_bson_print_visit_bool(const obj_bson_iter_t *iter, const char *key, obj_bool_t v_bool, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    printf("[bool]");
    if (v_bool) {
        printf("true");
    } else {
        printf("false");
    }
    return true;
}

static obj_bool_t obj_bson_print_visit_null(const obj_bson_iter_t *iter, const char *key, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    printf("[null]");
    printf("null");
    return true;
}

static obj_bool_t obj_bson_print_visit_int32(const obj_bson_iter_t *iter, const char *key, obj_int32_t v_int32, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    printf("[int32]");
    printf("%d", v_int32);
    return true;
}

static obj_bool_t obj_bson_print_visit_int64(const obj_bson_iter_t *iter, const char *key, obj_int64_t v_int64, void *data) {
    obj_bson_print_state_t *state = (obj_bson_print_state_t *)data;
    printf("[int64]");
    printf("%lld", v_int64);
    return true;
}

/* visit a bson */
static obj_bool_t obj_bson_print_visit_all(const obj_bson_t *bson) {
    obj_bson_print_state_t state;
    obj_bson_iter_t iter;
    state.depth = 0;
    state.count = 0;
    if (!obj_bson_iter_init(&iter, bson)) {
        return false;
    }
    printf("{");
    if (obj_bson_iter_visit_all(&iter, &obj_bson_print_visitors, &state)) {
        printf("}");
        return true;
    }
    return false;
}

obj_bool_t obj_bson_print_visit(const obj_bson_t *bson) {
    return obj_bson_print_visit_all(bson);
}


/* visit until an error occurs */
obj_bool_t obj_bson_iter_visit_all(obj_bson_iter_t *iter, const obj_bson_visitor_t *visitor, void *data) {
    obj_bson_type_t bson_type;
    const char *key;
    /* loop until  */
    while (obj_bson_iter_next_internal(iter, &key, &bson_type)) {
        /* TODO validate key */
        /* if (*key ) */
        if (visitor->visit_before && !visitor->visit_before(iter, key, data)) {
            return false;
        }
        switch (bson_type) {
            /* can't reach here */
            case OBJ_BSON_TYPE_EOO:
                break;
            case OBJ_BSON_TYPE_DOUBLE:
                if (visitor->visit_double && !visitor->visit_double(iter, key, obj_bson_iter_double(iter), data)) {
                    return false;
                }
                break;
            case OBJ_BSON_TYPE_UTF8: {
                const char *utf8;
                obj_int32_t utf8_len;
                utf8 = obj_bson_iter_utf8(iter, &utf8_len);
                /* TODO validate utf8 string */
                if (visitor->visit_utf8 && !visitor->visit_utf8(iter, key, utf8_len, utf8, data)) {
                    return false;
                }
                break;
            }
            case OBJ_BSON_TYPE_OBJECT: {
                obj_int32_t len;
                const obj_uint8_t *buf;
                buf = obj_bson_iter_object(iter, &len);
                obj_bson_t bson;
                if (!obj_bson_init_static(&bson, buf, len)) {
                    return false;
                }
                if (visitor->visit_object && !visitor->visit_object(iter, key, &bson, data)) {
                    return false;
                }
                break;
            }
            case OBJ_BSON_TYPE_ARRAY: {
                obj_int32_t len;
                const obj_uint8_t *buf;
                buf = obj_bson_iter_array(iter, &len);
                obj_bson_t bson;
                if (!obj_bson_init_static(&bson, buf, len)) {
                    return false;
                }
                if (visitor->visit_array && !visitor->visit_array(iter, key, &bson, data)) {
                    return false;
                }
                break;
            }
            case OBJ_BSON_TYPE_BINARY: {
                const obj_uint8_t *binary;
                obj_int32_t binary_len;
                binary = obj_bson_iter_binary(iter, &binary_len);
                if (visitor->visit_binary && !visitor->visit_binary(iter, key, binary_len, binary, data)) {
                    return false;
                }
                break;
            }
            case OBJ_BSON_TYPE_BOOL:
                if (visitor->visit_bool && !visitor->visit_bool(iter, key, obj_bson_iter_bool(iter), data)) {
                    return false;
                }
                break;
            case OBJ_BSON_TYPE_NULL:
                if (visitor->visit_null && !visitor->visit_null(iter, key, data)) {
                    return false;
                }
                break;
            case OBJ_BSON_TYPE_INT32:
                if (visitor->visit_int32 && !visitor->visit_int32(iter, key, obj_bson_iter_int32(iter), data)) {
                    return false;
                }
                break;
            case OBJ_BSON_TYPE_INT64:
                if (visitor->visit_int64 && !visitor->visit_int64(iter, key, obj_bson_iter_int64(iter), data)) {
                    return false;
                }
                break;
            default:
                break;
        }
    }
    return true;
}

