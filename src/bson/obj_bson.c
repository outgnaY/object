#include "bson/obj_bson.h"
#include "util/obj_math.h"
#include "util/obj_string.h"
#include "util/obj_endian.h"
#include "mem/obj_mem.h"
#include "mem/obj_mem_simple.h"

static void obj_bson_encode_length(obj_bson_t *bson);


static obj_bool_t obj_bson_stack_grow(obj_bson_stack_t *bson_stack, obj_size_t size);

static obj_bool_t obj_bson_heap_grow(obj_bson_heap_t *bson_heap, obj_size_t size);

static obj_bool_t obj_bson_grow(obj_bson_t *bson, obj_size_t size);

static obj_bool_t obj_bson_append_va(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, const obj_uint8_t *first_data, va_list args);

static obj_bool_t obj_bson_append(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, const obj_uint8_t *first_data, ...);





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
    if (bson->flag & OBJ_BSON_FLAG_STACK) {
        obj_bson_stack_t *bson_stack = (obj_bson_stack_t *)bson;
        for (i = 0; i < bson->len; i++) {
            printf("%#x ", bson_stack->data[i]);
        }
    } else {
        obj_bson_heap_t *bson_heap = (obj_bson_heap_t *)bson;
        for (i = 0; i < bson->len; i++) {
            printf("%#x ", bson_heap->data[i]);
        }
    }
    printf("\n");
}

obj_bson_t *obj_bson_init_heap() {
    obj_bson_heap_t *bson_heap;
    /* allocate continuous space */
    bson_heap = (obj_bson_heap_t *)obj_alloc(sizeof(obj_bson_heap_t) + OBJ_BSON_HEAP_DATA_INIT);
    if (bson_heap == NULL) {
        return NULL;
    }
    bson_heap->base.flag = OBJ_BSON_FLAG_HEAP;
    bson_heap->base.len = 5;
    bson_heap->data = (obj_uint8_t *)(bson_heap + 1);
    if (bson_heap->data == NULL) {
        obj_free(bson_heap->data);
        return NULL;
    }
    bson_heap->data[0] = 5;
    bson_heap->data[1] = 0;
    bson_heap->data[2] = 0;
    bson_heap->data[3] = 0;
    bson_heap->data[4] = 0;
    return (obj_bson_t *)bson_heap;
}


obj_bson_t *obj_bson_init_data(obj_uint8_t *data, int len) {
    obj_bson_heap_t *bson_heap;
    bson_heap = (obj_bson_heap_t *)obj_alloc(sizeof(obj_bson_heap_t));
    if (bson_heap == NULL) {
        return NULL;
    }
    bson_heap->base.flag = OBJ_BSON_FLAG_RDONLY | OBJ_BSON_FLAG_HEAP | OBJ_BSON_FLAG_DATA_EXTERNAL;
    bson_heap->base.len = len;
    bson_heap->data = data;
    return (obj_bson_t *)bson_heap;
}

void obj_bson_destroy(obj_bson_t *bson) {
    obj_assert(bson);
    if (bson->flag & OBJ_BSON_FLAG_HEAP) {
        obj_free(bson);
    }
}

static void obj_bson_encode_length(obj_bson_t *bson) {
    obj_int32_t len = (obj_int32_t)bson->len;
#if OBJ_BYTE_ORDER == OBJ_LITTLE_ENDIAN
    obj_memcpy(obj_bson_data(bson), &len, sizeof(obj_int32_t));
#else
    obj_int32_t len_le = obj_int32_to_le(len);
    obj_memcpy(obj_bson_data(bson), &len_le, sizeof(obj_int32_t))
#endif
}

/* show corresponding bson type name */
const char *obj_bson_type_to_name(obj_bson_type_t type) {
    obj_assert(type >= 0 && type < NELEM(obj_bson_type_name));
    return obj_bson_type_name[type];
}

obj_uint8_t *obj_bson_data(const obj_bson_t *bson) {
    if (bson->flag & OBJ_BSON_FLAG_STACK) {
        obj_bson_stack_t *bson_stack = (obj_bson_stack_t *)bson;
        return bson_stack->data;
    } else {
        obj_bson_heap_t *bson_heap = (obj_bson_heap_t *)bson;
        return bson_heap->data;
    }
}

/* grow current bson to a bigger size, transfer to heap if necessary */
static obj_bool_t obj_bson_stack_grow(obj_bson_stack_t *bson_stack, obj_size_t size) {
    obj_size_t new_size;
    if (bson_stack->base.len + size <= sizeof(bson_stack->data)) {
        return true;
    }
    obj_bson_heap_t *bson_heap;
    new_size = obj_next_power_of_two(bson_stack->base.len + sizeof(obj_bson_heap_t));
    if (new_size <= OBJ_INT32_MAX) {
        bson_heap = obj_alloc(new_size);
        if (bson_heap == NULL) {
            return false;
        }
        bson_heap->data = (obj_uint8_t *)(bson_heap + 1);
        obj_memcpy(&bson_heap->base, &bson_stack->base, sizeof(obj_bson_t));
        obj_memcpy(bson_heap->data, bson_stack->data, bson_stack->base.len);
        return true;
    }
    return false;
}

/* grow current bson to a bigger size */
static obj_bool_t obj_bson_heap_grow(obj_bson_heap_t *bson_heap, obj_size_t size) {
    obj_size_t new_size;
    new_size = bson_heap->base.len;
    new_size = obj_next_power_of_two(new_size);
    if (new_size <= OBJ_INT32_MAX) {
        bson_heap->data = obj_realloc(bson_heap->data, new_size);
        if (bson_heap->data == NULL) {
            return false;
        }
        return true;
    }
    return false;
}

/* check available space */
static obj_bool_t obj_bson_grow(obj_bson_t *bson, obj_size_t size) {
    if (bson->flag & OBJ_BSON_FLAG_STACK) {
        return obj_bson_stack_grow((obj_bson_stack_t *)bson, size);
    } else {
        return obj_bson_heap_grow((obj_bson_heap_t *)bson, size);
    }
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
    obj_bson_t *bson_heap_temp;
    switch (value->type) {
    case OBJ_BSON_TYPE_DOUBLE:
        ret = obj_bson_append_double(bson, key, key_len, value->value.v_double);
        break;
    case OBJ_BSON_TYPE_UTF8:
        ret = obj_bson_append_utf8(bson, key, key_len, value->value.v_utf8.str, value->value.v_utf8.len);
        break;
    case OBJ_BSON_TYPE_OBJECT:
        bson_heap_temp = obj_bson_init_data(value->value.v_object.data, value->value.v_object.len);
        if (bson_heap_temp != NULL) {
            ret = obj_bson_append_object(bson, key, key_len, bson_heap_temp);
            obj_free(bson_heap_temp);
        }
        break;
    case OBJ_BSON_TYPE_ARRAY:
        bson_heap_temp = obj_bson_init_data(value->value.v_array.data, value->value.v_array.len);
        if (bson_heap_temp != NULL) {
            ret = obj_bson_append_array(bson, key, key_len, bson_heap_temp);
            obj_free(bson_heap_temp);
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