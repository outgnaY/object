#include "obj_core.h"

static obj_bool_t obj_index_key_init(obj_index_key_t *index_key, obj_bson_t *bson);
static int obj_index_key_element_compare(obj_uint8_t **p1, obj_uint8_t **p2);


/* 
 * for key pattern {a: 1, b: -1} 
 * get(0) == 1; get(1) == -1
 */
int obj_index_key_order_get(obj_index_key_order_t key_order, int i) {
    return ((1 << i) & key_order) ? -1 : 1;
}

unsigned obj_index_key_order_descending(obj_index_key_order_t key_order, unsigned mask) {
    return key_order & mask;
}

/* create index key */
obj_index_key_t *obj_index_key_create(obj_bson_t *bson) {
    obj_index_key_t *index_key = obj_alloc(sizeof(obj_index_key_t));
    if (index_key == NULL) {
        return NULL;
    }
    index_key->data = NULL;
    if (!obj_index_key_init(index_key, bson)) {
        obj_free(index_key);
        return NULL;
    }
    return index_key;
}

/* init index key */
static obj_bool_t obj_index_key_init(obj_index_key_t *index_key, obj_bson_t *bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, bson);
    obj_bson_value_t *value = NULL;
    obj_bson_type_t bson_type;
    const char *key;
    obj_uint8_t bits = 0;
    int expect_size = 0;
    int cnt = 0;
    obj_uint8_t type;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        cnt++;
        value = (obj_bson_value_t *)obj_bson_iter_value(&iter);
        switch (bson_type) {
            case OBJ_BSON_TYPE_DOUBLE: {
                expect_size += 1 + sizeof(double);
                break;
            }
            case OBJ_BSON_TYPE_UTF8: {
                expect_size += 1 + sizeof(obj_int32_t) + value->value.v_utf8.len;
                break;
            }
            case OBJ_BSON_TYPE_BINARY: {
                expect_size += 1 + sizeof(obj_int32_t) + value->value.v_binary.len;
                break;
            }
            case OBJ_BSON_TYPE_INT32: {
                expect_size += 1 + sizeof(obj_int32_t);
                break;
            }
            case OBJ_BSON_TYPE_INT64: {
                expect_size += 1 + sizeof(obj_int64_t);
                break;
            }
            default:
                obj_assert(0);
                break;
        }
    }
    obj_uint8_t *data = obj_alloc(expect_size);
    if (data == NULL) {
        return false;
    }
    index_key->data = data;
    obj_uint8_t *cur = data;
    obj_bson_iter_init(&iter, bson);
    int now_cnt = 0;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        now_cnt++;
        if (now_cnt < cnt) {
            bits |= OBJ_INDEX_KEY_TYPE_HASMORE;
        }
        value = (obj_bson_value_t *)obj_bson_iter_value(&iter);
        switch (bson_type) {
            case OBJ_BSON_TYPE_DOUBLE: {
                type = OBJ_INDEX_KEY_TYPE_DOUBLE | bits;
                obj_memcpy(cur, &type, 1);
                cur += 1;
                double d = value->value.v_double;
                d = obj_double_to_le(d);
                obj_memcpy(cur, &d, sizeof(double));
                cur += sizeof(double);
                break;
            }
            case OBJ_BSON_TYPE_UTF8: {
                type = OBJ_INDEX_KEY_TYPE_UTF8 | bits;
                obj_memcpy(cur, &type, 1);
                cur += 1;
                /* len */
                obj_memcpy(cur, &value->value.v_utf8.len, sizeof(obj_int32_t));
                cur += sizeof(obj_int32_t);
                /* content */
                obj_memcpy(cur, value->value.v_utf8.str, value->value.v_utf8.len);
                cur += value->value.v_utf8.len;
                break;
            }
            case OBJ_BSON_TYPE_BINARY: {
                type = OBJ_INDEX_KEY_TYPE_BINARY | bits;
                obj_memcpy(cur, &type, 1);
                cur += 1;
                /* len */
                obj_memcpy(cur, &value->value.v_binary.len, sizeof(obj_int32_t));
                cur += sizeof(obj_int32_t);
                /* content */
                obj_memcpy(cur, value->value.v_binary.data, value->value.v_binary.len);
                cur += value->value.v_binary.len;
                break;
            }
            case OBJ_BSON_TYPE_INT32: {
                type = OBJ_INDEX_KEY_TYPE_INT32 | bits;
                obj_memcpy(cur, &type, 1);
                cur += 1;
                obj_int32_t int32 = value->value.v_int32;
                int32 = obj_int32_to_le(int32);
                obj_memcpy(cur, &int32, sizeof(obj_int32_t));
                cur += sizeof(obj_int32_t);
                break;
            }
            case OBJ_BSON_TYPE_INT64: {
                type = OBJ_INDEX_KEY_TYPE_INT64 | bits;
                obj_memcpy(cur, &type, 1);
                cur += 1;
                obj_int64_t int64 = value->value.v_int64;
                int64 = obj_int64_to_le(int64);
                obj_memcpy(cur, &int64, sizeof(obj_int64_t));
                cur += sizeof(obj_int64_t);
                break;
            }
            default:
                obj_assert(0);
                break;
        }
        if (now_cnt >= cnt) {
            break;
        }
        bits = 0;
    }
    obj_assert(cur - data == expect_size);
    return true;

}

/* destroy index key */
void obj_index_key_destroy(obj_index_key_t *index_key) {
    obj_assert(index_key);
    obj_free(index_key->data);
    obj_free(index_key);
}

/* compare index key elements */
static int obj_index_key_element_compare(obj_uint8_t **p1, obj_uint8_t **p2) {
    int type1 = (**p1 & OBJ_INDEX_KEY_TYPE_MASK);
    int type2 = (**p2 & OBJ_INDEX_KEY_TYPE_MASK);
    obj_assert(type1 == type2);
    /* skip type */
    *p1 = *p1 + 1;
    *p2 = *p2 + 1;
    /* same type */
    switch (type1) {
        case OBJ_INDEX_KEY_TYPE_DOUBLE: {
            double val1, val2;
            obj_memcpy(&val1, *p1, sizeof(double));
            obj_memcpy(&val2, *p2, sizeof(double));
            val1 = obj_double_from_le(val1);
            val2 = obj_double_from_le(val2);
            if (val1 < val2) {
                return -1;
            }
            if (val1 != val2) {
                return 1;
            }
            *p1 = *p1 + sizeof(double);
            *p2 = *p2 + sizeof(double);
            break;
        }
        case OBJ_INDEX_KEY_TYPE_UTF8: {
            obj_int32_t size1 = *((obj_int32_t *)(*p1));
            obj_int32_t size2 = *((obj_int32_t *)(*p2));
            size1 = obj_int32_from_le(size1);
            size2 = obj_int32_from_le(size2);
            obj_int32_t common = (size1 < size2 ? size1 : size2);
            /* skip size */
            *p1 = *p1 + sizeof(obj_int32_t);
            *p2 = *p2 + sizeof(obj_int32_t);
            int res = obj_memcmp(*p1, *p2, common);
            if (res) {
                return res;
            }
            int diff = size1 - size2;
            if (diff) {
                return diff;
            }
            obj_assert(size1 == size2);
            *p1 = *p1 + size1;
            *p2 = *p2 + size2;
            break;
        }
        case OBJ_INDEX_KEY_TYPE_BINARY: {
            obj_int32_t size1 = *((obj_int32_t *)(*p1));
            obj_int32_t size2 = *((obj_int32_t *)(*p2));
            size1 = obj_int32_from_le(size1);
            size2 = obj_int32_from_le(size2);
            obj_int32_t common = (size1 < size2 ? size1 : size2);
            /* skip size */
            *p1 = *p1 + sizeof(obj_int32_t);
            *p2 = *p2 + sizeof(obj_int32_t);
            int res = obj_memcmp(*p1, *p2, common);
            if (res) {
                return res;
            }
            int diff = size1 - size2;
            if (diff) {
                return diff;
            }
            obj_assert(size1 == size2);
            *p1 = *p1 + size1;
            *p2 = *p2 + size2;
            break;
        }
        case OBJ_INDEX_KEY_TYPE_INT32: {
            obj_int32_t val1 = *((obj_int32_t *)(*p1));
            obj_int32_t val2 = *((obj_int32_t *)(*p2));
            val1 = obj_int32_from_le(val1);
            val2 = obj_int32_from_le(val2);
            if (val1 < val2) {
                return -1;
            }
            if (val1 != val2) {
                return 1;
            }
            *p1 = *p1 + sizeof(obj_int32_t);
            *p2 = *p2 + sizeof(obj_int32_t);
            break;
        }
        case OBJ_INDEX_KEY_TYPE_INT64: {
            obj_int64_t val1 = *((obj_int64_t *)(*p1));
            obj_int64_t val2 = *((obj_int64_t *)(*p2));
            val1 = obj_int64_from_le(val1);
            val2 = obj_int64_from_le(val2);
            if (val1 < val2) {
                return -1;
            }
            if (val1 != val2) {
                return 1;
            }
            *p1 = *p1 + sizeof(obj_int64_t);
            *p2 = *p2 + sizeof(obj_int64_t);
            break;
        }
        default:
            obj_assert(0);
            break;
    }
    return 0;
}

/* compare index keys */
int obj_index_key_compare(obj_index_key_t *index_key1, obj_index_key_t *index_key2, obj_index_key_order_t key_order) {
    obj_uint8_t *p1 = index_key1->data;
    obj_uint8_t *p2 = index_key2->data;
    unsigned mask = 1;
    while (true) {
        obj_uint8_t val1 = *p1;
        obj_uint8_t val2 = *p2;
        int res = obj_index_key_element_compare(&p1, &p2);
        if (res) {
            if (obj_index_key_order_descending(key_order, mask)) {
                res = -res;
            }
            return res;
        }
        obj_assert(res == 0);
        res = ((int)(val1 & OBJ_INDEX_KEY_TYPE_HASMORE)) - ((int)(val2 & OBJ_INDEX_KEY_TYPE_HASMORE));
        obj_assert(res == 0);
        
        if ((val1 & OBJ_INDEX_KEY_TYPE_HASMORE) == 0) {
            break;
        }
        mask <<= 1;
    }
    return 0;
}

/* dump index key */
void obj_index_key_dump(obj_index_key_t *index_key) {
    obj_uint8_t *data = index_key->data;
    unsigned mask = 1;
    while (true) {
        obj_uint8_t val = *data;
        data += 1;
        switch (val & OBJ_INDEX_KEY_TYPE_MASK) {
            case OBJ_INDEX_KEY_TYPE_DOUBLE: {
                double val;
                obj_memcpy(&val, data, sizeof(double));
                val = obj_double_from_le(val);
                printf("type: double, val: %lf\n", val);
                data += sizeof(double);
                break;
            }
            case OBJ_INDEX_KEY_TYPE_UTF8: {
                obj_int32_t len = *((obj_int32_t *)data);
                len = obj_int32_from_le(len);
                printf("type: utf-8 string, len: %d\n", len);
                data += sizeof(obj_int32_t);
                data += len;
                break;
            }
            case OBJ_INDEX_KEY_TYPE_BINARY: {
                obj_int32_t len = *((obj_int32_t *)data);
                len = obj_int32_from_le(len);
                printf("type: binary data, len: %d\n", len);
                data += sizeof(obj_int32_t);
                data += len;
                break;
            }
            case OBJ_INDEX_KEY_TYPE_INT32: {
                obj_int32_t val = *((obj_int32_t *)(data));
                val = obj_int32_from_le(val);
                printf("type: int32, val: %d\n", val);
                data += sizeof(obj_int32_t);
                break;
            }
            case OBJ_INDEX_KEY_TYPE_INT64: {
                obj_int64_t val = *((obj_int64_t *)(data));
                val = obj_int64_from_le(val);
                printf("type: int64, val: %ld\n", val);
                data += sizeof(obj_int64_t);
                break;
            }
            default:
                obj_assert(0);
                break;
        }
        if ((val & OBJ_INDEX_KEY_TYPE_HASMORE) == 0) {
            break;
        }
        mask <<= 1;
    }
}