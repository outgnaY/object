#include "obj_core.h"


/* for macro */
typedef obj_int8_t int8;
typedef obj_uint8_t uint8;
typedef obj_int32_t int32;

static obj_global_error_code_t obj_bson_validate_read_cstring(obj_bson_validate_t *validate);
static obj_global_error_code_t obj_bson_validate_read_string(obj_bson_validate_t *validate);
static obj_bool_t obj_bson_validate_skip(obj_bson_validate_t *validate, int size);
static obj_bson_validate_frame_t *obj_bson_validate_frame_init();
static obj_global_error_code_t obj_bson_validate_kv(obj_bson_validate_t *validate, obj_bson_validate_state_t *next_state);
static obj_global_error_code_t obj_bson_validate_internal(obj_bson_validate_t *validate);


/* test if we can read a given type of number */
#define obj_bson_validate_can_read_number(validate, type) (!((((validate)->position) + sizeof(type)) > ((validate)->len)))

/* read a given type of number */
#define obj_bson_validate_read_number(validate, type, out) {\
    *(out) = (type)obj_to_le(obj_bson_validate_unsafe_load_number((((validate)->data) + ((validate)->position)), type), type);\
    (validate)->position += sizeof(type);\
}\

/* load a number of a given type */
#define obj_bson_validate_unsafe_load_number(ptr, type) (type)(*((type *)(ptr)))

/* try to read a c-string(field name) */
static obj_global_error_code_t obj_bson_validate_read_cstring(obj_bson_validate_t *validate) {
    /* search NULL */
    obj_uint8_t *x = obj_memchr(validate->data + validate->position, 0, validate->len - validate->position);
    if (!x) {
        return OBJ_CODE_BSON_NOT_NULL_TERMINATED_CSTRING;
    }
    /* calculate length */
    int len = x - (validate->data + validate->position);
    validate->position += len + 1;
    return OBJ_CODE_OK;
}

/* try to read a utf-8 string */
static obj_global_error_code_t obj_bson_validate_read_string(obj_bson_validate_t *validate) {
    int32 size;
    if (!obj_bson_validate_can_read_number(validate, int32)) {
        return OBJ_CODE_BSON_INVALID_BSON;
    }
    obj_bson_validate_read_number(validate, int32, &size);
    /* must have NULL at last */
    if (size <= 0) {
        return OBJ_CODE_BSON_INVALID_BSON;
    }
    if (!obj_bson_validate_skip(validate, size - 1)) {
        return OBJ_CODE_BSON_INVALID_BSON;
    }
    int8 c;
    if (!obj_bson_validate_can_read_number(validate, int8)) {
        return OBJ_CODE_BSON_INVALID_BSON;
    }
    obj_bson_validate_read_number(validate, int8, &c);
    if (c != 0) {
        return OBJ_CODE_BSON_NOT_NULL_TERMINATED_STRING;
    }
    return OBJ_CODE_OK;
}

/* skip size */
static obj_bool_t obj_bson_validate_skip(obj_bson_validate_t *validate, int size) {
    validate->position += size;
    return validate->position < validate->len;
}


/* init validate frame */
static obj_bson_validate_frame_t *obj_bson_validate_frame_init() {
    obj_bson_validate_frame_t *frame = obj_alloc(sizeof(obj_bson_validate_frame_t));
    if (frame == NULL) {
        return NULL;
    }
    frame->expected_size = 0;
    frame->start_position = 0;
    return frame;
}

/* validate a k-v pair */
static obj_global_error_code_t obj_bson_validate_kv(obj_bson_validate_t *validate, obj_bson_validate_state_t *next_state) {
    obj_global_error_code_t code = OBJ_CODE_OK;
    obj_bson_type_t type;
    int32 size;
    uint8 val;
    if (!obj_bson_validate_can_read_number(validate, uint8)) {
        return OBJ_CODE_BSON_INVALID_BSON;
    }
    obj_bson_validate_read_number(validate, uint8, &type);
    if (type == OBJ_BSON_TYPE_EOO) {
        *next_state = OBJ_BSON_VALIDATE_END_OBJ;
        return OBJ_CODE_OK;
    }
    code = obj_bson_validate_read_cstring(validate);
    if (code != OBJ_CODE_OK) {
        return code;
    }
    switch (type) {
    case OBJ_BSON_TYPE_DOUBLE:
        if (!obj_bson_validate_skip(validate, sizeof(double))) {
            return OBJ_CODE_BSON_INVALID_BSON;
        }
        return OBJ_CODE_OK;
    case OBJ_BSON_TYPE_UTF8:
        code = obj_bson_validate_read_string(validate);
        return code;
    case OBJ_BSON_TYPE_OBJECT:
    case OBJ_BSON_TYPE_ARRAY:
        *next_state = OBJ_BSON_VALIDATE_BEGIN_OBJ;
        return OBJ_CODE_OK;
    case OBJ_BSON_TYPE_BINARY:
        if (!obj_bson_validate_can_read_number(validate, int32)) {
            return OBJ_CODE_BSON_INVALID_BSON;
        }
        obj_bson_validate_read_number(validate, int32, &size);
        if (size < 0 || size > OBJ_BSON_MAX_SIZE) {
            return OBJ_CODE_BSON_SIZE_OVERFLOW;
        }
        if (!obj_bson_validate_skip(validate, 1 + size)) {
            return OBJ_CODE_BSON_INVALID_BSON;
        }
        return OBJ_CODE_OK;
    case OBJ_BSON_TYPE_BOOL:
        if (!obj_bson_validate_can_read_number(validate, uint8)) {
            return OBJ_CODE_BSON_INVALID_BSON;
        }
        obj_bson_validate_read_number(validate, uint8, &val);
        if (val != 0 && val != 1) {
            return OBJ_CODE_BSON_INVALID_BOOLEAN_VALUE;
        }
        return OBJ_CODE_OK;
    case OBJ_BSON_TYPE_NULL:
        return OBJ_CODE_OK;
    case OBJ_BSON_TYPE_INT32:
        if (!obj_bson_validate_skip(validate, sizeof(obj_int32_t))) {
            return OBJ_CODE_BSON_INVALID_BSON;
        }
        return OBJ_CODE_OK;
    case OBJ_BSON_TYPE_INT64:
        if (!obj_bson_validate_skip(validate, sizeof(obj_int64_t))) {
            return OBJ_CODE_BSON_INVALID_BSON;
        }
        return OBJ_CODE_OK;
    default:
        return OBJ_CODE_BSON_INVALID_BSON_TYPE;
    }
}

/* internal implemention of validate function */
static obj_global_error_code_t obj_bson_validate_internal(obj_bson_validate_t *validate) {
    obj_list_t *list = obj_list_create();
    obj_bson_validate_state_t state = OBJ_BSON_VALIDATE_BEGIN_OBJ;
    obj_bson_validate_frame_t *cur = NULL;
    obj_global_error_code_t code = OBJ_CODE_OK;;
    obj_bson_validate_state_t next_state;
    int actual_length;
    if (list == NULL) {
        return OBJ_CODE_BSON_VALIDATE_NOMEM;
    }
    while (state != OBJ_BSON_VALIDATE_DONE) {
        switch (state) {
        case OBJ_BSON_VALIDATE_BEGIN_OBJ:
            if (list->len > OBJ_BSON_VALIDATE_MAX_DEPTH) {
                return OBJ_CODE_BSON_VALIDATE_EXCEED_MAX_DEPTH;
            }
            /* init new frame */
            cur = obj_bson_validate_frame_init();
            if (cur == NULL) {
                code = OBJ_CODE_BSON_VALIDATE_NOMEM;
                goto clean;
            }
            obj_list_add_node_tail(list, cur);
            cur->start_position = validate->position;
            if (!obj_bson_validate_can_read_number(validate, int32)) {
                code = OBJ_CODE_BSON_INVALID_BSON;
                goto clean;
            }
            obj_bson_validate_read_number(validate, int32, &(cur->expected_size));
            state = OBJ_BSON_VALIDATE_WITHIN_OBJ;
        /* fall through */
        case OBJ_BSON_VALIDATE_WITHIN_OBJ:
            next_state = state;
            /* validate k-v pair */
            code = obj_bson_validate_kv(validate, &next_state);
            if (code != OBJ_CODE_OK) {
                goto clean;
            }
            state = next_state;
            break;
        case OBJ_BSON_VALIDATE_END_OBJ:
            actual_length = validate->position - cur->start_position;
            /* printf("actual length = %d, expected size = %d\n", actual_length, cur->expected_size); */
            if (actual_length != cur->expected_size) {
                code = OBJ_CODE_BSON_VALIDATE_WRONG_SIZE;
                goto clean;
            }
            obj_list_del_node_tail(list);
            if (obj_list_is_empty(list)) {
                state = OBJ_BSON_VALIDATE_DONE;
            } else {
                cur = (obj_bson_validate_frame_t *)(obj_list_get_tail(list)->value);
                /* printf("tail node expected size %d\n", cur->expected_size); */
                state = OBJ_BSON_VALIDATE_WITHIN_OBJ;
            }
            break;
        case OBJ_BSON_VALIDATE_DONE:
            /* should not reach here */
            break;
        }
    }
    /* clean */
    clean:
    if (list != NULL) {
        obj_list_destroy(list);
    }
    return code;
}

/* validate a bson */
obj_global_error_code_t obj_bson_validate(obj_uint8_t *data, int len) {
    /* invalid bson */
    if (len < 5) {
        return OBJ_CODE_BSON_INVALID_BSON;
    }
    obj_bson_validate_t validate;
    validate.data = data;
    validate.position = 0;
    validate.len = len;
    return obj_bson_validate_internal(&validate);
}