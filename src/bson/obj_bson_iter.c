#include "obj_core.h"

/* get current type */
#define OBJ_BSON_ITER_TYPE(iter) (obj_bson_type_t)(*((obj_uint8_t *)((iter)->buf + (iter)->off_type)))

/* key */
static const char *obj_bson_iter_key_unsafe(const obj_bson_iter_t *iter) {
    return (const char *)(iter->buf + iter->off_key);
}

static double obj_bson_iter_double_unsafe(const obj_bson_iter_t *iter) {
    double val;
    obj_memcpy(&val, iter->buf + iter->off_d1, sizeof(val));
    return obj_double_from_le(val);
}

/* utf-8 string length */
static obj_int32_t obj_bson_iter_utf8_len_unsafe(const obj_bson_iter_t *iter) {
    obj_int32_t len;
    obj_memcpy(&len, iter->buf + iter->off_d1, sizeof(len));
    len = obj_int32_from_le(len);
    return len - 1 > 0 ? len - 1 : 0;
}

static obj_int32_t obj_bson_iter_binary_len_unsafe(const obj_bson_iter_t *iter) {
    obj_int32_t len;
    obj_memcpy(&len, iter->buf + iter->off_d1, sizeof(len));
    len = obj_int32_from_le(len);
    return len < 0 ? 0 : len;
}

static obj_int32_t obj_bson_iter_object_len_unsafe(const obj_bson_iter_t *iter) {
    obj_int32_t len;
    obj_memcpy(&len, iter->buf + iter->off_d1, sizeof(len));
    len = obj_int32_from_le(len);
    return len; 
}

static obj_int32_t obj_bson_iter_array_len_unsafe(const obj_bson_iter_t *iter) {
    obj_int32_t len;
    obj_memcpy(&len, iter->buf + iter->off_d1, sizeof(len));
    len = obj_int32_from_le(len);
    return len;
}

static obj_bool_t obj_bson_iter_bool_unsafe(const obj_bson_iter_t *iter) {
    char val;
    obj_memcpy(&val, iter->buf + iter->off_d1, sizeof(val));
    return !!val;
}

static obj_int32_t obj_bson_iter_int32_unsafe(const obj_bson_iter_t *iter) {
    obj_int32_t val;
    obj_memcpy(&val, iter->buf + iter->off_d1, sizeof(val));
    return obj_int32_from_le(val);
}

static obj_int64_t obj_bson_iter_int64_unsafe(const obj_bson_iter_t *iter) {
    obj_int64_t val;
    obj_memcpy(&val, iter->buf + iter->off_d1, sizeof(val));
    return obj_int64_from_le(val);
}

/* called by obj_bson_iter_next */
static obj_bool_t obj_bson_iter_next_internal(obj_bson_iter_t *iter, const char **key, obj_bson_type_t *bson_type) {
    const obj_uint8_t *buf;
    obj_int32_t len;
    int off;
    /* end */
    if (!iter->buf) {
        *key = NULL;
        *bson_type = OBJ_BSON_TYPE_EOO;
        return false;
    }
    buf = iter->buf;
    len = iter->len;
    iter->off = iter->off_next;
    iter->off_type = iter->off;
    iter->off_key = iter->off + 1;
    iter->off_d1 = iter->off_d2 = 0;
    for (off = iter->off_key; off < len; off++) {
        if (!buf[off]) {
            /* end of key */
            iter->off_d1 = ++off;
            goto fill_data_fields;
        }
    }
    goto mark_invalid;
fill_data_fields:
    *key = obj_bson_iter_key_unsafe(iter);
    *bson_type = OBJ_BSON_ITER_TYPE(iter);
    /* handle different bson types */
    switch (*bson_type) {
        case OBJ_BSON_TYPE_EOO:
            iter->off_err = off;
            goto mark_invalid;
        case OBJ_BSON_TYPE_DOUBLE:
            iter->off_next = off + 8;
            break;
        case OBJ_BSON_TYPE_UTF8: {
            obj_int32_t str_len;
            /* bound check */
            if (off >= len - 4) {
                iter->off_err = off;
                goto mark_invalid;
            }
            iter->off_d2 = off + 4;
            obj_memcpy(&str_len, iter->buf + iter->off_d1, sizeof(str_len));
            str_len = obj_int32_from_le(str_len);
            /* bound check */
            if (str_len <= 0 || str_len >= (len - (off + 4))) {
                iter->off_err = off;
                goto mark_invalid;
            }
            iter->off_next = off + 4 + str_len;
            break;
        }
        case OBJ_BSON_TYPE_OBJECT: {
            obj_int32_t l;
            if (off >= len - 4) {
                iter->off_err = off;
                goto mark_invalid;
            }
            obj_memcpy(&l, iter->buf + iter->off_d1, sizeof(l));
            l = obj_int32_from_le(l);
            /* ? */
            if (l > len || l >= len - off) {
                iter->off_err = off;
                goto mark_invalid;
            }
            iter->off_next = off + l;
            break;
        }
        case OBJ_BSON_TYPE_ARRAY: {
            obj_int32_t l;
            if (off >= len - 4) {
                iter->off_err = off;
                goto mark_invalid;
            }
            obj_memcpy(&l, iter->buf + iter->off_d1, sizeof(l));
            l = obj_int32_from_le(l);
            /* ? */
            if (l > len || l >= len - off) {
                iter->off_err = off;
                goto mark_invalid;
            }
            iter->off_next = off + l;
            break;
        }
        case OBJ_BSON_TYPE_BINARY: {
            obj_int32_t bin_len;
            /* bound check */
            if (off >= len - 4) {
                iter->off_err = off;
                goto mark_invalid;
            }
            iter->off_d2 = off + 4;
            obj_memcpy(&bin_len, iter->buf + iter->off_d1, sizeof(bin_len));
            bin_len = obj_int32_from_le(bin_len);
            /* bound check */
            if (bin_len < 0 || bin_len >= (len - (off + 4))) {
                iter->off_err = off;
                goto mark_invalid;
            }
            iter->off_next = iter->off + 4 + bin_len;
            break;
        }
        case OBJ_BSON_TYPE_BOOL: {
            char val;
            /* bound check */
            if (iter->off_d1 >= len) {
                iter->off_err = off;
                goto mark_invalid;
            }
            obj_memcpy(&val, iter->buf + iter->off_d1, sizeof(val));
            if (val != 0x00 && val != 0x01) {
                /* erro bool value */
                iter->off_err = off;
                goto mark_invalid;
            }
            iter->off_next = off + 1;
            break;
        }
        case OBJ_BSON_TYPE_NULL:
            iter->off_next = off;
            break;
        case OBJ_BSON_TYPE_INT32:
            iter->off_next = off + 4;
            break;
        case OBJ_BSON_TYPE_INT64:
            iter->off_next = off + 8;
            break;
        default:
            iter->off_err = off;
            goto mark_invalid;
    }
    if (iter->off_next >= len) {
        iter->off_err = off;
        goto mark_invalid;
    }
    iter->off_err = 0;
    return true;
mark_invalid:
    iter->buf = NULL;
    iter->len = 0;
    iter->off_next = 0;
    return false;
}


/* must validate bson before iter!!! */
obj_bool_t obj_bson_iter_init(obj_bson_iter_t *iter, obj_bson_t *bson) {
    obj_assert(iter);
    obj_assert(bson);
    if (bson->len < 5) {
        return false;
    }
    
    return true;
}



double obj_bson_iter_double(const obj_bson_iter_t *iter) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_DOUBLE) {
        return obj_bson_iter_double_unsafe(iter);
    }
    return 0;
}


const char *obj_bson_iter_utf8(const obj_bson_iter_t *iter, obj_int32_t *len) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_UTF8) {
        if (len) {
            *len = obj_bson_iter_utf8_len_unsafe(iter);
        }
        return (const char *)(iter->buf + iter->off_d2);
    }
    if (len) {
        *len = 0;
    }
    return NULL;
}

const obj_uint8_t *obj_bson_iter_binary(const obj_bson_iter_t *iter, obj_int32_t *len) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_BINARY) {
        if (len) {
            *len = obj_bson_iter_binary_len_unsafe(iter);
        }
        return (const obj_uint8_t *)(iter->buf + iter->off_d2);
    }
    if (len) {
        *len = 0;
    }
    return NULL;
}

const obj_uint8_t *obj_bson_iter_object(const obj_bson_iter_t *iter, obj_int32_t *len) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_OBJECT) {
        if (len) {
            *len = obj_bson_iter_object_len_unsafe(iter);
        }
        /* !! */
        return (const obj_uint8_t *)(iter->buf + iter->off_d1);
    }
    if (len) {
        *len = 0;
    }
    return NULL;
}

const obj_uint8_t *obj_bson_iter_array(const obj_bson_iter_t *iter, obj_int32_t *len) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_ARRAY) {
        if (len) {
            *len = obj_bson_iter_array_len_unsafe(iter);
        }
        /* !! */
        return (const obj_uint8_t *)(iter->buf + iter->off_d1);
    }
    if (len) {
        *len = 0;
    }
    return NULL;
}

obj_bool_t obj_bson_iter_bool(const obj_bson_iter_t *iter) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_BOOL) {
        return obj_bson_iter_bool_unsafe(iter);
    }
    return false;
}


obj_int32_t obj_bson_iter_int32(const obj_bson_iter_t *iter) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_INT32) {
        return obj_bson_iter_int32_unsafe(iter);
    }
    return 0;
}

obj_int64_t obj_bson_iter_int64(const obj_bson_iter_t *iter) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_INT64) {
        return obj_bson_iter_int64_unsafe(iter);
    }
    return 0;
}

/* advance the iterator to the next field */
obj_bool_t obj_bson_iter_next(obj_bson_iter_t *iter) {
    obj_bson_type_t bson_type;
    const char *key;
    return obj_bson_iter_next_internal(iter, &key, &bson_type);
}

/* current element */
const obj_bson_value_t *obj_bson_iter_value(obj_bson_iter_t *iter) {
    obj_bson_value_t *value;
    value = &iter->value;
    value->type = OBJ_BSON_ITER_TYPE(iter);
    switch (value->type) {
        case OBJ_BSON_TYPE_DOUBLE:
            value->value.v_double = obj_bson_iter_double(iter);
            break;
        case OBJ_BSON_TYPE_UTF8:
            value->value.v_utf8.str = (char *)obj_bson_iter_utf8(iter, &value->value.v_utf8.len);
            break;
        case OBJ_BSON_TYPE_OBJECT:
            value->value.v_object.data = (obj_uint8_t *)obj_bson_iter_object(iter, &value->value.v_object.len);
            break;
        case OBJ_BSON_TYPE_ARRAY:
            value->value.v_array.data = (obj_uint8_t *)obj_bson_iter_array(iter, &value->value.v_array.len);
            break;
        case OBJ_BSON_TYPE_BINARY:
            value->value.v_binary.data = (obj_uint8_t *)obj_bson_iter_binary(iter, &value->value.v_binary.len);
            break;
        case OBJ_BSON_TYPE_BOOL:
            value->value.v_bool = obj_bson_iter_bool(iter);
            break;
        case OBJ_BSON_TYPE_NULL:
            break;
        case OBJ_BSON_TYPE_INT32:
            value->value.v_int32 = obj_bson_iter_int32(iter);
            break;
        case OBJ_BSON_TYPE_INT64:
            value->value.v_int64 = obj_bson_iter_int64(iter);
            break;
        default:
            return NULL;
    }
    return value;
}









