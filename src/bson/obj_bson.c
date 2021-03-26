#include "obj_core.h"

static void obj_bson_encode_length(obj_bson_t *bson);

static obj_bson_value_t *obj_bson_get_path_element(obj_bson_iter_t *iter, char *start, char *end);

static void obj_bson_grow(obj_bson_t *bson, obj_size_t size);

static void obj_bson_append_va(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, obj_uint8_t *first_data, va_list args);

static void obj_bson_append(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, obj_uint8_t *first_data, ...);

static void obj_bson_append_bson_begin(obj_bson_t *bson, char *key, int key_len, obj_bson_type_t child_type, obj_bson_t *child);

static void obj_bson_append_bson_end(obj_bson_t *bson, obj_bson_t *child);

static obj_uint8_t obj_g_zero = 0;

static char *obj_bson_type_name[] = {
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
    printf("offset: %d\n", bson->offset);
}

/* create a empty bson */
obj_bson_t *obj_bson_create() {
    obj_bson_t *bson;
    bson = (obj_bson_t *)obj_alloc(sizeof(obj_bson_t));
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

/* already validated */
obj_bson_t *obj_bson_create_with_data(obj_uint8_t *data, obj_int32_t len) {
    obj_bson_t *bson;
    bson = (obj_bson_t *)obj_alloc(sizeof(obj_bson_t));
    obj_int32_t len_le;
    obj_memcpy(&len_le, data, sizeof(len_le));
    if (obj_int32_from_le(len_le) != len) {
        return NULL;
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
void obj_bson_init_static_with_len(obj_bson_t *bson, obj_uint8_t *data, obj_int32_t len) {
    obj_int32_t len_le;
    obj_memcpy(&len_le, data, sizeof(len_le));
    bson->flag = OBJ_BSON_FLAG_STATIC | OBJ_BSON_FLAG_RDONLY;
    bson->len = bson->cap = len;
    bson->depth = 0;
    bson->data = (obj_uint8_t *)data;
    bson->offset = 0;
    bson->parent = NULL;
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

/* compare bson1 and bson2 using pattern */
/*
int obj_bson_compare(obj_bson_t *bson1, obj_bson_t *bson2, obj_bson_t *pattern) {
    
}
*/


/* get bson element according to path */
obj_bson_value_t *obj_bson_get_path(obj_bson_t *bson, char *path) {
    char *ret = NULL;
    char *start = path;
    char *end = NULL;
    /* at '\0 */
    char *path_end = obj_strlen(path) + start;
    obj_bson_value_t *value = NULL;
    obj_bson_t cur_bson;
    /* make copy */
    obj_memcpy(&cur_bson, bson, sizeof(obj_bson_t));
    while (true) {
        ret = obj_strchr(start, '.');
        obj_bson_iter_t iter;
        obj_bson_iter_init(&iter, &cur_bson);
        if (ret == NULL) {
            /* last */
            end = path_end - 1;

        } else {
            end = ret - 1;
        }
        value = obj_bson_get_path_element(&iter, start, end);
        /* array out of range */
        if (value == NULL) {
            return NULL;
        }
        if (ret == NULL) {
            /* end */
            break;
        }
        /* step into */
        if (value->type == OBJ_BSON_TYPE_OBJECT) {
            obj_bson_init_static_with_len(&cur_bson, value->value.v_object.data, value->value.v_object.len);
        } else if (value->type == OBJ_BSON_TYPE_ARRAY) {
            obj_bson_init_static_with_len(&cur_bson, value->value.v_array.data, value->value.v_array.len);
        }
        start = ret + 1;
    }
    return value;
}

/* 
 * get corresponding element according to path. format checked
 * e.x.1: a[0]
 * e.x.2: aa
 * e.x.3: aa[0][1][2]
 */
static obj_bson_value_t *obj_bson_get_path_element(obj_bson_iter_t *iter, char *start, char *end) {
    char *key = NULL;
    int key_len;
    obj_bson_type_t bson_type;
    obj_bson_value_t *value = NULL;
    /* parse current path */
    char *bracket = NULL;
    bracket = obj_strchr(start, '[');
    obj_bool_t not_array = true;
    int path_len = end - start + 1;
    if (bracket == NULL) {
        /* normal path */        
        
    } else {
        /* array path */
        not_array = false;
    }
    if (not_array) {
        while (obj_bson_iter_next_internal(iter, &key, &bson_type)) {
            key_len = obj_strlen(key);
            if (key_len != path_len) {
                continue;
            }
            if (obj_memcmp(key, start, key_len) == 0) {
                /* find */
                value = obj_bson_iter_value(iter);
                return value;
            }
        }
    } else {
        /* array */
        char *arr_name = start;
        int arr_name_len = bracket - start;
        /*
        obj_bson_t *array = NULL;
        */
        obj_bson_t array;
        while (obj_bson_iter_next_internal(iter, &key, &bson_type)) {
            key_len = obj_strlen(key);
            if (key_len != arr_name_len) {
                continue;
            }
            if (obj_memcmp(key, start, key_len) == 0) {
                obj_assert(bson_type == OBJ_BSON_TYPE_ARRAY);
                /* find */
                value = obj_bson_iter_value(iter);
                obj_bson_init_static_with_len(&array, value->value.v_array.data, value->value.v_array.len);
                break;
            }
        }
        /* parse */
        
        obj_bson_t *cur_bson = &array;
        char *cur_pos = bracket + 1;
        int cur_index;
        char c;
        char *key = NULL;
        int cnt = 0;
        obj_bson_type_t bson_type;
        while (true) {
            cur_index = 0;
            while ((c = cur_pos[0]) != ']') {
                cur_index = cur_index * 10 + (cur_pos[0] - '0');
                cur_pos += 1;
            }
            obj_bson_iter_init(iter, cur_bson);
            /* get according to index */
            while (obj_bson_iter_next_internal(iter, &key, &bson_type)) {
                if (cnt == cur_index) {
                    value = obj_bson_iter_value(iter);
                    break;
                } else {
                    cnt++;
                }
            }
            if (cnt < cur_index) {
                return NULL;
            }
            if (cur_pos == end) {
                /* reach end */
                return value;
            }
            obj_assert(value->type == OBJ_BSON_TYPE_ARRAY);
            obj_bson_init_static_with_len(cur_bson, value->value.v_array.data, value->value.v_array.len);
            /* skip ']' */
            cur_pos++;
        }
        
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
char *obj_bson_type_to_name(obj_bson_type_t type) {
    obj_assert(type >= 0 && type < OBJ_NELEM(obj_bson_type_name));
    return obj_bson_type_name[type];
}

obj_uint8_t *obj_bson_data(obj_bson_t *bson) {
    return bson->data + bson->offset;
}


/* check available space */
static void obj_bson_grow(obj_bson_t *bson, obj_size_t size) {
    obj_size_t req;
    /* consider child as well */
    req = bson->offset + bson->len + size + bson->depth;
    if (req <= bson->cap) {
        return;
    }
    req = obj_next_power_of_two(req);
    if (req <= OBJ_INT32_MAX) {
        bson->data = obj_realloc(bson->data, req);
        bson->cap = req;
    }
}

static void obj_bson_append_va(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, obj_uint8_t *first_data, va_list args) {
    obj_uint8_t *data;
    int data_len;
    obj_uint8_t *ptr;
    obj_bson_grow(bson, (obj_size_t)n_bytes);
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
            data = va_arg(args, obj_uint8_t *);
        }
    } while(n_pairs);
    obj_bson_encode_length(bson);
    *ptr = '\0';
}

static void obj_bson_append(obj_bson_t *bson, int n_pairs, int n_bytes, int first_len, obj_uint8_t *first_data, ...) {
    obj_assert(n_pairs);
    obj_assert(first_len);
    obj_assert(first_data);
    va_list args;
    /* check if too big */
    if (n_bytes > (OBJ_BSON_MAX_SIZE - bson->len)) {
        return;
    }
    va_start(args, first_data);
    obj_bson_append_va(bson, n_pairs, n_bytes, first_len, first_data, args);
    va_end(args);
}

/* append double */
void obj_bson_append_double(obj_bson_t *bson, char *key, int key_len, double value) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_DOUBLE;
    double value_le;
    value_le = obj_double_to_le(value);
    obj_bson_append(bson, 4, (1 + key_len + 1 + 8), 
    1, &type, 
    key_len, key,
    1, &obj_g_zero, 
    8, &value_le);
}

/* append utf-8 string */
void obj_bson_append_utf8(obj_bson_t *bson, char *key, int key_len, char *value, int value_len) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_UTF8;
    if (value == NULL) {
        obj_bson_append_null(bson, key, key_len);
    }
    obj_int32_t value_len_le;
    value_len_le = obj_int32_to_le((obj_int32_t)(value_len + 1));
    obj_bson_append(bson, 6, (1 + key_len + 1 + 4 + value_len + 1),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    4, &value_len_le,
    value_len, value,
    1, &obj_g_zero);
}

static void obj_bson_append_bson_begin(obj_bson_t *bson, char *key, int key_len, obj_bson_type_t child_type, obj_bson_t *child) {
    obj_uint8_t type = child_type;
    /* empty by default */
    obj_uint8_t empty[5] = {5};
    obj_assert(bson && key && child);
    obj_assert(!(bson->flag & OBJ_BSON_FLAG_IN_CHILD));
    obj_assert(!(bson->flag & OBJ_BSON_FLAG_RDONLY));
    obj_assert((child_type == OBJ_BSON_TYPE_OBJECT) || (child_type == OBJ_BSON_TYPE_ARRAY));
    obj_bson_append(bson, 4, (1 + key_len + 1 + 5),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    5, empty);
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
}

static void obj_bson_append_bson_end(obj_bson_t *bson, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(bson->flag & OBJ_BSON_FLAG_IN_CHILD);
    obj_assert(!(child->flag & OBJ_BSON_FLAG_IN_CHILD));
    /* in flag */
    bson->flag &= ~OBJ_BSON_FLAG_IN_CHILD;
    /* not including the default 5 byte added */
    bson->len = (bson->len + child->len - 5);
    obj_bson_data(bson)[bson->len - 1] = '\0';
    obj_bson_encode_length(bson);
}

void obj_bson_append_array_begin(obj_bson_t *bson, char *key, int key_len, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(child);
    obj_bson_append_bson_begin(bson, key, key_len, OBJ_BSON_TYPE_ARRAY, child);
}

void obj_bson_append_array_end(obj_bson_t *bson, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(child);
    obj_bson_append_bson_end(bson, child);
}

void obj_bson_append_object_begin(obj_bson_t *bson, char *key, int key_len, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(child);
    obj_bson_append_bson_begin(bson, key, key_len, OBJ_BSON_TYPE_OBJECT, child);
}

void obj_bson_append_object_end(obj_bson_t *bson, obj_bson_t *child) {
    obj_assert(bson);
    obj_assert(child);
    obj_bson_append_bson_end(bson, child);
}

/* append object */
void obj_bson_append_object(obj_bson_t *bson, char *key, int key_len, obj_bson_t *value) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_OBJECT;
    obj_bson_append(bson, 4, (1 + key_len + 1 + value->len),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    value->len, obj_bson_data(value));
}

/* append array */
void obj_bson_append_array(obj_bson_t *bson, char *key, int key_len, obj_bson_t *value) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_ARRAY;
    obj_bson_append(bson, 4, (1 + key_len + 1 + value->len),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    value->len, obj_bson_data(value));
}

/* append binary */
void obj_bson_append_binary(obj_bson_t *bson, char *key, int key_len, obj_uint8_t *value, int value_len) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_BINARY;
    obj_int32_t value_len_le;
    value_len_le = obj_int32_to_le((obj_int32_t)value_len);
    obj_bson_append(bson, 5, (1 + key_len + 1 + 4 + value_len),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    4, &value_len_le,
    value_len, value);
}

/* append bool */
void obj_bson_append_bool(obj_bson_t *bson, char *key, int key_len, obj_bool_t value) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_BOOL;
    obj_bson_append(bson, 4, (1 + key_len + 1 + 1), 
    1, &type, 
    key_len, key,
    1, &obj_g_zero,
    1, &value);
}

/* append null */
void obj_bson_append_null(obj_bson_t *bson, char *key, int key_len) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_NULL;
    obj_bson_append(bson, 3, (1 + key_len + 1),
    1, &type,
    key_len, key,
    1, &obj_g_zero);
}

/* append int32 */
void obj_bson_append_int32(obj_bson_t *bson, char *key, int key_len, obj_int32_t value) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_INT32;
    obj_int32_t value_le;
    value_le = obj_int32_to_le(value);
    obj_bson_append(bson, 4, (1 + key_len + 1 + 4),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    4, &value_le);
}

/* append int64 */
void obj_bson_append_int64(obj_bson_t *bson, char *key, int key_len, obj_int64_t value) {
    obj_assert(bson);
    obj_assert(key);
    static obj_uint8_t type = (obj_uint8_t)OBJ_BSON_TYPE_INT64;
    obj_int64_t value_le;
    value_le = obj_int64_to_le(value);
    obj_bson_append(bson, 4, (1 + key_len + 1 + 8),
    1, &type,
    key_len, key,
    1, &obj_g_zero,
    8, &value_le);
}

/* general append function */
void obj_bson_append_value(obj_bson_t *bson, char *key, int key_len, obj_bson_value_t *value) {
    obj_assert(bson);
    obj_assert(key);
    obj_assert(value);
    obj_bson_t bson_temp;
    switch (value->type) {
    case OBJ_BSON_TYPE_DOUBLE:
        obj_bson_append_double(bson, key, key_len, value->value.v_double);
        break;
    case OBJ_BSON_TYPE_UTF8:
        obj_bson_append_utf8(bson, key, key_len, value->value.v_utf8.str, value->value.v_utf8.len);
        break;
    case OBJ_BSON_TYPE_OBJECT:
        obj_bson_init_static_with_len(&bson_temp, value->value.v_object.data, value->value.v_object.len);
        obj_bson_append_object(bson, key, key_len, &bson_temp);
        obj_bson_destroy(&bson_temp);
        break;
    case OBJ_BSON_TYPE_ARRAY:
        obj_bson_init_static_with_len(&bson_temp, value->value.v_array.data, value->value.v_array.len);
        obj_bson_append_object(bson, key, key_len, &bson_temp);
        obj_bson_destroy(&bson_temp);
        break;
    case OBJ_BSON_TYPE_BINARY:
        obj_bson_append_binary(bson, key, key_len, value->value.v_binary.data, value->value.v_binary.len);
        break;
    case OBJ_BSON_TYPE_BOOL:
        obj_bson_append_bool(bson, key, key_len, value->value.v_bool);
        break;
    case OBJ_BSON_TYPE_NULL:
        obj_bson_append_null(bson, key, key_len);
        break;
    case OBJ_BSON_TYPE_INT32:
        obj_bson_append_int32(bson, key, key_len, value->value.v_int32);
        break;
    case OBJ_BSON_TYPE_INT64:
        obj_bson_append_int64(bson, key, key_len, value->value.v_int64);
        break;
    default:
        break;
    }
}


