#include "obj_core.h"

static obj_expr_parse_fn obj_expr_parse_get_parser(const char *name);
static obj_expr_type_t obj_expr_parse_get_type(const char *name);
static obj_bool_t obj_expr_parse_is_expression(obj_bson_value_t *value);
static obj_bool_t obj_expr_parse_is_legal_compare_value(const obj_bson_value_t *value);
static obj_bool_t obj_expr_parse_is_legal_equal_value(const obj_bson_value_t *value);
static obj_status_with_t obj_expr_parse_not(const char *name, const obj_bson_value_t *value, obj_expr_parse_level_t current_level);
static obj_status_with_t obj_expr_parse_or(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level);
static obj_status_with_t obj_expr_parse_and(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level);
static obj_status_with_t obj_expr_parse_nor(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level);
static obj_status_with_t obj_expr_parse_top_level(obj_expr_type_t expr_type, const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level);
static obj_status_with_t obj_expr_parse_sub(const char *name, obj_bson_t *bson, obj_expr_base_t *root, obj_expr_parse_level_t current_level);
static obj_status_with_t obj_expr_parse_sub_field(obj_bson_t *bson, const char *name, const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level);

/* parse function map. MUST be added with name order!!! */
static obj_expr_parse_fn_pair_t obj_expr_parse_fn_map[] = {
    {"and", obj_expr_parse_and},
    {"nor", obj_expr_parse_nor},
    {"or", obj_expr_parse_or}
};

/* expression type map. MUST be added with name order!!! */
static obj_expr_parse_type_pair_t obj_expr_parse_type_map[] = {
    {"eq", OBJ_EXPR_EQ},
    {"gt", OBJ_EXPR_GT},
    {"gte", OBJ_EXPR_GTE},
    {"lt", OBJ_EXPR_LT},
    {"lte", OBJ_EXPR_LTE},
    {"neq", OBJ_EXPR_NEQ},
    {"not", OBJ_EXPR_NOT}
};

/* get corresponding parser */
static obj_expr_parse_fn obj_expr_parse_get_parser(const char *name) {
    /* binary search */
    int lo = 0, hi = OBJ_NELEM(obj_expr_parse_fn_map) - 1, mid;
    int res;
    const char *cur_name;
    while (lo < hi) {
        mid = (lo + hi) / 2;
        cur_name = obj_expr_parse_fn_map[mid].name;
        res = obj_strcmp(cur_name, name);
        if (res < 0) {
            lo = mid + 1;
        } else if (res == 0) {
            return obj_expr_parse_fn_map[mid].parser;
        } else {
            hi = mid - 1;
        }
    }
    /* lo >= hi */
    if (lo > hi) {
        return NULL;
    }
    /* lo == hi */
    if (!obj_strcmp(obj_expr_parse_fn_map[lo].name, name)) {
        return obj_expr_parse_fn_map[lo].parser;
    }
    return NULL;
}

/* get corresponding type */
static obj_expr_type_t obj_expr_parse_get_type(const char *name) {
    /* binary search */
    int lo = 0, hi = OBJ_NELEM(obj_expr_parse_type_map) - 1, mid;
    int res;
    const char *cur_name;
    while (lo < hi) {
        mid = (lo + hi) / 2;
        cur_name = obj_expr_parse_type_map[mid].name;
        res = obj_strcmp(cur_name, name);
        if (res < 0) {
            lo = mid + 1;
        } else if (res == 0) {
            return obj_expr_parse_type_map[mid].type;
        } else {
            hi = mid - 1;
        }
    }
    /* lo >= hi */
    if (lo > hi) {
        return -1;
    }
    /* lo == hi */
    if (!obj_strcmp(obj_expr_parse_type_map[lo].name, name)) {
        return obj_expr_parse_type_map[lo].type;
    }
    return -1;
}

/* check if this is an expression */
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
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, &bson);
    const char *key;
    obj_bson_type_t bson_type;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        if (key[0] != '$') {
            return false;
        }
    }
    return true;
}

/* check if this is a legal value for common compare expression($eq, $lt, $lte, $gt, $gte, $neq) */
static inline obj_bool_t obj_expr_parse_is_legal_compare_value(const obj_bson_value_t *value) {
    return value->type == OBJ_BSON_TYPE_DOUBLE || value->type == OBJ_BSON_TYPE_INT32 || value->type == OBJ_BSON_TYPE_INT64 || value->type == OBJ_BSON_TYPE_UTF8 || value->type == OBJ_BSON_TYPE_BINARY;
}

/* check if this is a legal value for $eq | $neq compare expression */
static inline obj_bool_t obj_expr_parse_is_legal_equal_value(const obj_bson_value_t *value) {
    return obj_expr_parse_is_legal_compare_value(value) || value->type == OBJ_BSON_TYPE_BOOL || value->type == OBJ_BSON_TYPE_NULL;
}

/* parse entrance */
obj_status_with_t obj_expr_parse(const obj_bson_t *bson) {
    obj_expr_parse_level_t current_level = OBJ_EXPR_PARSE_LEVEL_PREDICATE_TOP_LEVEL;
    return obj_expr_parse_all(bson, current_level);
}

/* parse bson to expression */
/* TODO clean after error */
obj_status_with_t obj_expr_parse_all(const obj_bson_t *bson, obj_expr_parse_level_t current_level) {
    obj_expr_parse_level_t next_level = (current_level == OBJ_EXPR_PARSE_LEVEL_PREDICATE_TOP_LEVEL) ? OBJ_EXPR_PARSE_LEVEL_USER_OBJ_TOP_LEVEL : current_level;
    obj_bson_iter_t iter;
    obj_bson_type_t bson_type;
    /* corresponding parser */
    obj_expr_parse_fn fn;
    obj_bson_value_t *value;
    obj_expr_base_t *root;
    const char *key;
    obj_status_with_t status;
    obj_bson_iter_init(&iter, bson);
    /* create $and as root */
    root = obj_expr_tree_create(OBJ_EXPR_AND);
    if (root == NULL) {
        return obj_status_with_create(NULL, "can't create expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
    }
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        /* get value */
        obj_bson_value_t *value = (obj_bson_value_t *)obj_bson_iter_value(&iter);
        /* $and/$nor/$or */
        if (key[0] == '$') {
            fn = obj_expr_parse_get_parser(key + 1);
            if (!fn) {
                /* not found. return error */
                return obj_status_with_create(NULL, "parser not found", OBJ_CODE_EXPR_PARSER_NOT_FOUND);
            }
            status = fn(key, value, current_level);
            if (!obj_status_isok(&status)) {
                return status;
            }
            obj_assert(status.data != NULL);
            if (!obj_expr_tree_add_child(root, (obj_expr_base_t *)status.data)) {
                return obj_status_with_create(root, "can't add child to expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
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
            continue;
        }
        /* example: {x: 5} */
        if (!obj_expr_parse_is_legal_compare_value(value)) {
            return obj_status_with_create(root, "compare type error", OBJ_CODE_EXPR_BAD_VALUE);
        }
        obj_expr_base_t *eq = obj_expr_compare_create(key, OBJ_EXPR_EQ, value);
        if (eq == NULL) {
            return obj_status_with_create(root, "can't create child expression: out of memory", OBJ_CODE_EXPR_NOMEM);
        }
        if (!obj_expr_tree_add_child(root, eq)) {
            return obj_status_with_create(root, "can't add child to expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
        }
    }
    /* remove useless root $and */
    if (1 == obj_array_length(((obj_expr_tree_t *)root)->expr_list)) {
        obj_expr_base_t *new_root = (obj_expr_base_t *)obj_array_get_index_value(((obj_expr_tree_t *)root)->expr_list, 0, uintptr_t);
        /* free old root */
        obj_expr_tree_destroy(root);
        return obj_status_with_create(new_root, "", 0);
    }
    return obj_status_with_create(root, "", 0);
}

/* $not */
static obj_status_with_t obj_expr_parse_not(const char *name, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    if (value->type != OBJ_BSON_TYPE_OBJECT) {
        return obj_status_with_create(NULL, "$not need a object", OBJ_CODE_EXPR_BAD_VALUE);
    }
    obj_bson_t bson;
    obj_bson_init_static_with_len(&bson, value->value.v_object.data, value->value.v_object.len);
    if (obj_bson_is_empty(&bson)) {
        return obj_status_with_create(NULL, "$not can't be empty", OBJ_CODE_EXPR_BAD_VALUE);
    }
    obj_expr_base_t *and_expr = obj_expr_tree_create(OBJ_EXPR_AND);
    if (and_expr == NULL) {
        return obj_status_with_create(NULL, "can't create $not: out of memory", OBJ_CODE_EXPR_NOMEM);
    }
    obj_status_with_t sub_status = obj_expr_parse_sub(name, &bson, and_expr, current_level);
    if (!obj_status_isok(&sub_status)) {
        /* obj_expr_tree_destroy(and_expr); */
        return sub_status;
    }
    /* create not */
    obj_expr_base_t *not_expr = obj_expr_not_create(and_expr);
    if (not_expr == NULL) {
        obj_expr_tree_destroy(and_expr);
        return obj_status_with_create(NULL, "can't create $not: out of memory", OBJ_CODE_EXPR_NOMEM);
    }
    return obj_status_with_create(not_expr, "", 0);
}

/* $or */
static obj_status_with_t obj_expr_parse_or(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    return obj_expr_parse_top_level(OBJ_EXPR_OR, key, value, current_level);
}

/* and */
static obj_status_with_t obj_expr_parse_and(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    return obj_expr_parse_top_level(OBJ_EXPR_AND, key, value, current_level);
}

/* nor */
static obj_status_with_t obj_expr_parse_nor(const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    return obj_expr_parse_top_level(OBJ_EXPR_NOR, key, value, current_level);
}

/* parse $or/$and/$nor */
static obj_status_with_t obj_expr_parse_top_level(obj_expr_type_t expr_type, const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    if (value->type != OBJ_BSON_TYPE_ARRAY) {
        /* must be an array */
        return obj_status_with_create(NULL, "$or/$and/$nor must have an array of expressions as condition", OBJ_CODE_EXPR_BAD_VALUE);
    }
    obj_expr_base_t *temp;
    obj_bson_iter_t iter;
    obj_bson_t array_bson;
    obj_bson_type_t bson_type;
    obj_status_with_t sub;
    obj_bson_value_t *child_value;
    const char *child_key;
    temp = obj_expr_tree_create(expr_type);
    if (temp == NULL) {
        return obj_status_with_create(NULL, "can't create expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
    }
    obj_bson_init_static_with_len(&array_bson, value->value.v_array.data, value->value.v_array.len);
    obj_bson_iter_init(&iter, &array_bson);
    while (obj_bson_iter_next_internal(&iter, &child_key, &bson_type)) {
        if (bson_type != OBJ_BSON_TYPE_OBJECT) {
            return obj_status_with_create(NULL, "$or/$and/$nor entries need to be full objects", OBJ_CODE_EXPR_BAD_VALUE);
        }
        /* parse children */
        child_value = (obj_bson_value_t *)obj_bson_iter_value(&iter);
        obj_bson_t sub_bson;
        obj_bson_init_static_with_len(&sub_bson, child_value->value.v_object.data, child_value->value.v_object.len);
        sub = obj_expr_parse_all(&sub_bson, current_level);
        if (!obj_status_isok(&sub)) {
            return sub;
        }
        if (!obj_expr_tree_add_child(temp, (obj_expr_base_t *)sub.data)) {
            return obj_status_with_create(temp, "can't add child to expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
        }
    }
    return obj_status_with_create(temp, "", 0);
}

/* example: {x: {$gt: 5, $lt: 8}} */
static obj_status_with_t obj_expr_parse_sub(const char *name, obj_bson_t *bson, obj_expr_base_t *root, obj_expr_parse_level_t current_level) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, bson);
    const char *key;
    obj_bson_type_t bson_type;
    obj_bson_value_t *value;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        value = (obj_bson_value_t *)obj_bson_iter_value(&iter);
        obj_status_with_t status = obj_expr_parse_sub_field(bson, name, key, value, current_level);
        if (!obj_status_isok(&status)) {
            return status;
        }
        /* add child */
        if (!obj_expr_tree_add_child(root, (obj_expr_base_t *)status.data)) {
            return obj_status_with_create(root, "can't add child to expression tree: out of memory", OBJ_CODE_EXPR_NOMEM);
        }
    }
    return obj_status_with_create(root, "", 0);
}

/**
 * parse a single field in a sub expression
 * if the query is {x: {$gt: 5, $lt: 8}},
 * $gt: 5 is a single field
 */
static obj_status_with_t obj_expr_parse_sub_field(obj_bson_t *bson, const char *name, const char *key, const obj_bson_value_t *value, obj_expr_parse_level_t current_level) {
    obj_expr_type_t expr_type = obj_expr_parse_get_type(key + 1);
    if (expr_type == -1) {
        return obj_status_with_create(NULL, "", OBJ_CODE_EXPR_TYPE_NOT_FOUND);
    }
    obj_expr_base_t *expr;
    switch (expr_type) {
        case OBJ_EXPR_NOT: {
            return obj_expr_parse_not(name, value, current_level);
        }
        case OBJ_EXPR_EQ: {
            if (!obj_expr_parse_is_legal_equal_value(value)) {
                return obj_status_with_create(expr, "illegal type for $eq", OBJ_CODE_EXPR_BAD_VALUE);
            }
            expr = obj_expr_compare_create(name, OBJ_EXPR_EQ, value);
            if (expr == NULL) {
                return obj_status_with_create(NULL, "can't create $eq expression: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            return obj_status_with_create(expr, "", 0);
        }
        case OBJ_EXPR_NEQ: {
            if (!obj_expr_parse_is_legal_equal_value(value)) {
                return obj_status_with_create(expr, "illegal type for $neq", OBJ_CODE_EXPR_BAD_VALUE);
            }
            obj_expr_base_t *eq = obj_expr_compare_create(name, OBJ_EXPR_EQ, value);
            if (eq == NULL) {
                return obj_status_with_create(NULL, "can't create $neq expression: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            expr = obj_expr_not_create(eq);
            if (expr == NULL) {
                obj_expr_compare_destroy(eq);
                return obj_status_with_create(NULL, "can't create $neq expression: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            return obj_status_with_create(expr, "", 0);
        }
        case OBJ_EXPR_LT: {
            if (!obj_expr_parse_is_legal_compare_value(value)) {
                return obj_status_with_create(expr, "illegal type for $lt", OBJ_CODE_EXPR_BAD_VALUE);
            }
            expr = obj_expr_compare_create(name, OBJ_EXPR_LT, value);
            if (expr == NULL) {
                return obj_status_with_create(NULL, "can't create $lt expression: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            return obj_status_with_create(expr, "", 0);
        }
        case OBJ_EXPR_LTE: {
            if (!obj_expr_parse_is_legal_compare_value(value)) {
                return obj_status_with_create(expr, "illegal type for $lte", OBJ_CODE_EXPR_BAD_VALUE);
            }
            expr = obj_expr_compare_create(name, OBJ_EXPR_LTE, value);
            if (expr == NULL) {
                return obj_status_with_create(NULL, "can't create $lte expression: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            return obj_status_with_create(expr, "", 0);
        }
        case OBJ_EXPR_GT: {
            if (!obj_expr_parse_is_legal_compare_value(value)) {
                return obj_status_with_create(expr, "illegal type for $gte", OBJ_CODE_EXPR_BAD_VALUE);
            }
            expr = obj_expr_compare_create(name, OBJ_EXPR_GT, value);
            if (expr == NULL) {
                return obj_status_with_create(NULL, "can't create $gt expression: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            return obj_status_with_create(expr, "", 0);
        }
        case OBJ_EXPR_GTE: {
            if (!obj_expr_parse_is_legal_compare_value(value)) {
                return obj_status_with_create(expr, "illegal type for $gte", OBJ_CODE_EXPR_BAD_VALUE);
            }
            expr = obj_expr_compare_create(name, OBJ_EXPR_GTE, value);
            if (expr == NULL) {
                return obj_status_with_create(NULL, "can't create $gte expression: out of memory", OBJ_CODE_EXPR_NOMEM);
            }
            return obj_status_with_create(expr, "", 0);
        }
        default: 
            break;
    }
    /* should not reach here */
    obj_assert(0);
    return obj_status_with_create(NULL, "", -1);
}