#include "obj_core.h"

/* parser map. MUST be added with order!!! */
static obj_expr_parser_t obj_expr_parsers[] = {
    {"and", obj_expr_parse_and},
    {"nor", obj_expr_parse_nor},
    {"or", obj_expr_parse_or}
};

/* get corresponding parser */
static obj_expr_parse_fn obj_expr_parse_get_parser(const char *name) {
    /* binary search */
    int lo = 0, hi = OBJ_NELEM(obj_expr_parsers) - 1, mid;
    int res;
    const char *cur_name;
    while (lo < hi) {
        mid = (lo + hi) / 2;
        cur_name = obj_expr_parsers[mid].name;
        res = obj_strcmp(cur_name, name);
        if (res < 0) {
            lo = mid + 1;
        } else if (res == 0) {
            return obj_expr_parsers[mid].parser;
        } else {
            hi = mid - 1;
        }
    }
    /* lo >= hi */
    if (lo > hi) {
        return NULL;
    }
    if (!obj_strcmp(obj_expr_parsers[lo].name, name)) {
        return obj_expr_parsers[lo].parser;
    }
    return NULL;
}

/* if this is an expression */
static obj_bool_t obj_expr_parse_is_expression(obj_bson_value_t *value) {
    if (value->type != OBJ_BSON_TYPE_OBJECT) {
        return false;
    }
    obj_bson_t bson;
    obj_bson_init_static_with_len(&bson, value->value.v_object.data, value->value.v_object.len);
    /* check if empty */
    if (obj_bson_is_empty(&bson)) {
        return false;
    }
    /* TODO check elements? */
    return true;
}

/* parse entrance */
obj_status_with_t obj_expr_parse(const obj_bson_t *bson) {
    obj_expr_parse_level_t current_level = OBJ_EXPR_PARSE_LEVEL_PREDICATE_TOP_LEVEL;
    return obj_expr_parse_all(bson, current_level);
}

/* parse bson to expression */
obj_status_with_t obj_expr_parse_all(const obj_bson_t *bson, obj_expr_parse_level_t current_level) {
    obj_expr_parse_level_t next_level = (current_level == OBJ_EXPR_PARSE_LEVEL_PREDICATE_TOP_LEVEL) ? OBJ_EXPR_PARSE_LEVEL_USER_OBJ_TOP_LEVEL : current_level;
    obj_bson_iter_t iter;
    obj_bson_type_t bson_type;
    /* corresponding parser */
    obj_expr_parse_fn fn;
    obj_bson_value_t *value;
    obj_expr_base_t *root;
    char *key;
    obj_status_with_t status;
    obj_bson_iter_init(&iter, bson);
    /* create $and as root */
    root = obj_expr_tree_create(OBJ_EXPR_AND);
    if (root == NULL) {
        return obj_status_create(NULL, "can't create expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
    }
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        /* get value */
        obj_bson_value_t *value = obj_bson_iter_value(&iter);
        /* $and/$nor/$or */
        if (key[0] == '$') {
            fn = obj_expr_parse_get_parser(key + 1);
            if (!fn) {
                /* not found. return error */
                return obj_status_create(NULL, "parser not found", OBJ_CODE_EXPR_PARSER_NOT_FOUND);
            }
            status = fn(key, value, current_level);
            if (!obj_status_isok(&status)) {
                return status;
            }
            obj_assert(status.data != NULL);
            if (!obj_expr_tree_add_child(root, (obj_expr_base_t *)status.data)) {
                return obj_status_create(root, "can't add child to expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            continue;
        }
        /* example: {x: {$gt: 5, $lt: 8}} */
        if (obj_expr_parse_is_expression(value)) {
            obj_bson_t sub_bson;
            obj_bson_init_static_with_len(&sub_bson, value->value.v_object.data, value->value.v_object.len);
            status = obj_expr_parse_sub(key, &sub_bson, root, next_level);
            if (!obj_status_isok(&status)) {
                return status;
            }
            if (!obj_expr_tree_add_child(root, (obj_expr_base_t *)status.data)) {
                return obj_status_create(root, "can't add child to expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            continue;
        }
        /* example: {x: 5} */
        obj_
        obj_expr_tree_add_child(root, )
    }
    /* remove useless root $and */
    if (1 == obj_array_length(((obj_expr_tree_t *)root)->expr_list)) {
        obj_expr_base_t *new_root = (obj_expr_base_t *)obj_array_get_index_value(((obj_expr_tree_t *)root)->expr_list, 0, uintptr_t);
        /* free old root */
        
    }
    return root;
}

/* $or */
obj_status_with_t obj_expr_parse_or(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    return obj_expr_parse_top_level(OBJ_EXPR_OR, key, value, current_level);
}

/* and */
obj_status_with_t obj_expr_parse_and(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    return obj_expr_parse_top_level(OBJ_EXPR_AND, key, value, current_level);
}

/* nor */
obj_status_with_t obj_expr_parse_nor(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    return obj_expr_parse_top_level(OBJ_EXPR_NOR, key, value, current_level);
}

/* parse $or/$and/$nor */
obj_status_with_t obj_expr_parse_top_level(obj_expr_type_t expr_type, const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    if (value->type != OBJ_BSON_TYPE_ARRAY) {
        /* must be an array */
        return obj_status_create(NULL, "$or/$and/$nor must have an array of expressions as condition", OBJ_CODE_EXPR_BAD_VALUE);
    }
    obj_expr_base_t *temp;
    obj_bson_iter_t iter;
    obj_bson_t array_bson;
    obj_bson_type_t bson_type;
    obj_status_with_t sub;
    obj_bson_value_t *value;
    char *key;
    temp = obj_expr_tree_create(expr_type);
    if (temp == NULL) {
        return obj_status_create(NULL, "can't create expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
    }
    obj_bson_init_static_with_len(&array_bson, value->value.v_array.data, value->value.v_array.len);
    obj_bson_iter_init(&iter, &array_bson);
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        if (bson_type != OBJ_BSON_TYPE_OBJECT) {
            return obj_status_create(NULL, "$or/$and/$nor entries need to be full objects", OBJ_CODE_EXPR_BAD_VALUE);
        }
        /* parse children */
        value = obj_bson_iter_value(&iter);
        obj_bson_t sub_bson;
        obj_bson_init_static_with_len(&sub_bson, value->value.v_object.data, value->value.v_object.len);
        sub = obj_expr_parse_all(&sub_bson, current_level);
        if (obj_status_isok(&sub)) {
            return sub;
        }
        if (!obj_expr_tree_add_child(temp, (obj_expr_base_t *)sub.data)) {
            return obj_status_create(temp, "can't add child to expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
        }
    }
    return obj_status_create(temp, "", 0);
}

/* example: {x: {$gt: 5, $lt: 8}} */
obj_status_with_t obj_expr_parse_sub(const char *name, obj_bson_t *bson, obj_expr_base_t *root, obj_expr_parse_level_t current_level) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, bson);
    char *key;
    obj_bson_type_t bson_type;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        obj_status_with_t status = obj_expr_parse_sub_field(bson, );
        if (!obj_status_isok(&status)) {
            return status;
        }
        /* add child */
        if (!obj_expr_tree_add_child(root, (obj_expr_base_t *)status.data)) {
            return obj_status_create(root, "can't add child to expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
        }
    }
    return obj_status_create(root, "", 0);
}

/**
 * parse a single field in a sub expression
 * if the query is {x: {$gt: 5, $lt: 8}},
 * $gt: 5 is a single field
 */
obj_status_with_t obj_expr_parse_sub_field(obj_bson_t *bson, const char *name, const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    /* eq */
    if (!obj_strcmp("$eq", key)) {

    }
    /**/
}

/* parse ==($eq) */
obj_status_with_t obj_expr_parse_comparison() {

}



// obj_expr_parse_compare()