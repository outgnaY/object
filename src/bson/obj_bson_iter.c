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
    obj_int32_t val;
    obj_memcpy(&val, iter->buf + iter->off_d1, sizeof(val));
    val = obj_int32_from_le(val);
    return val - 1 > 0 ? val - 1 : 0;
}

/* utf-8 string */
static const char *obj_bson_iter_utf8_unsafe(const obj_bson_iter_t *iter, obj_int32_t *len) {
    *len = obj_bson_iter_utf8_len_unsafe(iter);
    return (const char *)(iter->buf + iter->off_d2);
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
    iter->off = iter->next_off;
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
        case OBJ_BSON_TYPE_DOUBLE:
        case OBJ_BSON_TYPE_INT64:
            iter->next_off = off + 8;
            break;
        case OBJ_BSON_TYPE_BOOL:
        
    }

mark_invalid:

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



/* check if has next */
/*
obj_bool_t obj_bson_iter_has_next(obj_bson_iter_t *iter) {

}

void obj_bson_iter_next(obj_bson_iter_t *iter) {

}
*/

double obj_bson_iter_double(const obj_bson_iter_t *iter) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_DOUBLE) {
        return obj_bson_iter_double_unsafe(iter);
    }
    return 0;
}


const char *obj_bson_iter_utf8(const obj_bson_iter_t *iter, obj_int32_t *len) {
    if (OBJ_BSON_ITER_TYPE(iter) == OBJ_BSON_TYPE_UTF8) {

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
obj_bool_t obj_bson_iter_next() {

}




/* visitor */
obj_bool_t obj_bson_iter_visit_all(obj_bson_iter_t *iter, const obj_bson_visitor_t *visitor, void *data) {

}



