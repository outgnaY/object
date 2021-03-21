#include "obj_core.h"

/* execute query plan tree */

/* build query plan tree */
/*
obj_plan_tree_base_node_t *obj_plan_tree_build() {

}
*/

/* TODO: stats */

/* for plan tree node which has only one child */
static obj_plan_tree_base_node_t *obj_plan_tree_get_child(obj_plan_tree_base_node_t *node) {
    obj_array_t *children = &node->children;
    obj_assert(children->size == 1);
    obj_plan_tree_base_node_t *child = NULL;
    child = (obj_plan_tree_base_node_t *)obj_array_get_index_value(children, 0, uintptr_t);
    return child;
}

/* ********** eof node ********** */

obj_plan_tree_eof_node_t *obj_plan_tree_eof_node_create() {
    obj_plan_tree_eof_node_t *node = (obj_plan_tree_eof_node_t *)obj_alloc(sizeof(obj_plan_tree_eof_node_t));
    if (node == NULL) {
        return NULL;
    }
    /* probably don't need to init array */
    if (!obj_array_init(&node->base.children, sizeof(obj_plan_tree_base_node_t *))) {
        obj_free(node);
        return NULL;
    }
    return node;
}

/* get type */
static obj_plan_tree_node_type_t obj_plan_tree_eof_node_get_type() {
    return OBJ_PLAN_TREE_NODE_TYPE_EOF;
}

/* eof node work() */
static obj_plan_tree_exec_state_t obj_plan_tree_eof_node_work(obj_plan_tree_base_node_t *node, obj_plan_working_set_id_t *out) {
    return OBJ_PLAN_TREE_STATE_EOF;
}

/* is eof */
static obj_bool_t obj_plan_tree_eof_node_is_eof(obj_plan_tree_base_node_t *node) {
    return true;
}

/* ********** fetch node ********** */
/*
obj_plan_tree_fetch_node_t *obj_plan_tree_fetch_node_create() {

}
*/
/* get type */
/*
static obj_plan_tree_node_type_t obj_plan_tree_fetch_node_get_type() {
    return OBJ_PLAN_TREE_NODE_TYPE_FETCH;
}
*/
/* fetch node work() */
/*
static obj_plan_tree_exec_state_t obj_plan_tree_fetch_node_work(obj_plan_tree_base_node_t *node, obj_plan_working_set_id_t *out) {
    if (node->methods->is_eof(node)) {
        return OBJ_PLAN_TREE_STATE_EOF;
    }
    obj_plan_working_set_id_t id = OBJ_PLAN_WORKING_SET_INVALID_ID;
    obj_plan_tree_base_node_t *child = obj_plan_tree_get_child(node);
    obj_plan_tree_exec_state_t state = child->methods->work(child, &id);
    if (state == OBJ_PLAN_TREE_STATE_ADVANCED) {

    } else if (state == OBJ_PLAN_TREE_STATE_INTERNAL_ERROR) {
        
    }
    return state;
}
*/
/* is eof */
/*
static obj_bool_t obj_plan_tree_fetch_node_is_eof(obj_plan_tree_base_node_t *node) {
    obj_plan_tree_fetch_node_t *fetch_node = (obj_plan_tree_fetch_node_t *)node;
    obj_plan_tree_base_node_t *child = obj_plan_tree_get_child(node);
    return child->methods->is_eof(child);
}
*/

/* ********** collection scan node ********** */

obj_plan_tree_collection_scan_node_t *obj_plan_tree_collection_scan_node_create(obj_plan_tree_collection_scan_params_t params, obj_plan_working_set_t *ws, obj_expr_base_t *filter) {
    obj_plan_tree_collection_scan_node_t *node = (obj_plan_tree_collection_scan_node_t *)obj_alloc(sizeof(obj_plan_tree_collection_scan_node_t));
    if (node == NULL) {
        return NULL;
    }
    
    node->params = params;
    node->ws = ws;
    node->filter = filter;
    return node;
}

/* get type */
static obj_plan_tree_node_type_t obj_plan_tree_collection_scan_node_get_type() {
    return OBJ_PLAN_TREE_NODE_TYPE_COLLECTION_SCAN;
}

/* collection scan node work() */
static obj_plan_tree_exec_state_t obj_plan_tree_collection_scan_node_work(obj_plan_working_set_id_t *out) {

}

/* is eof */
static obj_bool_t obj_plan_tree_collection_scan_node_is_eof(obj_plan_tree_base_node_t *node) {
    obj_plan_tree_collection_scan_node_t *collection_scan_node = (obj_plan_tree_collection_scan_node_t *)node;
}

/* ********** index scan node ********** */

obj_plan_tree_index_scan_node_t *obj_plan_tree_index_scan_node_create() {
    obj_plan_tree_index_scan_node_t *node = (obj_plan_tree_index_scan_node_t *)obj_alloc(sizeof(obj_plan_tree_index_scan_node_t));
    if (node == NULL) {
        return NULL;
    }

    return node;
}

/* get type */
static obj_plan_tree_node_type_t obj_plan_tree_index_scan_get_type() {
    return OBJ_PLAN_TREE_NODE_TYPE_INDEX_SCAN;
}

/* index scan node work() */
static obj_plan_tree_exec_state_t obj_plan_tree_index_scan_node_work(obj_plan_working_set_id_t *out) {

}

/* is eof */
static obj_bool_t obj_plan_tree_index_scan_node_is_eof(obj_plan_tree_base_node_t *node) {
    obj_plan_tree_index_scan_node_t *index_scan_node = (obj_plan_tree_index_scan_node_t *)node;
}

/* ********** sort node ********** */

obj_plan_tree_sort_node_t *obj_plan_tree_sort_node_create(obj_plan_tree_sort_params_t params, obj_plan_working_set_t *ws, obj_plan_tree_base_node_t *child) {
    obj_plan_tree_sort_node_t *node = (obj_plan_tree_sort_node_t *)obj_alloc(sizeof(obj_plan_tree_sort_node_t *));
    if (node == NULL) {
        return NULL;
    }
    if (!obj_array_init(&node->base.children, sizeof(obj_plan_tree_base_node_t *))) {
        obj_free(node);
        return NULL;
    }
    /* add child */
    if (!obj_array_push_back(&node->base.children, &child)) {
        obj_free(node);
        return NULL;
    }
    if (!obj_array_init(&node->data, sizeof(obj_plan_tree_sort_node_data_item_t))) {
        obj_array_destroy_static(&node->base.children);
        obj_free(node);
        return NULL;
    }
    node->params = params;
    node->ws = ws;
    node->mem_usage = 0;
    node->sorted = false;
    /* current position */
    node->curr = -1;
    return node;
}

/* get type */
static obj_plan_tree_node_type_t obj_plan_tree_sort_node_get_type() {
    return OBJ_PLAN_TREE_NODE_TYPE_SORT;
}

/* compare */
static int obj_plan_tree_sort_node_data_item_compare(const void *data_item1, const void *data_item2) {
    obj_plan_tree_sort_node_data_item_t *sort_node_data_item1 = (obj_plan_tree_sort_node_data_item_t *)data_item1;
    obj_plan_tree_sort_node_data_item_t *sort_node_data_item2 = (obj_plan_tree_sort_node_data_item_t *)data_item2;
    /* TODO optimize */
    return obj_bson_compare(sort_node_data_item1->sort_key, sort_node_data_item2->sort_key, sort_node_data_item1->pattern);
}

/* sort node work() */
static obj_plan_tree_exec_state_t obj_plan_tree_sort_node_work(obj_plan_tree_base_node_t *node, obj_plan_working_set_id_t *out) {
    obj_plan_tree_sort_node_t *sort_node = (obj_plan_tree_sort_node_t *)node;
    /* check memory usage */

    /* all works are done, return eof */
    if (node->methods->is_eof(node)) {
        return OBJ_PLAN_TREE_STATE_EOF;
    }
    
    /* if not sorted yet */
    if (!sort_node->sorted) {
        obj_plan_working_set_id_t id = OBJ_PLAN_WORKING_SET_INVALID_ID;
        obj_plan_tree_base_node_t *child = obj_plan_tree_get_child(node);
        obj_plan_tree_exec_state_t state = child->methods->work(child, &id);
        if (state == OBJ_PLAN_TREE_STATE_ADVANCED) {
            obj_plan_working_set_member_t *member = obj_plan_working_set_get(sort_node->ws, id);
            obj_plan_tree_sort_node_data_item_t data_item;
            data_item.ws_id = id;
            /* TODO set sort key */
            data_item.sort_key = ;
            /* set pattern */
            data_item.pattern = sort_node->params.pattern;
            /* add to data set */
            obj_array_push_back(&sort_node->data, &data_item);
            return OBJ_PLAN_TREE_STATE_NEED_TIME;
        } else if (state == OBJ_PLAN_TREE_STATE_EOF) {
            /* child done. do the sort */
            obj_array_sort(&sort_node->data, obj_plan_tree_sort_node_data_item_compare);
            /* set to begin */
            sort_node->curr = 0;
            sort_node->sorted = true;
            return OBJ_PLAN_TREE_STATE_NEED_TIME;
        } else if (state == OBJ_PLAN_TREE_STATE_INTERNAL_ERROR) {
            /* TODO error occurred */
        }

        return state;
    }
    /* already sorted */
    obj_assert(sort_node->sorted);
    /* we haven't returned all results */
    obj_assert(sort_node->data.size != sort_node->curr);
    obj_plan_tree_sort_node_data_item_t *data_item = (obj_plan_tree_sort_node_data_item_t *)obj_array_get_index(&sort_node->data, sort_node->curr);
    *out = data_item->ws_id;
    sort_node->curr++;
    return OBJ_PLAN_TREE_STATE_ADVANCED;
}

/* is eof */
static obj_bool_t obj_plan_tree_sort_node_is_eof(obj_plan_tree_base_node_t *node) {
    obj_plan_tree_sort_node_t *sort_node = (obj_plan_tree_sort_node_t *)node;
    obj_plan_tree_base_node_t *child = obj_plan_tree_get_child(node);
    /* 
     * done if:
     * 1. child has no more results 
     * 2. results from child are sorted
     * 3. we have returned all results
     */
    return child->methods->is_eof(child) && sort_node->sorted && sort_node->data.size == sort_node->curr;
}

/* ********** projection node ********** */

obj_plan_tree_projection_node_t *obj_plan_tree_projection_node_create() {
    obj_plan_tree_projection_node_t *node = (obj_plan_tree_projection_node_t *)obj_alloc(sizeof(obj_plan_tree_projection_node_t));
    if (node == NULL) {
        return NULL;
    }

    return node;
}

/* get type */
static obj_plan_tree_node_type_t obj_plan_tree_projection_node_get_type() {
    return OBJ_PLAN_TREE_NODE_TYPE_PROJECTION;
}

/* projection node work() */
static obj_plan_tree_exec_state_t obj_plan_tree_projection_node_work(obj_plan_working_set_id_t *out) {

}

/* is eof */
static obj_bool_t obj_plan_tree_projection_node_is_eof(obj_plan_tree_base_node_t *node) {
    obj_plan_tree_projection_node_t *projection_node = (obj_plan_tree_projection_node_t *)node;
}

/* ********** skip node ********** */

obj_plan_tree_skip_node_t *obj_plan_tree_skip_node_create(obj_plan_working_set_t *ws, obj_plan_tree_base_node_t *child, int skip) {
    obj_plan_tree_skip_node_t *node = (obj_plan_tree_skip_node_t *)obj_alloc(sizeof(obj_plan_tree_skip_node_t));
    if (node == NULL) {
        return NULL;
    }
    if (!obj_array_init(&node->base.children, sizeof(obj_plan_tree_base_node_t *))) {
        obj_free(node);
        return NULL;
    }
    /* add child */
    if (!obj_array_push_back(&node->base.children, &child)) {
        obj_free(node);
        return NULL;
    }
    node->ws = ws;
    node->num_to_skip = skip;
    return node;
}

/* get type */
static obj_plan_tree_node_type_t obj_plan_tree_skip_node_get_type() {
    return OBJ_PLAN_TREE_NODE_TYPE_SKIP;
}

/* skip node work() */
static obj_plan_tree_exec_state_t obj_plan_tree_skip_node_work(obj_plan_tree_base_node_t *node, obj_plan_working_set_id_t *out) {
    obj_plan_tree_skip_node_t *skip_node = (obj_plan_tree_skip_node_t *)node;
    obj_plan_working_set_id_t id = OBJ_PLAN_WORKING_SET_INVALID_ID;
    obj_plan_tree_base_node_t *child = obj_plan_tree_get_child(node);
    obj_plan_tree_exec_state_t state = child->methods->work(child, &id);
    if (state == OBJ_PLAN_TREE_STATE_ADVANCED) {
        /* skip if need */
        if (skip_node->num_to_skip > 0) {
            --skip_node->num_to_skip;
            obj_plan_working_set_free(skip_node->ws, id);
            return OBJ_PLAN_TREE_STATE_NEED_TIME;
        }
        *out = id;
        /* try again */
        return OBJ_PLAN_TREE_STATE_NEED_TIME;
    } else if (state == OBJ_PLAN_TREE_STATE_INTERNAL_ERROR) {
        /* TODO error occurred */

    }
    return state;
}

/* is eof */
static obj_bool_t obj_plan_tree_skip_node_is_eof(obj_plan_tree_base_node_t *node) {
    obj_plan_tree_skip_node_t *skip_node = (obj_plan_tree_skip_node_t *)node;
    obj_plan_tree_base_node_t *child = obj_plan_tree_get_child(node);
    return child->methods->is_eof(child);
}

/* ********** limit node ********** */

obj_plan_tree_limit_node_t *obj_plan_tree_limit_node_create(obj_plan_working_set_t *ws, obj_plan_tree_base_node_t *child, int limit) {
    obj_plan_tree_limit_node_t *node = (obj_plan_tree_limit_node_t *)obj_alloc(sizeof(obj_plan_tree_limit_node_t));
    if (node == NULL) {
        return NULL;
    }
    if (!obj_array_init(&node->base.children, sizeof(obj_plan_tree_base_node_t *))) {
        obj_free(node);
        return NULL;
    }
    /* add child */
    if (!obj_array_push_back(&node->base.children, &child)) {
        obj_free(node);
        return NULL;
    }
    node->ws = ws;
    node->num_to_return = limit;
    return node;
}

/* get type */
static obj_plan_tree_node_type_t obj_plan_tree_limit_node_get_type() {
    return OBJ_PLAN_TREE_NODE_TYPE_LIMIT;
}

/* limit node work() */
static obj_plan_tree_exec_state_t obj_plan_tree_limit_node_work(obj_plan_tree_base_node_t *node, obj_plan_working_set_id_t *out) {
    obj_plan_tree_limit_node_t *limit_node = (obj_plan_tree_limit_node_t *)node;
    if (limit_node->num_to_return == 0) {
        return OBJ_PLAN_TREE_STATE_EOF;
    }
    obj_plan_working_set_id_t id = OBJ_PLAN_WORKING_SET_INVALID_ID;
    obj_plan_tree_base_node_t *child = obj_plan_tree_get_child(node);
    obj_plan_tree_exec_state_t state = child->methods->work(child, &id);
    if (state == OBJ_PLAN_TREE_STATE_ADVANCED) {
        /* returned one */
        *out = id;
        --limit_node->num_to_return;
    } else if (state == OBJ_PLAN_TREE_STATE_INTERNAL_ERROR) {
        /* TODO error occurred */
    }

    return state;
}

/* is eof */
static obj_bool_t obj_plan_tree_limit_node_is_eof(obj_plan_tree_base_node_t *node) {
    obj_plan_tree_limit_node_t *limit_node = (obj_plan_tree_limit_node_t *)node;
    obj_plan_tree_base_node_t *child = obj_plan_tree_get_child(node);
    return limit_node->num_to_return == 0 || child->methods->is_eof(child);
}