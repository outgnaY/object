#include "obj_core.h"

/* print message for debug */
static obj_bool_t obj_bson_visit_print_visit_before(const obj_bson_iter_t *iter, const char *key, void *data);
static obj_bool_t obj_bson_visit_print_visit_double(const obj_bson_iter_t *iter, double v_double, void *data);
static obj_bool_t obj_bson_visit_print_visit_utf8(const obj_bson_iter_t *iter, obj_int32_t len, const char *v_utf8, void *data);
static obj_bool_t obj_bson_visit_print_visit_object(const obj_bson_iter_t *iter, const obj_bson_t *v_object, void *data);
static obj_bool_t obj_bson_visit_print_visit_array(const obj_bson_iter_t *iter, const obj_bson_t *v_array, void *data);
static obj_bool_t obj_bson_visit_print_visit_binary(const obj_bson_iter_t *iter, obj_int32_t len, const obj_uint8_t *v_binary, void *data);
static obj_bool_t obj_bson_visit_print_visit_bool(const obj_bson_iter_t *iter, obj_bool_t v_bool, void *data);
static obj_bool_t obj_bson_visit_print_visit_null(const obj_bson_iter_t *iter, void *data);
static obj_bool_t obj_bson_visit_print_visit_int32(const obj_bson_iter_t *iter, obj_int32_t v_int32, void *data);
static obj_bool_t obj_bson_visit_print_visit_int64(const obj_bson_iter_t *iter, obj_int64_t v_int64, void *data);
static obj_bool_t obj_bson_visit_print_visit_all(const obj_bson_t *bson);

/* validate bson */
static obj_bool_t obj_bson_visit_validate_visit_before(const obj_bson_iter_t *iter, const char *key, void *data);
static obj_bool_t obj_bson_visit_validate_visit_utf8(const obj_bson_iter_t *iter, obj_int32_t len, const char *v_utf8, void *data);
static obj_bool_t obj_bson_visit_validate_visit_object(const obj_bson_iter_t *iter, const obj_bson_t *v_object, void *data);
static obj_bool_t obj_bson_visit_validate_visit_array(const obj_bson_iter_t *iter, const obj_bson_t *v_array, void *data);
static obj_bool_t obj_bson_visit_validate_visit_all(const obj_bson_t *bson, obj_bson_visit_validate_flag_t flag);


/* print message for debug */
const obj_bson_visit_visitor_t obj_bson_visit_print_visitors = {
    obj_bson_visit_print_visit_before,
    obj_bson_visit_print_visit_double,
    obj_bson_visit_print_visit_utf8,
    obj_bson_visit_print_visit_object,
    obj_bson_visit_print_visit_array,
    obj_bson_visit_print_visit_binary,
    obj_bson_visit_print_visit_bool,
    obj_bson_visit_print_visit_null,
    obj_bson_visit_print_visit_int32,
    obj_bson_visit_print_visit_int64
};

const obj_bson_visit_visitor_t obj_bson_visit_validate_visitors = {
    obj_bson_visit_validate_visit_before,
    NULL,
    obj_bson_visit_validate_visit_utf8,
    obj_bson_visit_validate_visit_object,
    obj_bson_visit_validate_visit_array,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* ---------- print message for debug ---------- */
static obj_bool_t obj_bson_visit_print_visit_before(const obj_bson_iter_t *iter, const char *key, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    if (state->count) {
        printf(",");
    }
    printf("%s:", key);
    state->count++;
}


static obj_bool_t obj_bson_visit_print_visit_double(const obj_bson_iter_t *iter, double v_double, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
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

static obj_bool_t obj_bson_visit_print_visit_utf8(const obj_bson_iter_t *iter, obj_int32_t len, const char *v_utf8, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    int i;
    printf("[utf8]");
    for (i = 0; i < len; i++) {
        printf("%c", v_utf8[i]);
    }
    return true;
}

static obj_bool_t obj_bson_visit_print_visit_object(const obj_bson_iter_t *iter, const obj_bson_t *v_object, void *data) {
    /* printf("visit object\n"); */
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    obj_bson_visit_print_state_t child_state;
    obj_bson_iter_t child;
    printf("[object]");
    if (!obj_bson_iter_init(&child, v_object)) {
        return false;
    }
    printf("{");
    child_state.depth = state->depth + 1;
    child_state.count = 0;
    if (!obj_bson_iter_visit_all(&child, &obj_bson_visit_print_visitors, &child_state)) {
        return false;
    }
    printf("}");
    return true;
}

static obj_bool_t obj_bson_visit_print_visit_array(const obj_bson_iter_t *iter, const obj_bson_t *v_array, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    obj_bson_visit_print_state_t child_state;
    obj_bson_iter_t child;
    printf("[array]");
    if (!obj_bson_iter_init(&child, v_array)) {
        return false;
    }
    printf("[");
    child_state.depth = state->depth + 1;
    child_state.count = 0;
    if (!obj_bson_iter_visit_all(&child, &obj_bson_visit_print_visitors, &child_state)) {
        return false;
    }
    printf("]");
    return true;
}

static obj_bool_t obj_bson_visit_print_visit_binary(const obj_bson_iter_t *iter, obj_int32_t len, const obj_uint8_t *v_binary, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    int i;
    printf("[binary]");
    for (i = 0; i < len; i++) {
        printf("%02x", v_binary[i]);
    }
    return true;
}

static obj_bool_t obj_bson_visit_print_visit_bool(const obj_bson_iter_t *iter, obj_bool_t v_bool, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    printf("[bool]");
    if (v_bool) {
        printf("true");
    } else {
        printf("false");
    }
    return true;
}

static obj_bool_t obj_bson_visit_print_visit_null(const obj_bson_iter_t *iter, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    printf("[null]");
    printf("null");
    return true;
}

static obj_bool_t obj_bson_visit_print_visit_int32(const obj_bson_iter_t *iter, obj_int32_t v_int32, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    printf("[int32]");
    printf("%d", v_int32);
    return true;
}

static obj_bool_t obj_bson_visit_print_visit_int64(const obj_bson_iter_t *iter, obj_int64_t v_int64, void *data) {
    obj_bson_visit_print_state_t *state = (obj_bson_visit_print_state_t *)data;
    printf("[int64]");
    printf("%ld", v_int64);
    return true;
}

/* visit a bson */
static obj_bool_t obj_bson_visit_print_visit_all(const obj_bson_t *bson) {
    obj_bson_visit_print_state_t state;
    obj_bson_iter_t iter;
    state.depth = 0;
    state.count = 0;
    if (!obj_bson_iter_init(&iter, bson)) {
        return false;
    }
    /*
    printf("{");
    if (obj_bson_iter_visit_all(&iter, &obj_bson_visit_print_visitors, &state)) {
        printf("}");
        return true;
    }
    return false;
    */
    return obj_bson_visit_print_visit_object(&iter, bson, &state);
}

obj_bool_t obj_bson_visit_print_visit(const obj_bson_t *bson) {
    return obj_bson_visit_print_visit_all(bson);
}

/* ----------validate bson ---------- */
static obj_bool_t obj_bson_visit_validate_visit_before(const obj_bson_iter_t *iter, const char *key, void *data) {
    obj_bson_visit_validate_state_t *state = (obj_bson_visit_validate_state_t *)data;
    if ((state->flag & OBJ_BSON_VALIDATE_FLAG_EMPTY_KEYS) == 0) {
        if (key[0] == '\0') {
            return false;
        }
    }
    return true;
}
/*
static obj_bool_t obj_bson_visit_validate_visit_double(const obj_bson_iter_t *iter, const char *key, void *data) {

}
*/
static obj_bool_t obj_bson_visit_validate_visit_utf8(const obj_bson_iter_t *iter, obj_int32_t len, const char *v_utf8, void *data) {
    obj_bson_visit_validate_state_t *state = (obj_bson_visit_validate_state_t *)data;
    obj_bool_t allow_null;
    allow_null = ((state->flag & OBJ_BSON_VALIDATE_FLAG_UTF8_ALLOW_NULL) != 0);
    return obj_validate_utf8_string(v_utf8, len, allow_null);
}

static obj_bool_t obj_bson_visit_validate_visit_object(const obj_bson_iter_t *iter, const obj_bson_t *v_object, void *data) {
    obj_bson_visit_validate_state_t *state = (obj_bson_visit_validate_state_t *)data;
    obj_bson_iter_t child;
    obj_bool_t res;
    if (!obj_bson_iter_init(&child, v_object)) {
        return false;
    }
    res = obj_bson_iter_visit_all(&child, &obj_bson_visit_validate_visitors, state);
    return res && state->off_err == -1;
}

static obj_bool_t obj_bson_visit_validate_visit_array(const obj_bson_iter_t *iter, const obj_bson_t *v_array, void *data) {
    obj_bson_visit_validate_state_t *state = (obj_bson_visit_validate_state_t *)data;
    obj_bson_iter_t child;
    obj_bool_t res;
    if (!obj_bson_iter_init(&child, v_array)) {
        return false;
    }
    res = obj_bson_iter_visit_all(&child, &obj_bson_visit_validate_visitors, state);
    return res && state->off_err == -1;
}
/*
static obj_bool_t obj_bson_visit_validate_visit_binary(const obj_bson_iter_t *iter, obj_int32_t len, const obj_uint8_t *v_binary, void *data) {

}

static obj_bool_t obj_bson_visit_validate_visit_bool(const obj_bson_iter_t *iter, obj_bool_t v_bool, void *data) {

}

static obj_bool_t obj_bson_visit_validate_visit_null(const obj_bson_iter_t *iter, void *data) {

}

static obj_bool_t obj_bson_visit_validate_visit_int32(const obj_bson_iter_t *iter, obj_int32_t v_int32, void *data) {

}

static obj_bool_t obj_bson_visit_validate_visit_int64(const obj_bson_iter_t *iter, obj_int64_t v_int64, void *data) {

}
*/

static obj_bool_t obj_bson_visit_validate_visit_all(const obj_bson_t *bson, obj_bson_visit_validate_flag_t flag) {
    obj_bson_visit_validate_state_t state;
    obj_bson_iter_t iter;
    obj_bool_t res;
    state.flag = flag;
    state.off_err = -1;
    if (!obj_bson_iter_init(&iter, bson)) {
        return false;
    }
    res = obj_bson_visit_validate_visit_object(&iter, bson, &state);
    return res && state.off_err == -1;
}

obj_bool_t obj_bson_visit_validate_visit(const obj_bson_t *bson, obj_bson_visit_validate_flag_t flag) {
    return obj_bson_visit_validate_visit_all(bson, flag);
}



