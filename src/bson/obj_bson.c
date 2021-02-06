#include "obj_core.h"

static void obj_bson_encode_length(obj_bson_t *bson);

static obj_bool_t obj_bson_grow(obj_bson_t *bson, obj_size_t size);

static obj_bool_t obj_bson_append_va(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, const obj_uint8_t *first_data, va_list args);

static obj_bool_t obj_bson_append(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, const obj_uint8_t *first_data, ...);

static obj_bool_t obj_bson_append_bson_begin(obj_bson_t *bson, const char *key, int key_len, obj_bson_type_t child_type, obj_bson_t *child);

static obj_bool_t obj_bson_append_bson_end(obj_bson_t *bson, obj_bson_t *child);

static const obj_uint8_t obj_g_zero = 0;

static const char *obj_bson_type_name[] = {
    "missing",
    "double",
    "string",
    "object",
    "array",
    "binary",
    "bool",
    "null",
    "int32",
    "int64"
};

/* print hex form of bson */
void obj_bson_print(obj_bson_t *bson) {
    int i;
    for (i = 0; i < bson->len; i++) {
        printf("%02x ", bson->data[i]);
    }
    printf("\n");
    printf("cap: %d\n", bson->cap);
    printf("depth: %d\n", bson->depth);
    printf("len: %d\n", bson->len);
}

/* init a empty bson */
obj_bson_t *obj_bson_init() {
    obj_bson_t *bson;
    bson = (obj_bson_t *)obj_alloc(sizeof(obj_bson_t));
    if (bson == NULL) {
        return NULL;
    }
    bson->data = (obj_uint8_t *)obj_alloc(OBJ_BSON_INIT_SIZE);
    bson->flag = 0;
    bson->len = 5;
    bson->depth = 0;
    bson->cap = OBJ_BSON_INIT_SIZE;
    bson->offset = 0;
    bson->parent = NULL;
    bson->data[0] = 5;
    bson->data[1] = 0;
    bson->data[2] = 0;
    bson->data[3] = 0;
    bson->data[4] = 0;
    return bson;
}

/* init bson with given data */
obj_bson_t *obj_bson_init_with_data(const obj_uint8_t *data, obj_int32_t len) {
    obj_bson_t *bson;
    bson = (obj_bson_t *)obj_alloc(sizeof(obj_bson_t));
    if (bson == NULL) {
        return NULL;
    }
    obj_int32_t len_le;
    if (len < 5 || len > OBJ_BSON_MAX_SIZE) {
        return false;
    }
    obj_memcpy(&len_le, data, sizeof(len_le));
    if (obj_int32_from_le(len_le) != len) {
        return false;
    }
    /* must end with '\0' */
    if (data[len - 1]) {
        return false;
    }
    bson->flag = OBJ_BSON_FLAG_RDONLY;
    bson->len = bson->cap = len;
    bson->data = (obj_uint8_t *)data;
    bson->depth = 0;
    bson->offset = 0;
    bson->parent = NULL;
    return bson;
}

/* init bson from a static context, with given length */
obj_bool_t obj_bson_init_static_with_len(obj_bson_t *bson, const obj_uint8_t *data, obj_int32_t len) {
    obj_int32_t len_le;
    if (len < 5 || len > OBJ_BSON_MAX_SIZE) {
        return false;
    }
    obj_memcpy(&len_le, data, sizeof(len_le));
    if (obj_int32_from_le(len_le) != len) {
        return false;
    }
    /* must end with '\0' */
    if (data[len - 1]) {
        return false;
    }
    bson->flag = OBJ_BSON_FLAG_STATIC | OBJ_BSON_FLAG_RDONLY;
    bson->len = bson->cap = len;
    bson->depth = 0;
    bson->data = (obj_uint8_t *)data;
    bson->offset = 0;
    bson->parent = NULL;
    return true;
}

obj_bool_t obj_bson_is_empty(obj_bson_t *bson) {
    return bson->len <= 5;
}

/* destroy a bson */
void obj_bson_destroy(obj_bson_t *bson) {
    obj_assert(bson);
    if (!(bson->flag & (OBJ_BSON_FLAG_RDONLY | OBJ_BSON_FLAG_NOFREE))) {
        obj_free(bson->data);
    }
    if (!(bson->flag & OBJ_BSON_FLAG_STATIC)) {
        obj_free(bson);
    }
}

static void obj_bson_encode_length(obj_bson_t *bson) {
    obj_int32_t len = (obj_int32_t)bson->len;
#ifdef ENDIAN_LITTLE
    obj_memcpy(obj_bson_data(bson), &len, sizeof(obj_int32_t));
#else
    obj_int32_t len_le = obj_int32_to_le(len);
    obj_memcpy(obj_bson_data(bson), &len_le, sizeof(obj_int32_t))
#endif
}

/* show corresponding bson type name */
const char *obj_bson_type_to_name(obj_bson_type_t type) {
    obj_assert(type >= 0 && type < OBJ_NELEM(obj_bson_type_name));
    return obj_bson_type_name[type];
}

obj_uint8_t *obj_bson_data(const obj_bson_t *bson) {
    return bson->data + bson->offset;
}


/* check available space */
static obj_bool_t obj_bson_grow(obj_bson_t *bson, obj_size_t size) {
    obj_size_t req;
    /* consider child as well */
    req = bson->offset + bson->len + size + bson->depth;
    if (req <= bson->cap) {
        return true;
    }
    req = obj_next_power_of_two(req);
    if (req <= OBJ_INT32_MAX) {
        bson->data = obj_realloc(bson->data, req);
        if (bson->data == NULL) {
            return false;
        }
        bson->cap = req;
        return true;
    }
    return false;
}

static obj_bool_t obj_bson_append_va(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, const obj_uint8_t *first_data, va_list args) {
    const obj_uint8_t *data;
    int data_len;
    obj_uint8_t *ptr;
    /* first, check available space */
    if (!obj_bson_grow(bson, (obj_size_t)n_bytes)) {
        return false;
    }
    data = first_data;
    data_len = first_len;
    ptr = obj_bson_data(bson) + bson->len - 1;
    do {
        n_pairs--;
        obj_memcpy(ptr, data, data_len);
        bson->len += data_len;
        ptr += data_len;
        if (n_pairs) {
            data_len = va_arg(args, int);
            data = va_arg(args, const obj_uint8_t *);
        }
    } while(n_pairs);
    obj_bson_encode_length(bson);
    *ptr = '\0';
    return true;
}

static obj_bool_t obj_bson_append(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, const obj_uint8_t *first_data, ...) {
    obj_assert(n_pairs);
    obj_assert(first_len);
    obj_assert(first_data);
    va_list args;
    obj_bool_t ret;
    /* check if too big */
    if (n_bytes > (OBJ_BSON_MAX_SIZE - bson->len)) {
        return false;
    }
    va_start(args, first_data);
    ret = obj_bson_append_va(bson, n_pairs, n_bytes, first_len, first_data, args);
    va_end(args);
    return ret;
}

/* append double */
obj_bool_t obj_bson_append_double(obj_bson_t *bson, const char *key, int key_len, double value) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_DOUBLE;
    double value_le;
    value_le = obj_double_to_le(value);
    return obj_bson_append(bson, 4, (1 + key_len + 1 + 8), 
    1, &type, 
    key_len, key,
    1, &obj_g_zero, 
    8, &value_le);
}

/* append utf-8 string */
obj_bool_t obj_bson_append_utf8(obj_bson_t *bson, const char *key, int key_len, const char *value, int value_len) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_UTF8;
    if (value == NULL) {
        obj_bson_append_null(bson, key, key_len);
    }
    obj_int32_t value_len_le;
    value_len_le = obj_int32_to_le((obj_int32_t)(value_len + 1));
    return obj_bson_append(bson, 6, (1 + key_len + 1 + 4 + value_len + 1),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    4, &value_len_le,
    value_len, value,
    1, &obj_g_zero);
}

static obj_bool_t obj_bson_append_bson_begin(obj_bson_t *bson, const char *key, int key_len, obj_bson_type_t child_type, obj_bson_t *child) {
    const obj_uint8_t type = child_type;
    /* empty by default */
    const obj_uint8_t empty[5] = {5};
    obj_assert(bson && key && child);
    obj_assert(!(bson->flag & OBJ_BSON_FLAG_IN_CHILD));
    obj_assert(!(bson->flag & OBJ_BSON_FLAG_RDONLY));
    obj_assert((child_type == OBJ_BSON_TYPE_OBJECT) || (child_type == OBJ_BSON_TYPE_ARRAY));
    if (!obj_bson_append(bson, 4, (1 + key_len + 1 + 5),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    5, empty)) {
        return false;
    }
    bson->flag |= OBJ_BSON_FLAG_IN_CHILD;
    child->flag = (OBJ_BSON_FLAG_CHILD | OBJ_BSON_FLAG_NOFREE | OBJ_BSON_FLAG_STATIC);
    if (bson->flag & OBJ_BSON_FLAG_CHILD) {
        child->depth = bson->depth + 1;
    } else {
        child->depth = 1;
    }
    /* init child */
    child->parent = bson;
    /* 1: ending '\0';5: empty child */
    child->offset = bson->offset + bson->len - 1 - 5;
    child->len = 5;
    child->cap = bson->cap;
    child->data = bson->data;

    return true;
}

static obj_bool_t obj_bson_append_bson_end(obj_bson_t *bson, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(bson->flag & OBJ_BSON_FLAG_IN_CHILD);
    obj_assert(!(child->flag & OBJ_BSON_FLAG_IN_CHILD));
    /* in flag */
    bson->flag &= ~OBJ_BSON_FLAG_IN_CHILD;
    /* not including the default 5 byte added */
    bson->len = (bson->len + child->len - 5);
    obj_bson_data(bson)[bson->len - 1] = '\0';
    obj_bson_encode_length(bson);
    return true;
}

obj_bool_t obj_bson_append_array_begin(obj_bson_t *bson, const char *key, int key_len, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(child);
    return obj_bson_append_bson_begin(bson, key, key_len, OBJ_BSON_TYPE_ARRAY, child);
}

obj_bool_t obj_bson_append_array_end(obj_bson_t *bson, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(child);
    return obj_bson_append_bson_end(bson, child);
}

obj_bool_t obj_bson_append_object_begin(obj_bson_t *bson, const char *key, int key_len, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(child);
    return obj_bson_append_bson_begin(bson, key, key_len, OBJ_BSON_TYPE_OBJECT, child);
}

obj_bool_t obj_bson_append_object_end(obj_bson_t *bson, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(child);
    return obj_bson_append_bson_end(bson, child);
}

/* append object */
obj_bool_t obj_bson_append_object(obj_bson_t *bson, const char *key, int key_len, const obj_bson_t *value) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_OBJECT;
    return obj_bson_append(bson, 4, (1 + key_len + 1 + value->len),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    value->len, obj_bson_data(value));
}

/* append array */
obj_bool_t obj_bson_append_array(obj_bson_t *bson, const char *key, int key_len, const obj_bson_t *value) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_ARRAY;
    return obj_bson_append(bson, 4, (1 + key_len + 1 + value->len),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    value->len, obj_bson_data(value));
}

/* append binary */
obj_bool_t obj_bson_append_binary(obj_bson_t *bson, const char *key, int key_len, const obj_uint8_t *value, int value_len) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_BINARY;
    obj_int32_t value_len_le;
    value_len_le = obj_int32_to_le((obj_int32_t)value_len);
    return obj_bson_append(bson, 5, (1 + key_len + 1 + 4 + value_len),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    4, &value_len_le,
    value_len, value);
}

/* append bool */
obj_bool_t obj_bson_append_bool(obj_bson_t *bson, const char *key, int key_len, obj_bool_t value) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_BOOL;
    return obj_bson_append(bson, 4, (1 + key_len + 1 + 1), 
    1, &type, 
    key_len, key,
    1, &obj_g_zero,
    1, &value);
}

/* append null */
obj_bool_t obj_bson_append_null(obj_bson_t *bson, const char *key, int key_len) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_NULL;
    return obj_bson_append(bson, 3, (1 + key_len + 1),
    1, &type,
    key_len, key,
    1, &obj_g_zero);
}

/* append int32 */
obj_bool_t obj_bson_append_int32(obj_bson_t *bson, const char *key, int key_len, obj_int32_t value) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_INT32;
    obj_int32_t value_le;
    value_le = obj_int32_to_le(value);
    return obj_bson_append(bson, 4, (1 + key_len + 1 + 4),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    4, &value_le);
}

/* append int64 */
obj_bool_t obj_bson_append_int64(obj_bson_t *bson, const char *key, int key_len, obj_int64_t value) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_INT64;
    obj_int64_t value_le;
    value_le = obj_int64_to_le(value);
    return obj_bson_append(bson, 4, (1 + key_len + 1 + 8),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    8, &value_le);
}

/* general append function */
obj_bool_t obj_bson_append_value(obj_bson_t *bson, const char *key, int key_len, const obj_bson_value_t *value) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    obj_bool_t ret = false;
    obj_bson_t bson_temp;
    switch (value->type) {
    case OBJ_BSON_TYPE_DOUBLE:
        ret = obj_bson_append_double(bson, key, key_len, value->value.v_double);
        break;
    case OBJ_BSON_TYPE_UTF8:
        ret = obj_bson_append_utf8(bson, key, key_len, value->value.v_utf8.str, value->value.v_utf8.len);
        break;
    case OBJ_BSON_TYPE_OBJECT:
        if (obj_bson_init_static_with_len(&bson_temp, value->value.v_object.data, value->value.v_object.len)) {
            ret = obj_bson_append_object(bson, key, key_len, &bson_temp);
            obj_bson_destroy(&bson_temp);
        }
        break;
    case OBJ_BSON_TYPE_ARRAY:
        if (obj_bson_init_static_with_len(&bson_temp, value->value.v_array.data, value->value.v_array.len)) {
            ret = obj_bson_append_object(bson, key, key_len, &bson_temp);
            obj_bson_destroy(&bson_temp);
        }
        break;
    case OBJ_BSON_TYPE_BINARY:
        ret = obj_bson_append_binary(bson, key, key_len, value->value.v_binary.data, value->value.v_binary.len);
        break;
    case OBJ_BSON_TYPE_BOOL:
        ret = obj_bson_append_bool(bson, key, key_len, value->value.v_bool);
        break;
    case OBJ_BSON_TYPE_NULL:
        ret = obj_bson_append_null(bson, key, key_len);
        break;
    case OBJ_BSON_TYPE_INT32:
        ret = obj_bson_append_int32(bson, key, key_len, value->value.v_int32);
        break;
    case OBJ_BSON_TYPE_INT64:
        ret = obj_bson_append_int64(bson, key, key_len, value->value.v_int64);
        break;
    default:
        break;
    }
    return ret;
}

/* type of kv pair */
obj_bson_type_t obj_bson_kv_type(obj_bson_kv_t *obj_bson_kv) {
    return (obj_bson_type_t)(*((obj_uint8_t *)(obj_bson_kv->data)));
}

/* field name of kv pair */
/*
char *obj_bson_kv_field_name(obj_bson_kv_t *obj_bson_kv) {
    return obj_bson_kv->data + 1;
}
*/