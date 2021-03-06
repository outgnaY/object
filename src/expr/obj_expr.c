#include "obj_core.h"


/* tag */
static int obj_expr_tag_compare(const void *a, const void *b);

/* compare expression */
static int obj_expr_compare_expr_num_child(obj_expr_base_expr_t *expr);
static void obj_expr_compare_expr_destroy(obj_expr_base_expr_t *expr);
static char *obj_expr_compare_expr_get_path(obj_expr_base_expr_t *expr);
static obj_bool_t obj_expr_compare_expr_match(obj_expr_base_expr_t *expr, obj_bson_t *bson);
static obj_bool_t obj_expr_compare_expr_match_single(obj_expr_base_expr_t *expr, obj_bson_value_t *value);

/* tree expression */
static int obj_expr_tree_expr_num_child(obj_expr_base_expr_t *expr);
static obj_expr_base_expr_t *obj_expr_tree_expr_get_child(obj_expr_base_expr_t *expr, int i);
static void obj_expr_tree_expr_free_child(void *ptr);
static void obj_expr_tree_expr_destroy(obj_expr_base_expr_t *expr);
static char *obj_expr_tree_expr_get_path(obj_expr_base_expr_t *expr);
static obj_bool_t obj_expr_tree_expr_match(obj_expr_base_expr_t *expr, obj_bson_t *data);
static obj_bool_t obj_expr_tree_expr_match_single(obj_expr_base_expr_t *expr, obj_bson_value_t *value);


/* debug methods */
static void obj_expr_print(obj_expr_base_expr_t *expr, int skip);
static void obj_expr_print_compare(int skip, obj_expr_base_expr_t *expr);
static void obj_expr_print_tree(int skip, obj_expr_base_expr_t *expr);
/*
static void obj_expr_print_not(int skip, obj_expr_base_expr_t *expr);
*/
static void obj_expr_print_value(obj_bson_value_t *value);


static char *obj_expr_type_str_map[] = {
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

/* compare values, for filter */
static int obj_expr_compare_value(obj_bson_value_t *value1, obj_bson_value_t *value2) {
    obj_bson_type_t type1 = value1->type;
    obj_bson_type_t type2 = value2->type;
    obj_assert(type1 == type2);
    switch (type1) {
        case OBJ_BSON_TYPE_DOUBLE:
            if (value1->value.v_double < value2->value.v_double) {
                return -1;
            } else if (value1->value.v_double != value2->value.v_double) {
                return 1;
            } 
            return 0;
        case OBJ_BSON_TYPE_UTF8: {
            int common, len1, len2;
            len1 = value1->value.v_utf8.len;
            len2 = value2->value.v_utf8.len;
            common = (len1 < len2 ? len1 : len2);
            int res = obj_memcmp(value1->value.v_utf8.str, value2->value.v_utf8.str, common);
            if (res) {
                return res;
            }
            return len1 - len2;
        }
        case OBJ_BSON_TYPE_BINARY: {
            int common, len1, len2;
            len1 = value1->value.v_binary.len;
            len2 = value2->value.v_binary.len;
            common = (len1 < len2 ? len1 : len2);
            int res = obj_memcmp(value1->value.v_binary.data, value2->value.v_binary.data, common);
            if (res) {
                return res;
            }
            return len1 - len2;
        }
        case OBJ_BSON_TYPE_INT32:
            if (value1->value.v_int32 < value2->value.v_int32) {
                return -1;
            } else if (value1->value.v_int32 != value2->value.v_int32) {
                return 1;
            } 
            return 0;
        case OBJ_BSON_TYPE_INT64:
            if (value1->value.v_int64 < value2->value.v_int64) {
                return -1;
            } else if (value1->value.v_int64 != value2->value.v_int64) {
                return 1;
            }
            return 0;
        default:
            obj_assert(0);
            obj_assert(NULL);
    }

}



/* ********** tag data ********** */

/* tags must be index tag */
void obj_expr_sort_using_tags(obj_expr_base_expr_t *expr) {
    obj_expr_base_expr_t *child = NULL;
    int num_child = expr->methods->num_child(expr);
    int i;
    for (i = 0; i < num_child; i++) {
        child = expr->methods->get_child(expr, i);
        obj_expr_sort_using_tags(child);
    }
    if (num_child <= 1) {
        return;
    }
    obj_assert(expr->type == OBJ_EXPR_TYPE_OR || expr->type == OBJ_EXPR_TYPE_AND);
    obj_expr_tree_expr_t *tree_expr = (obj_expr_tree_expr_t *)expr;
    obj_array_sort(&tree_expr->expr_list, obj_expr_tag_compare); 
}

static int obj_expr_tag_compare(const void *a, const void *b) {
    obj_expr_base_expr_t *expr1 = *(obj_expr_base_expr_t **)a;
    obj_expr_base_expr_t *expr2 = *(obj_expr_base_expr_t **)b;
    obj_expr_index_tag_t *index_tag1 = (obj_expr_index_tag_t *)(expr1->tag);
    obj_expr_index_tag_t *index_tag2 = (obj_expr_index_tag_t *)(expr2->tag);
    if (index_tag1 == NULL) {
        if (index_tag2 == NULL) {
            return 0;
        }
        return 1;
    }
    /* index_tag1 != NULL */
    if (index_tag2 == NULL) {  
        return -1;
    }
    if (index_tag1->index != index_tag2->index) {
        return index_tag1->index - index_tag2->index;
    }
    return index_tag1->pos - index_tag2->pos;
}

void obj_expr_tag_dump(obj_expr_tag_t *tag) {
    printf("(");
    if (tag->type == OBJ_EXPR_TAG_TYPE_INDEX) {
        obj_expr_index_tag_t *index_tag = (obj_expr_index_tag_t *)tag;
        printf("index tag, index %d position %d", index_tag->index, index_tag->pos);
    } else {
        obj_expr_relevant_tag_t *rt = (obj_expr_relevant_tag_t *)tag;
        printf("relevant tag, path: %s, ", rt->path);
        printf("first: [");
        int i;
        int index;
        for (i = 0; i < rt->first.size; i++) {
            index = obj_array_get_index_value(&rt->first, i, int);
            printf("%d ", index);
        }
        printf("], not first: [");
        for (i = 0; i < rt->not_first.size; i++) {
            index = obj_array_get_index_value(&rt->not_first, i, int);
            printf("%d ", index);
        }
        printf("]");
    }
    printf(")\n");
}

/* set tag data for expression */
inline void obj_expr_set_tag(obj_expr_base_expr_t *expr, obj_expr_tag_t *tag) {
    expr->tag = tag;
}

/* destroy tag */
void obj_expr_tag_destroy(obj_expr_tag_t *tag) {
    if (tag->type == OBJ_EXPR_TAG_TYPE_INDEX) {
        obj_free(tag);
    } else {
        obj_expr_relevant_tag_t *rt = (obj_expr_relevant_tag_t *)tag;
        obj_array_destroy_static(&rt->first);
        obj_array_destroy_static(&rt->not_first);
        obj_free(tag);
    }
}

/* reset tag */
void obj_expr_reset_tag(obj_expr_base_expr_t *expr) {
    if (expr->tag != NULL) {
        obj_expr_tag_destroy(expr->tag);
        expr->tag = NULL;
    }
    int i;
    obj_expr_base_expr_t *child = NULL;
    for (i = 0; i < expr->methods->num_child(expr); i++) {
        child = expr->methods->get_child(expr, i);
        /* reset children tag recursively */
        obj_expr_reset_tag(child);
    }
}

/* create relevant tag */
obj_expr_relevant_tag_t *obj_expr_relevant_tag_create() {
    obj_expr_relevant_tag_t *relevant_tag = obj_alloc(sizeof(obj_expr_relevant_tag_t));
    relevant_tag->base.type = OBJ_EXPR_TAG_TYPE_RELEVANT;
    obj_array_init(&relevant_tag->first, sizeof(int));
    obj_array_init(&relevant_tag->not_first, sizeof(int));
    return relevant_tag;
}

/* create index tag */
obj_expr_index_tag_t *obj_expr_index_tag_create(int i) {
    obj_expr_index_tag_t *index_tag = obj_alloc(sizeof(obj_expr_index_tag_t));
    index_tag->base.type = OBJ_EXPR_TAG_TYPE_INDEX;
    index_tag->index = i;
    index_tag->pos = 0;
    return index_tag;
}

/* create compound index tag */
obj_expr_index_tag_t *obj_expr_index_tag_compound_create(int i, int p) {
    obj_expr_index_tag_t *index_tag = obj_alloc(sizeof(obj_expr_index_tag_t));
    index_tag->base.type = OBJ_EXPR_TAG_TYPE_INDEX;
    index_tag->index = i;
    index_tag->pos = p;
    return index_tag;
}


/* ********** compare expression ********** */

static obj_expr_methods_t compare_expr_methods = {
    obj_expr_compare_expr_num_child,
    NULL,
    obj_expr_compare_expr_destroy,
    obj_expr_compare_expr_get_path,
    obj_expr_compare_expr_match,
    obj_expr_compare_expr_match_single
};

static int obj_expr_compare_expr_num_child(obj_expr_base_expr_t *expr) {
    /* obj_expr_compare_t *compare_expr = (obj_expr_compare_t *)expr; */
    return 0;
}

/* create compare expression */
obj_expr_base_expr_t *obj_expr_compare_expr_create(char *path, obj_expr_type_t type, obj_bson_value_t *value) {
    obj_assert(type == OBJ_EXPR_TYPE_EQ || type == OBJ_EXPR_TYPE_GT || type == OBJ_EXPR_TYPE_GTE || type == OBJ_EXPR_TYPE_LT || type == OBJ_EXPR_TYPE_LTE);
    /* printf("compare value type %d\n", value->type); */
    obj_expr_compare_expr_t *expr;
    expr = obj_alloc(sizeof(obj_expr_compare_expr_t));
    expr->base.type = type;
    expr->base.methods = &compare_expr_methods;
    expr->base.tag = NULL;
    expr->path = (char *)path;
    expr->value = *value;
    return (obj_expr_base_expr_t *)expr;
}

/* destroy compare expression */
static void obj_expr_compare_expr_destroy(obj_expr_base_expr_t *expr) {
    obj_assert(expr && (expr->type == OBJ_EXPR_TYPE_EQ || expr->type == OBJ_EXPR_TYPE_GT || expr->type == OBJ_EXPR_TYPE_GTE || expr->type == OBJ_EXPR_TYPE_LT || expr->type == OBJ_EXPR_TYPE_LTE));
    obj_free(expr);
}

static char *obj_expr_compare_expr_get_path(obj_expr_base_expr_t *expr) {
    obj_expr_compare_expr_t *compare_expr = (obj_expr_compare_expr_t *)expr;
    return compare_expr->path;
}

static obj_bool_t obj_expr_compare_expr_match(obj_expr_base_expr_t *expr, obj_bson_t *bson) {
    /* can't reach here */
    obj_assert(0);
    return false;
}

static obj_bool_t obj_expr_compare_expr_match_single(obj_expr_base_expr_t *expr, obj_bson_value_t *value) {
    obj_expr_compare_expr_t *compare_expr = (obj_expr_compare_expr_t *)expr;
    obj_assert(compare_expr->value.type == value->type);
    int res = obj_expr_compare_value(value, &compare_expr->value);
    /* printf("%d %d res = %d\n", value->value.v_int32, compare_expr->value.value.v_int32, res); */
    switch (expr->type) {
        case OBJ_EXPR_TYPE_EQ:
            return res == 0;
        case OBJ_EXPR_TYPE_GT:
            return res > 0;
        case OBJ_EXPR_TYPE_GTE:
            return res >= 0;
        case OBJ_EXPR_TYPE_LT:
            return res < 0;
        case OBJ_EXPR_TYPE_LTE:
            return res <= 0;
        default:
            obj_assert(0);
            break;
    }
}

/* ********** tree expression ********** */

static obj_expr_methods_t tree_expr_methods = {
    obj_expr_tree_expr_num_child,
    obj_expr_tree_expr_get_child,
    obj_expr_tree_expr_destroy,
    obj_expr_tree_expr_get_path,
    obj_expr_tree_expr_match,
    obj_expr_tree_expr_match_single
};

static int obj_expr_tree_expr_num_child(obj_expr_base_expr_t *expr) {
    obj_expr_tree_expr_t *tree_expr = (obj_expr_tree_expr_t *)expr;
    return tree_expr->expr_list.size;
}

static obj_expr_base_expr_t *obj_expr_tree_expr_get_child(obj_expr_base_expr_t *expr, int i) {
    obj_expr_tree_expr_t *tree_expr = (obj_expr_tree_expr_t *)expr;
    obj_assert(i < tree_expr->expr_list.size);
    obj_expr_base_expr_t *child = (obj_expr_base_expr_t *)obj_array_get_index_value(&tree_expr->expr_list, i, uintptr_t);
    return child;
}

static void obj_expr_tree_expr_free_child(void *ptr) {
    obj_expr_base_expr_t *child = *(obj_expr_base_expr_t **)ptr;
    /* call child's destructor */
    child->methods->destroy(child);
}

/* create tree expression */
obj_expr_base_expr_t *obj_expr_tree_expr_create(obj_expr_type_t type) {
    obj_assert(type == OBJ_EXPR_TYPE_AND || type == OBJ_EXPR_TYPE_OR/* || type == OBJ_EXPR_TYPE_NOR */);
    obj_expr_tree_expr_t *expr;
    expr = obj_alloc(sizeof(obj_expr_tree_expr_t));
    expr->base.type = type;
    expr->base.methods = &tree_expr_methods;
    expr->base.tag = NULL;
    /* init children array */
    obj_array_init(&expr->expr_list, sizeof(obj_expr_base_expr_t *));
    obj_array_set_free(&expr->expr_list, obj_expr_tree_expr_free_child);
    return (obj_expr_base_expr_t *)expr;
}

/* destroy tree expression */
static void obj_expr_tree_expr_destroy(obj_expr_base_expr_t *expr) {
    obj_expr_tree_expr_t *tree_expr = (obj_expr_tree_expr_t *)expr;
    obj_assert(expr && (expr->type == OBJ_EXPR_TYPE_AND || expr->type == OBJ_EXPR_TYPE_OR/* || expr->type == OBJ_EXPR_TYPE_NOR */));
    obj_array_destroy_static(&tree_expr->expr_list);
    obj_free(tree_expr);
}

/* add child to tree expression */
void obj_expr_tree_expr_add_child(obj_expr_base_expr_t *expr, obj_expr_base_expr_t *child) {
    obj_expr_tree_expr_t *tree_expr = (obj_expr_tree_expr_t *)expr;
    obj_assert(expr && (expr->type == OBJ_EXPR_TYPE_AND || expr->type == OBJ_EXPR_TYPE_OR/* || expr->type == OBJ_EXPR_TYPE_NOR */));
    obj_array_push_back(&tree_expr->expr_list, &child);
}

static char *obj_expr_tree_expr_get_path(obj_expr_base_expr_t *expr) {
    return NULL;
}

/* consider: {"$and": [{"a.b", 1}, {"c.d": {"$lt": 7}}]} */
static obj_bool_t obj_expr_tree_expr_match(obj_expr_base_expr_t *expr, obj_bson_t *data) {
    obj_expr_type_t expr_type = expr->type;
    int i;
    obj_expr_base_expr_t *child = NULL;
    obj_bool_t child_match = false;
    obj_bson_iter_t iter;
    char *key;
    obj_bson_value_t *value;
    obj_bson_type_t bson_type;
    if (expr_type == OBJ_EXPR_TYPE_AND) {
        /* AND */
        for (i = 0; i < expr->methods->num_child(expr); i++) {
            child = expr->methods->get_child(expr, i);
            /* all children must match */
            if (child->type >= OBJ_EXPR_TYPE_AND && child->type <= OBJ_EXPR_TYPE_OR) {
                /* tree expr */
                child_match = child->methods->match(child, data);
                if (!child_match) {
                    return false;
                }
            } else {
                /* leaf expr */
                obj_expr_compare_expr_t *leaf_child = (obj_expr_compare_expr_t *)child;
                /* get path */
                char *path = leaf_child->path;
                /* get value according to path */
                value = obj_bson_get_path(data, path);
                child_match = child->methods->match_single(child, value);
                if (!child_match) {
                    return false;
                }
            }
        }
        return true;
    } else {
        /* OR */
        for (i = 0; i < expr->methods->num_child(expr); i++) {
            child = expr->methods->get_child(expr, i);
            /* if any child match */
            if (child->type >= OBJ_EXPR_TYPE_AND && child->type <= OBJ_EXPR_TYPE_OR) {
                /* tree expr */
                child_match = child->methods->match(child, data);
                if (child_match) {
                    return true;
                }
            } else {
                /* leaf expr */
                obj_expr_compare_expr_t *leaf_child = (obj_expr_compare_expr_t *)child;
                /* get path */
                char *path = leaf_child->path;
                /* get value according to path */
                value = obj_bson_get_path(data, path);
                child_match = child->methods->match_single(child, value);
                if (child_match) {
                    return true;
                }
            }
        }
        return false;
    }
}

static obj_bool_t obj_expr_tree_expr_match_single(obj_expr_base_expr_t *expr, obj_bson_value_t *value) {
    /* can't reach */
    obj_assert(0);
    return false;
}

/* ********** debug methods ********** */

/* for debug */
void obj_expr_dump(obj_expr_base_expr_t *expr) {
    obj_expr_print(expr, 0);
}

/* helper function */
static void obj_expr_print(obj_expr_base_expr_t *expr, int skip) {
    if (expr == NULL) {
        return;
    }
    switch (expr->type) {
        /* compare */
        case OBJ_EXPR_TYPE_EQ: 
        case OBJ_EXPR_TYPE_LT:
        case OBJ_EXPR_TYPE_LTE:
        case OBJ_EXPR_TYPE_GT:
        case OBJ_EXPR_TYPE_GTE:
            obj_expr_print_compare(skip, expr);
            break;
        /* $and/$or/$nor */
        case OBJ_EXPR_TYPE_AND:
        case OBJ_EXPR_TYPE_OR:
            obj_expr_print_tree(skip, expr);
            break;
        default:
            obj_assert(0);
    }
}

static void obj_expr_print_compare(int skip, obj_expr_base_expr_t *expr) {
    int i;
    if (expr->tag != NULL) {
        obj_expr_tag_dump(expr->tag);
    }
    for (i = 0; i < skip; i++) {
        printf(" ");
    }
    printf("path: %s", ((obj_expr_compare_expr_t *)expr)->path);
    printf("%s", obj_expr_type_str_map[expr->type]);
    obj_expr_print_value(&((obj_expr_compare_expr_t *)expr)->value);
    printf("\n");
}

static void obj_expr_print_tree(int skip, obj_expr_base_expr_t *expr) {
    int i;
    if (expr->tag != NULL) {
        obj_expr_tag_dump(expr->tag);
    }
    for (i = 0; i < skip; i++) {
        printf(" ");
    }
    printf("%s:\n", obj_expr_type_str_map[expr->type]);
    int len = obj_strlen(obj_expr_type_str_map[expr->type]);
    obj_array_t *arr = &((obj_expr_tree_expr_t *)expr)->expr_list;
    obj_expr_base_expr_t *child;
    for (i = 0; i < obj_array_length(arr); i++) {
        child = (obj_expr_base_expr_t *)obj_array_get_index_value(arr, i, uintptr_t);
        obj_expr_print(child, skip + len + 1);
    }
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
                printf("%02x", value->value.v_binary.data[i]);
            }
            break;
        }
        default: 
            obj_assert(0);
    }
}