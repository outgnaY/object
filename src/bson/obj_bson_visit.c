#include "obj_core.h"


/* bson to json, for debug */

static obj_bool_t obj_bson_to_json_visit_double(const obj_bson_iter_t *iter, const char *key, double v_double, void *data) {
    obj_bson_to_json_state_t *state = (obj_bson_to_json_state_t *)data;
    /* ??? by libbson */
    if (v_double != v_double) {
        return obj_sds_cat(state->str, "NaN") != NULL;
    } else if (v_double * 0 != 0) {
        if (v_double > 0) {
            return obj_sds_cat(state->str, "Inf") != NULL;
        } else {
            return obj_sds_cat(state->str, "-Inf") != NULL;
        }
    } else {
        return obj_sds_catprintf(state->str, "%lf", v_double) != NULL;
    }
}

static obj_bool_t obj_bson_to_json_visit_utf8(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const char *v_utf8, void *data) {

}

static obj_bool_t obj_bson_to_json_visit_object(const obj_bson_iter_t *iter, const char *key, void *data) {

}

static obj_bool_t obj_bson_to_json_visit_array(const obj_bson_iter_t *iter, const char *key, void *data) {

}

static obj_bool_t obj_bson_to_json_visit_binary(const obj_bson_iter_t *iter, const char *key, obj_int32_t len, const obj_uint8_t *v_binary, void *data) {
    
}

static obj_bool_t obj_bson_to_json_visit_bool(const obj_bson_iter_t *iter, const char *key, obj_bool_t v_bool, void *data) {
    obj_bson_to_json_state_t *state = (obj_bson_to_json_state_t *)data;
    return obj_sds_cat(state->str, (v_bool ? "true" : "false")) != NULL;
}

static obj_bool_t obj_bson_to_json_visit_null(const obj_bson_iter_t *iter, const char *key, void *data) {
    obj_bson_to_json_state_t *state = (obj_bson_to_json_state_t *)data;
    return obj_sds_cat(state->str, "null") != NULL;
}

static obj_bool_t obj_bson_to_json_visit_int32(const obj_bson_iter_t *iter, const char *key, obj_int32_t v_int32, void *data) {
    obj_bson_to_json_state_t *state = (obj_bson_to_json_state_t *)data;
    return obj_sds_catprintf(state->str, "%d", v_int32) != NULL;
}

static obj_bool_t obj_bson_to_json_visit_int64(const obj_bson_iter_t *iter, const char *key, obj_int64_t v_int64, void *data) {
    obj_bson_to_json_state_t *state = (obj_bson_to_json_state_t *)data;
    return obj_sds_catprintf(state->str, "%lld", v_int64) != NULL;
}

/* visitor */
obj_bool_t obj_bson_iter_visit_all(obj_bson_iter_t *iter, const obj_bson_visitor_t *visitor, void *data) {
    obj_bson_type_t bson_type;
    const char *key;
    /* loop until  */
    while (obj_bson_iter_next_internal(iter, &key, &bson_type)) {
        /* validate key */
        /* if (*key ) */
        switch (bson_type) {
            /* can't reach here */
            case OBJ_BSON_TYPE_EOO:
                break;
            case OBJ_BSON_TYPE_DOUBLE:
                if (visitor->visit_double && visitor->visit_double(iter, key, obj_bson_iter_double(iter), data)) {
                    return true;
                }
                break;
            case OBJ_BSON_TYPE_UTF8: {
                const char *utf8;
                obj_int32_t utf8_len;
                utf8 = obj_bson_iter_utf8(iter, &utf8_len);
                /* TODO validate utf8 string */
                if (visitor->visit_utf8 && visitor->visit_utf8(iter, key, utf8_len, utf8, data)) {
                    return true;
                }
                break;
            }
            case OBJ_BSON_TYPE_OBJECT:

            case OBJ_BSON_TYPE_ARRAY:

            case OBJ_BSON_TYPE_BINARY: {
                const obj_uint8_t *binary;
                obj_int32_t binary_len;
                binary = obj_bson_iter_binary(iter, &binary_len);
                if (visitor->visit_binary && visitor->visit_binary(iter, key, binary_len, binary, data)) {
                    return true;
                }
                break;
            }
            case OBJ_BSON_TYPE_BOOL:
                if (visitor->visit_bool && visitor->visit_bool(iter, key, obj_bson_iter_bool(iter), data)) {
                    return true;
                }
                break;
            case OBJ_BSON_TYPE_NULL:
                if (visitor->visit_null && visitor->visit_null(iter, key, data)) {
                    return true;
                }
                break;
            case OBJ_BSON_TYPE_INT32:
                if (visitor->visit_int32 && visitor->visit_int32(iter, key, obj_bson_iter_int32(iter), data)) {
                    return true;
                }
                break;
            case OBJ_BSON_TYPE_INT64:
                if (visitor->visit_int64 && visitor->visit_int64(iter, key, obj_bson_iter_int64(iter), data)) {
                    return true;
                }
                break;
            default:
                break;
        }
    }
}