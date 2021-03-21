#include "obj_core.h"

static char *find_cmd_name = "find";
static char *filter_field = "filter";
static char *projection_field = "projection";
static char *sort_field = "sort";
static char *skip_field = "skip";
static char *limit_field = "limit";


/* query request methods */
static void obj_query_request_init(obj_query_request_t *qr);
/* standard query methods */
static int obj_query_expression_compare(const void *a, const void *b);
static void obj_query_expression_tree_sort(obj_expr_base_expr_t *root);

/* ********** query request methods ********** */

/* parse query request from find command */
obj_status_with_t obj_query_parse_from_find_cmd(obj_bson_t *cmd) {
    obj_assert(cmd);
    obj_query_request_t *qr = obj_alloc(sizeof(obj_query_request_t));
    obj_query_request_init(qr);
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, cmd);
    char *key;
    obj_bson_type_t bson_type;
    obj_bson_value_t *value = NULL;
    /* iterate through  */
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        value = (obj_bson_value_t *)obj_bson_iter_value(&iter);
        if (obj_strcmp(find_cmd_name, key) == 0) {
            if (bson_type != OBJ_BSON_TYPE_UTF8) {
                obj_free(qr);
                return obj_status_with_create(NULL, "collection name must be of type string", OBJ_CODE_QUERY_WRONG_TYPE);
            }
            qr->full_name = value->value.v_utf8.str;
        } else if (obj_strcmp(filter_field, key) == 0) {
            if (bson_type != OBJ_BSON_TYPE_OBJECT) {
                obj_free(qr);
                return obj_status_with_create(NULL, "filter field must be of type object", OBJ_CODE_QUERY_WRONG_TYPE);
            }
            /* assign */
            obj_bson_init_static_with_len(&qr->filter, value->value.v_object.data, value->value.v_object.len);
        } else if (obj_strcmp(projection_field, key) == 0) {
            if (bson_type != OBJ_BSON_TYPE_OBJECT) {
                obj_free(qr);
                return obj_status_with_create(NULL, "projection field must be of type object", OBJ_CODE_QUERY_WRONG_TYPE);
            }
            /* assign */
            obj_bson_init_static_with_len(&qr->projection, value->value.v_object.data, value->value.v_object.len);
        } else if (obj_strcmp(sort_field, key) == 0) {
            if (bson_type != OBJ_BSON_TYPE_OBJECT) {
                obj_free(qr);
                return obj_status_with_create(NULL, "sort field must be of type object", OBJ_CODE_QUERY_WRONG_TYPE);
            }
            /* assign */
            obj_bson_init_static_with_len(&qr->sort, value->value.v_object.data, value->value.v_object.len);
        } else if (obj_strcmp(skip_field, key) == 0) {
            if (bson_type != OBJ_BSON_TYPE_INT32) {
                obj_free(qr);
                return obj_status_with_create(NULL, "skip field must be of type int32", OBJ_CODE_QUERY_WRONG_TYPE);
            }
            qr->skip = value->value.v_int32;
        } else if (obj_strcmp(limit_field, key) == 0) {
            if (bson_type != OBJ_BSON_TYPE_INT32) {
                obj_free(qr);
                return obj_status_with_create(NULL, "limit field must be of type int32", OBJ_CODE_QUERY_WRONG_TYPE);
            }
            qr->limit = value->value.v_int32;
        }
        /* else if (obj_strcmp(hint_field, key) == 0) {

        } */
    }
    if (qr->full_name == NULL) {
        obj_free(qr);
        return obj_status_with_create(NULL, "missing full name", OBJ_CODE_QUERY_MISSING_FIELD);
    }
    return obj_status_with_create(qr, "", OBJ_CODE_OK);
}

/* dump query request */
void obj_query_request_dump(obj_query_request_t *qr) {
    printf("query request:\n");
    printf("full name: %s\n", qr->full_name);
    /* filter */
    if (!obj_bson_is_empty(&qr->filter)) {
        printf("filter:\n");
        obj_bson_visit_print_visit(&qr->filter);
        printf("\n");
    }
    /* projection */
    if (!obj_bson_is_empty(&qr->projection)) {
        printf("projection:\n");
        obj_bson_visit_print_visit(&qr->projection);
        printf("\n");
    }
    /* sort */
    if (!obj_bson_is_empty(&qr->sort)) {
        printf("sort:\n");
        obj_bson_visit_print_visit(&qr->sort);
        printf("\n");
    }
    /* skip */
    if (qr->skip != -1) {
        printf("skip: %d\n", qr->skip);
    }
    /* limit */
    if (qr->limit != -1) {
        printf("limit: %d\n", qr->limit);
    }
}

/* init query request */
static inline void obj_query_request_init(obj_query_request_t *qr) {
    obj_memset(qr, 0, sizeof(obj_query_request_t));
    qr->skip = qr->limit = -1;
}

/* ********** standard query methods ********** */

/* transform query request to standard query */
obj_status_with_t obj_query_standardize(obj_query_request_t *qr) {
    obj_assert(qr);
    obj_standard_query_t *sq = obj_alloc(sizeof(obj_standard_query_t));
    obj_expr_base_expr_t *root = NULL;
    /* has filter expression */
    if (!obj_bson_is_empty(&qr->filter)) {
        obj_status_with_t parse_status = obj_expr_parse(&qr->filter);
        /* parse error occurred */
        if (parse_status.code != OBJ_CODE_OK) {
            obj_free(sq);
            return obj_status_with_create(NULL, "parse expression tree error", parse_status.code);
        }
        obj_assert(parse_status.data != NULL);
        root = (obj_expr_base_expr_t *)parse_status.data;
        
    }
    /* init standard query */
    obj_standard_query_init(sq, qr, root);
    return obj_status_with_create(sq, "", OBJ_CODE_OK);
}

static int obj_query_expression_compare(const void *a, const void *b) {
    obj_expr_base_expr_t *expr1 = (obj_expr_base_expr_t *)a;
    obj_expr_base_expr_t *expr2 = (obj_expr_base_expr_t *)b;
    /* compare type */
    if (expr1->type != expr2->type) {
        return expr1->type - expr2->type;
    }
    /* same type */
    /* if have path, compare path */
    int path_compare;
    if (expr1->type >= OBJ_EXPR_TYPE_EQ && expr1->type <= OBJ_EXPR_TYPE_GTE) {
        path_compare = obj_strcmp(((obj_expr_compare_expr_t *)expr1)->path, ((obj_expr_compare_expr_t *)expr2)->path);
        if (path_compare != 0) {
            return path_compare;
        }
    }
    int num_child1 = expr1->methods->num_child(expr1);
    int num_child2 = expr2->methods->num_child(expr2);
    int num_child = (num_child1 < num_child2 ? num_child1 : num_child2);
    obj_expr_base_expr_t *child1 = NULL;
    obj_expr_base_expr_t *child2 = NULL;
    int i, child_compare;
    for (i = 0; i < num_child; i++) {
        child1 = expr1->methods->get_child(expr1, i);
        child2 = expr2->methods->get_child(expr2, i);
        child_compare = obj_query_expression_compare(child1, child2);
        if (child_compare != 0) {
            return child_compare;
        }
    }
    if (num_child1 != num_child2) {
        return num_child1 < num_child2 ? -1 : 1;
    }
    /* equal */
    return 0;
}

/* sort expression tree */
static void obj_query_expression_tree_sort(obj_expr_base_expr_t *root) {
    int i;
    obj_expr_base_expr_t *child = NULL;
    /* sort every child recursively */
    for (i = 0; i < root->methods->num_child(root); i++) {
        child = root->methods->get_child(root, i);
        obj_query_expression_tree_sort(child);
    }
    /* sort children */
    if (root->methods->num_child(root) > 1) {
        /* array sort */
        obj_array_sort(&((obj_expr_tree_expr_t *)root)->expr_list, obj_query_expression_compare);
    }
    
}

/* init standard query */
inline void obj_standard_query_init(obj_standard_query_t *sq, obj_query_request_t *qr, obj_expr_base_expr_t *root) {
    obj_assert(sq);
    obj_assert(qr);
    /* optimize expression tree */
    root = obj_expr_optimize_tree(root);
    /* sort expression tree */
    obj_query_expression_tree_sort(root);
    sq->qr = qr;
    sq->root = root;
}

/* dump standard query */
void obj_standard_query_dump(obj_standard_query_t *sq) {
    printf("++++++++++ query request: ++++++++++\n");
    obj_query_request_dump(sq->qr);
    printf("\n");
    printf("++++++++++ query expression: ++++++++++\n");
    obj_expr_dump(sq->root);
    printf("\n");
}

