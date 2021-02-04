#include "obj_core.h"

static const char *obj_expr_type_str_map[] = {
    "&&",       /* AND */
    "||",       /* OR */
    "==",       /* EQ */
    "<=",       /* LTE */
    "<",        /* LT */
    ">",        /* GT */
    ">=",       /* GTE */
    "!",        /* NOT */
    "!!",       /* NOR */
    "!="        /* NEQ */
};

/* create not expression */
obj_expr_base_t *obj_expr_not_create(obj_expr_base_t *child) {
    obj_expr_not_t *expr = obj_alloc(sizeof(obj_expr_not_t));
    if (expr == NULL) {
        return NULL;
    }
    expr->type.type = OBJ_EXPR_NOT;
    expr->expr = child;
    return expr;
}

/* create compare expression */
obj_expr_base_t *obj_expr_compare_create(const char *path, obj_expr_type_t type, obj_bson_value_t *value) {
    obj_assert(type == OBJ_EXPR_EQ || type == OBJ_EXPR_GT || type == OBJ_EXPR_GTE || type == OBJ_EXPR_LT || type == OBJ_EXPR_LTE);
    obj_expr_compare_t *expr;
    expr = obj_alloc(sizeof(obj_expr_compare_t));
    if (expr == NULL) {
        return NULL;
    }
    expr->type.type = type;
    /*
    int len = obj_strlen(path);
    expr->path = obj_alloc(len + 1);
    if (expr->path == NULL) {
        obj_free(expr);
        return NULL;
    }
    obj_memcpy(expr->path, path, len);
    expr->path[len] = '\0';
    */
    expr->path = path;
    expr->value = *value;
    return (obj_expr_base_t *)expr;
}

/* destroy compare expression */
void obj_expr_compare_destroy(obj_expr_base_t *expr) {
    obj_assert(expr && (expr->type == OBJ_EXPR_EQ || expr->type == OBJ_EXPR_GT || expr->type == OBJ_EXPR_GTE || expr->type == OBJ_EXPR_LT || expr->type == OBJ_EXPR_LTE));
    /*
    if (((obj_expr_compare_t *)expr)->path != NULL) {
        obj_free(((obj_expr_compare_t *)expr)->path);
    }
    */
    obj_free(expr);
}

/* create tree expression */
obj_expr_base_t *obj_expr_tree_create(obj_expr_type_t type) {
    obj_assert(type == OBJ_EXPR_AND || type == OBJ_EXPR_OR || type == OBJ_EXPR_NOR);
    obj_expr_tree_t *expr;
    expr = obj_alloc(sizeof(obj_expr_tree_t));
    if (expr == NULL) {
        return NULL;
    }
    expr->type.type = type;
    /* children */
    expr->expr_list = obj_array_create(sizeof(obj_expr_base_t *));
    if (expr->expr_list == NULL) {
        obj_free(expr);
        return NULL;
    }
    return (obj_expr_base_t *)expr;
}

/* destroy tree expression */
void obj_expr_tree_destroy(obj_expr_base_t *expr) {
    obj_assert(expr && (expr->type == OBJ_EXPR_AND || expr->type == OBJ_EXPR_OR || expr->type == OBJ_EXPR_NOR));
    obj_array_destroy(((obj_expr_tree_t *)expr)->expr_list);
    obj_free(expr);
}

/* add child to tree expression */
obj_bool_t obj_expr_tree_add_child(obj_expr_base_t *expr, obj_expr_base_t *child) {
    obj_assert(expr && (expr->type == OBJ_EXPR_AND || expr->type == OBJ_EXPR_OR || expr->type == OBJ_EXPR_NOR));
    obj_assert(((obj_expr_tree_t *)expr)->expr_list);
    obj_array_t *expr_list = ((obj_expr_tree_t *)expr)->expr_list;
    return obj_array_push_back(expr_list, &child);
}

/* for debug */
void obj_expr_dump(obj_expr_base_t *expr) {
    obj_expr_print(expr, 0);
}

/* helper function */
static void obj_expr_print(obj_expr_base_t *expr, int skip) {
    switch (expr->type) {
        /* compare */
        case OBJ_EXPR_EQ: 
        case OBJ_EXPR_NEQ:
        case OBJ_EXPR_LT:
        case OBJ_EXPR_LTE:
        case OBJ_EXPR_GT:
        case OBJ_EXPR_GTE:
            obj_expr_print_compare(skip, expr);
            break;
        /* $and/$or/$nor */
        case OBJ_EXPR_AND:
        case OBJ_EXPR_OR:
        case OBJ_EXPR_NOR:
            obj_expr_print_tree(skip, expr);
            break;
        case OBJ_EXPR_NOT:
            obj_expr_print_not(skip, expr);
            break;
        default:
            obj_assert(0);
    }
}

static void obj_expr_print_compare(int skip, obj_expr_base_t *expr) {
    int i;
    for (i = 0; i < skip; i++) {
        printf(" ");
    }
    printf("%s", ((obj_expr_compare_t *)expr)->path);
    printf("%s", obj_expr_type_str_map[expr->type]);
    obj_expr_print_value(&((obj_expr_compare_t *)expr)->value);
}

static void obj_expr_print_tree(int skip, obj_expr_base_t *expr) {
    int i;
    for (i = 0; i < skip; i++) {
        printf(" ");
    }
    printf("%s:\n", obj_expr_type_str_map[expr->type]);
    int len = obj_strlen(obj_expr_type_str_map[expr->type]);
    obj_array_t *arr = ((obj_expr_tree_t *)expr)->expr_list;
    obj_expr_base_t *child;
    for (i = 0; i < obj_array_length(arr); i++) {
        child = (obj_expr_base_t *)obj_array_get_index_value(arr, i, uintptr_t);
        obj_expr_print(child, skip + len + 1);
    }
}

static void obj_expr_print_not(int skip, obj_expr_base_t *expr) {
    int i;
    for (i = 0; i < skip; i++) {
        printf(" ");
    }
    printf("!:\n");
    obj_expr_print(((obj_expr_not_t *)expr)->expr, skip + 2);
}

static void obj_expr_print_value(obj_bson_value_t *value) {
    obj_assert(value->type == OBJ_BSON_TYPE_DOUBLE || value->type == OBJ_BSON_TYPE_UTF8 || value->type == OBJ_BSON_TYPE_INT32 || value->type == OBJ_BSON_TYPE_INT64 || value->type == OBJ_BSON_TYPE_BINARY);
    switch (value->type) {
        case OBJ_BSON_TYPE_DOUBLE: 
            printf("%lf", value->value.v_double);
            break;
        case OBJ_BSON_TYPE_UTF8:
            printf("%s", value->value.v_utf8.str);
            break;
        case OBJ_BSON_TYPE_INT32:
            printf("%d", value->value.v_int32);
            break;
        case OBJ_BSON_TYPE_INT64:
            printf("%ld", value->value.v_int64);
            break;
        case OBJ_BSON_TYPE_BINARY: {
            int i;
            for (i = 0; i < value->value.v_binary.len; i++) {
                printf("%02x", value->value.v_binary.data);
            }
            break;
        }
        default: 
            obj_assert(0);
    }
}