#include "obj_core.h"

static obj_bool_t obj_query_index_expr_can_use_index(obj_expr_base_expr_t *expr);


static inline obj_bool_t obj_query_index_expr_can_use_index(obj_expr_base_expr_t *expr) {
    obj_expr_type_t expr_type = expr->type;
    return expr_type >= OBJ_EXPR_TYPE_EQ && expr_type <= OBJ_EXPR_TYPE_GTE;
}
/*
static inline obj_bool_t obj_query_index_expr_is_bounds_generating_not(obj_expr_base_expr_t *expr) {
    return expr->type == OBJ_EXPR_TYPE_NOT && obj_query_index_expr_can_use_index(expr->methods->get_child(expr, 0));
}
*/
static inline obj_bool_t obj_query_index_expr_is_bounds_generating(obj_expr_base_expr_t *expr) {
    return /* obj_query_index_expr_is_bounds_generating_not(expr) || */obj_query_index_expr_can_use_index(expr);
}

/* 
 * get all relevant fields of the query
 * e.x. for query {"a": 1, "b": 2}, return {"a", "b"}
 */
void obj_query_index_get_fields(obj_expr_base_expr_t *root, obj_set_t *out) {
    obj_expr_type_t expr_type = root->type;
    int i;
    /* do not traverse tree beyond a NOR */
    /*
    if (expr_type == OBJ_EXPR_TYPE_NOR) {
        return;
    }
    */
    if (obj_query_index_expr_can_use_index(root)) {
        /* leaf, add to set */
        obj_set_add(out, &((obj_expr_compare_expr_t *)root)->path);
    } else {
        int num_child = root->methods->num_child(root);
        obj_assert(num_child >= 1);
        obj_expr_base_expr_t *child;
        for (i = 0; i < num_child; i++) {
            child = root->methods->get_child(root, i);
            /* get field recursively */
            obj_query_index_get_fields(child, out);
        }
    }
}


/* 
 * find relevant indexes 
 * fields: obj_stringdata_t
 * all_indexes: [obj_bson_t *]
 * out: [obj_bson_t *]
 * e.x.  
 */
void obj_query_index_find_relevant_indexes(obj_set_t *fields, obj_array_t *all_indexes, obj_array_t *out) {
    int i = 0;
    for (i = 0; i < all_indexes->size; i++) {
        obj_bson_t *key_pattern = (obj_bson_t *)obj_array_get_index_value(all_indexes, i, uintptr_t);
        obj_bson_iter_t iter;
        const char *key;
        obj_bson_type_t bson_type;
        obj_bson_iter_init(&iter, key_pattern);
        obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
        obj_stringdata_t field = obj_stringdata_create((char *)key);
        /* add */
        if (obj_set_find(fields, &field)) {
            obj_array_push_back(out, &key_pattern);
        }
    }
}

/* set tags for expression tree */
void obj_query_index_rate_indexes(obj_expr_base_expr_t *root, obj_array_t *indexes) {
    obj_expr_type_t expr_type = root->type;
    /*
    if (expr_type == OBJ_EXPR_TYPE_NOR) {
        return;
    }
    */
    /*
     * e.x. if indexes are [{"a": 1}, {"b": 1}, {"a": 1, "b": 1}] 
     * path is "b"
     * then first: [1], not_first: [2]
     */
    if (obj_query_index_expr_is_bounds_generating(root)) {
        /* haven't tagged */
        obj_assert(root->tag == NULL);
        /* set relevant tag */
        root->tag = obj_expr_relevant_tag_create();
        if (root->tag == NULL) {
            return;
        }
        obj_expr_relevant_tag_t *rt = (obj_expr_relevant_tag_t *)(root->tag);
        /* set tag path */
        /*
        if (root->type == OBJ_EXPR_TYPE_NOT) {
            obj_expr_base_expr_t *child = root->methods->get_child(root, 0);
            obj_assert(child->type >= OBJ_EXPR_TYPE_EQ && child->type <= OBJ_EXPR_TYPE_GTE);
            rt->path = child->methods->get_path(child);
        } else {
            rt->path = root->methods->get_path(root);
        }
        */
        rt->path = root->methods->get_path(root);
        int i;
        for (i = 0; i < indexes->size; i++) {
            obj_bson_iter_t iter;
            const char *key;
            obj_bson_t *key_pattern;
            obj_bson_type_t bson_type;
            key_pattern = (obj_bson_t *)obj_array_get_index_value(indexes, i, uintptr_t);
            obj_bson_iter_init(&iter, key_pattern);
            obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
            if (obj_strcmp(key, rt->path.data) == 0) {
                /* add to first */
                obj_array_push_back(&rt->first, &i);
            }
            /* compound index */
            while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
                if (obj_strcmp(key, rt->path.data) == 0) {
                    obj_array_push_back(&rt->not_first, &i);
                }
            }
        }

    } else if (expr_type == OBJ_EXPR_TYPE_AND || expr_type == OBJ_EXPR_TYPE_OR) {
        int i;
        obj_expr_base_expr_t *child = NULL;
        for (i = 0; i < root->methods->num_child(root); i++) {
            child = root->methods->get_child(root, i);
            obj_query_index_rate_indexes(child, indexes);
        }
    }
}
/*
void obj_query_index_strip_unneeded_assignments(obj_expr_base_expr_t *root, obj_array_t *indexes) {
    if ()
}
*/
/* construct a data access plan */
obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_data_access(obj_expr_base_expr_t *root, obj_array_t *indexes) {
    if (root->type == OBJ_EXPR_TYPE_AND) {
        return obj_query_index_build_indexed_and(root, indexes);
    } else if (root->type == OBJ_EXPR_TYPE_OR) {
        return obj_query_index_build_indexed_or(root, indexes);
    } else {
        /* compare expressions */
        if (obj_query_index_expr_is_bounds_generating(root)) {

        }
    }
}

/* and */
static obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_and(obj_expr_base_expr_t *root, obj_array_t *indexes) {
    obj_array_t index_scan_nodes;
    if (!obj_array_init(&index_scan_nodes, sizeof(obj_query_plan_tree_base_node_t *))) {
        return NULL;
    }
    if (!obj_query_index_process_index_scans(root, indexes, &index_scan_nodes)) {
        return NULL;
    }
    obj_query_plan_tree_base_node_t *and_result;
    /* at least one child can use index */
    obj_assert(index_scan_nodes.size >= 1);
    if (index_scan_nodes.size == 1) {
        and_result = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&index_scan_nodes, 0, uintptr_t);
    } else {
        obj_query_plan_tree_and_node_t *and_node = obj_query_plan_tree_and_node_create();
        if (and_node == NULL) {
            return NULL;
        }
        /* add */
        if (!obj_query_plan_tree_add_children(and_node, &index_scan_nodes)) {
            obj_free(and_node);
            return NULL;
        }
        and_result = and_node;
    }

    return and_result;
}

/* or */
static obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_or(obj_expr_base_expr_t *root, obj_array_t *indexes) {

}

obj_bool_t obj_query_index_process_index_scans(obj_expr_base_expr_t *root, obj_array_t *indexes, obj_array_t *out) {

}

void obj_query_index_scan_build_state_init(obj_query_index_scan_build_state_t *state, obj_expr_base_expr_t *root, obj_array_t *indexes) {
    obj_assert(state);
    state->root = root;
    state->indexes = indexes;
    state->current_scan = NULL;
    state->cur_child = 0;
    state->cur_index = -1;
    
}