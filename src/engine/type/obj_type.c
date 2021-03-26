#include "obj_core.h"


static obj_uint64_t obj_typename_set_hash_func(void *key);
static int obj_typename_set_key_compare(void *key1, void *key2);
static void *obj_typename_set_key_get(void *data);
static void obj_typename_set_key_set(void *data, void *key);

static obj_bool_t obj_check_type_define_internal(obj_bson_value_t *value);
static obj_bool_t obj_check_type_internal(obj_bson_value_t *proto_value, obj_bson_value_t *data_value);


static obj_prealloc_map_methods_t typename_set_methods = {
    obj_typename_set_hash_func,
    obj_typename_set_key_compare,
    NULL,
    NULL,
    obj_typename_set_key_get,
    NULL,
    obj_typename_set_key_set,
    NULL,
    NULL,
    NULL
};

static obj_uint64_t obj_typename_set_hash_func(void *key) {
    char *str = *(char **)key;
    return obj_set_hash_function(str, obj_strlen(str));
}

static int obj_typename_set_key_compare(void *key1, void *key2) {
    char *str1 = *(char **)key1;
    char *str2 = *(char **)key2;
    return obj_strcmp(str1, str2);
}

static void *obj_typename_set_key_get(void *data) {
    return data;
}

static void obj_typename_set_key_set(void *data, void *key) {
    obj_memcpy(data, key, sizeof(char *));
}


/* type checker check type */
obj_bool_t obj_type_checker_check_type(obj_type_checker_t *checker, obj_bson_t *data) {
    return obj_check_type(checker->prototype, data);
}

/* check type define */
obj_bool_t obj_check_type_define(obj_bson_t *prototype) {
    obj_bson_iter_t iter;
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_value_t *value = NULL;
    obj_bson_iter_init(&iter, prototype);
    /* empty prototype is not allowed */
    if (obj_bson_is_empty(prototype)) {
        return false;
    }
    /* duplicate key is not allowed */
    obj_set_t typename_set;
    obj_set_init(&typename_set, &typename_set_methods, sizeof(char *));
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        if (obj_set_find(&typename_set, &key)) {
            return false;
        }
        obj_set_add(&typename_set, &key);
        value = obj_bson_iter_value(&iter);
        if (!obj_check_type_define_internal(value)) {
            return false;
        }
    }
    return true;
}

static obj_bool_t obj_check_type_define_internal(obj_bson_value_t *value) {
    if (value->type == OBJ_BSON_TYPE_ARRAY) {
        obj_bson_iter_t iter;
        obj_bson_t child;
        char *key = NULL;
        obj_bson_type_t bson_type;
        obj_bson_init_static_with_len(&child, value->value.v_array.data,  value->value.v_array.len);
        obj_bson_iter_init(&iter, &child);
        obj_bson_value_t *child_value = NULL;
        int cnt = 0;
        if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
            return false;
        }
        child_value = obj_bson_iter_value(&iter);
        /* check child recursively */
        if (!obj_check_type_define_internal(child_value)) {
            return false;
        }
        /* only 1 element */
        if (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
            return false;
        }
        return true;

    } else if (value->type == OBJ_BSON_TYPE_OBJECT) {
        obj_bson_iter_t iter;
        obj_bson_t child;
        char *key = NULL;
        obj_bson_type_t bson_type;
        obj_bson_init_static_with_len(&child, value->value.v_object.data,  value->value.v_object.len);
        obj_bson_iter_init(&iter, &child);
        obj_bson_value_t *child_value = NULL;
        int cnt = 0;
        obj_set_t typename_set;
        obj_set_init(&typename_set, &typename_set_methods, sizeof(char *));
        while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
            cnt++;
            if (obj_set_find(&typename_set, &key)) {
                return false;
            }
            obj_set_add(&typename_set, &key);
            child_value = obj_bson_iter_value(&iter);
            /* check child recursively */
            if (!obj_check_type_define_internal(child_value)) {
                return false;
            }
        }
        if (cnt == 0) {
            return false;
        }
        return true;
    } else {
        if (value->type != OBJ_BSON_TYPE_INT32) {
            return false;
        }
        return true;
    }
}

/* check type of data */
obj_bool_t obj_check_type(obj_bson_t *prototype, obj_bson_t *data) {
    obj_bson_iter_t proto_iter;
    char *proto_key = NULL;
    obj_bson_type_t proto_type;
    obj_bool_t proto_next;
    obj_bson_value_t *proto_value = NULL;

    obj_bson_iter_t data_iter;
    char *data_key = NULL;
    obj_bson_type_t data_type;
    obj_bool_t data_next;
    obj_bson_value_t *data_value = NULL;

    obj_bson_iter_init(&proto_iter, prototype);
    obj_bson_iter_init(&data_iter, data);

    while (true) {
        proto_next = obj_bson_iter_next_internal(&proto_iter, &proto_key, &proto_type);
        data_next = obj_bson_iter_next_internal(&data_iter, &data_key, &data_type);
        if (!proto_next && !data_next) {
            break;
        }
        if (proto_next != data_next) {
            return false;
        }
        proto_value = obj_bson_iter_value(&proto_iter);
        data_value = obj_bson_iter_value(&data_iter);
        if (obj_strcmp(proto_key, data_key) != 0) {
            return false;
        }
        if (!obj_check_type_internal(proto_value, data_value)) {
            return false;
        } 
    }
    return true;
}

/* check type recursively */
static obj_bool_t obj_check_type_internal(obj_bson_value_t *proto_value, obj_bson_value_t *data_value) {
    obj_bson_type_t proto_type = proto_value->type;
    obj_bson_type_t data_type = data_value->type;
    
    if (proto_type != OBJ_BSON_TYPE_INT32) {
        if (proto_type != data_type) {
            return false;
        }
        obj_assert(proto_type == OBJ_BSON_TYPE_ARRAY || proto_type == OBJ_BSON_TYPE_OBJECT);
        if (proto_type == OBJ_BSON_TYPE_ARRAY) {
            /* array */
            obj_bson_iter_t proto_iter;
            obj_bson_iter_t data_iter;
            obj_bson_t proto_bson;
            obj_bson_t data_bson;
            char *proto_key = NULL;
            char *data_key = NULL;
            obj_bson_type_t proto_bson_type;
            obj_bson_type_t data_bson_type;
            obj_bson_value_t *child_proto_value = NULL;
            obj_bson_value_t *child_data_value = NULL;
            obj_bson_init_static_with_len(&proto_bson, proto_value->value.v_object.data, proto_value->value.v_object.len);
            obj_bson_init_static_with_len(&data_bson, data_value->value.v_object.data, data_value->value.v_object.len);
            obj_bson_iter_init(&proto_iter, &proto_bson);
            obj_bson_iter_init(&data_iter, &data_bson);
            obj_bson_iter_next_internal(&proto_iter, &proto_key, &proto_bson_type);
            child_proto_value = obj_bson_iter_value(&proto_iter);
            /* allow empty array */
            while (obj_bson_iter_next_internal(&data_iter, &data_key, &data_bson_type)) {
                child_data_value = obj_bson_iter_value(&data_iter);
                if (!obj_check_type_internal(child_proto_value, child_data_value)) {
                    return false;
                }
            }
            return true;

        } else {
            /* object */
            obj_bson_iter_t proto_iter;
            obj_bson_iter_t data_iter;
            obj_bson_t proto_bson;
            obj_bson_t data_bson;
            char *proto_key = NULL;
            char *data_key = NULL;
            obj_bson_type_t proto_bson_type;
            obj_bson_type_t data_bson_type;
            obj_bson_value_t *child_proto_value = NULL;
            obj_bson_value_t *child_data_value = NULL;
            obj_bson_init_static_with_len(&proto_bson, proto_value->value.v_object.data, proto_value->value.v_object.len);
            obj_bson_init_static_with_len(&data_bson, data_value->value.v_object.data, data_value->value.v_object.len);
            obj_bson_iter_init(&proto_iter, &proto_bson);
            obj_bson_iter_init(&data_iter, &data_bson);
            while (true) {
                obj_bool_t proto_next = obj_bson_iter_next_internal(&proto_iter, &proto_key, &proto_bson_type);
                obj_bool_t data_next = obj_bson_iter_next_internal(&data_iter, &data_key, &data_bson_type);
                if (proto_next != data_next) {
                    return false;
                }
                if (!proto_next) {
                    return true;
                }
                if (obj_strcmp(proto_key, data_key) != 0) {
                    return false;
                }
                child_proto_value = obj_bson_iter_value(&proto_iter);
                child_data_value = obj_bson_iter_value(&data_iter);
                /* check recursively */
                if (!obj_check_type_internal(child_proto_value, child_data_value)) {
                    return false;
                }
            }
        }
    } else {
        obj_bson_type_t expect_type = (obj_bson_type_t)proto_value->value.v_int32;
        return expect_type == data_value->type;
    }
}


