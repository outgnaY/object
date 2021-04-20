#include "obj_core.h"



/* methods for indexed plan building */
static obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_and(obj_expr_base_expr_t *root, obj_array_t *indexes);
static obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_or(obj_expr_base_expr_t *root, obj_array_t *indexes);
static obj_bool_t obj_query_index_should_merge_with_leaf(obj_expr_base_expr_t *expr, obj_query_index_scan_build_state_t *state);
static void obj_query_index_merge_with_leaf(obj_expr_base_expr_t *expr, obj_query_index_scan_build_state_t *state);
static obj_query_plan_tree_base_node_t *obj_query_index_make_leaf_node(obj_index_catalog_entry_t *index_entry, int pos, obj_expr_base_expr_t *expr);
static void obj_query_index_finish_leaf_node(obj_query_plan_tree_base_node_t *node, obj_index_catalog_entry_t *index_entry);
static void obj_query_index_finish_and_output_leaf(obj_query_index_scan_build_state_t *state, obj_array_t *out);
static obj_bool_t obj_query_index_process_index_scans(obj_expr_base_expr_t *root, obj_array_t *indexes, obj_array_t *out, obj_bool_t *full_indexed);
static obj_bool_t obj_query_index_process_index_scans_subnode(obj_query_index_scan_build_state_t *state, obj_array_t *out);
static void obj_query_index_scan_build_state_init(obj_query_index_scan_build_state_t *state, obj_expr_base_expr_t *root, obj_array_t *indexes);
static void obj_query_index_scan_build_state_reset(obj_query_index_scan_build_state_t *state, obj_expr_index_tag_t *tag);



inline obj_bool_t obj_query_index_expr_can_use_index(obj_expr_base_expr_t *expr) {
    obj_expr_type_t expr_type = expr->type;
    return expr_type >= OBJ_EXPR_TYPE_EQ && expr_type <= OBJ_EXPR_TYPE_GTE;
}

inline obj_bool_t obj_query_index_expr_is_bounds_generating(obj_expr_base_expr_t *expr) {
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
 * fields: char *
 * all_indexes: [obj_index_catalog_entry_t]
 * out: [obj_index_catalog_entry_t]
 * e.x.  
 */
void obj_query_index_find_relevant_indexes(obj_set_t *fields, obj_array_t *all_indexes, obj_array_t *out) {
    int i = 0;
    for (i = 0; i < all_indexes->size; i++) {
        obj_index_catalog_entry_t *index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(all_indexes, i);
        obj_bson_iter_t iter;
        char *key;
        obj_bson_type_t bson_type;
        obj_bson_iter_init(&iter, index_entry->key_pattern);
        obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
        /* add */
        if (obj_set_find(fields, &key)) {
            obj_array_push_back(out, index_entry);
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
        root->tag = (obj_expr_tag_t *)obj_expr_relevant_tag_create();
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
            char *key;
            obj_index_catalog_entry_t *index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(indexes, i);
            obj_bson_type_t bson_type;
            obj_bson_iter_init(&iter, index_entry->key_pattern);
            obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
            if (obj_strcmp(key, rt->path) == 0) {
                /* add to first */
                obj_array_push_back(&rt->first, &i);
            }
            /* compound index */
            while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
                if (obj_strcmp(key, rt->path) == 0) {
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







/* ********** build indexed plan ********** */




/* 
 * Entrance: construct a data access plan 
 */
obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_data_access(obj_expr_base_expr_t *root, obj_array_t *indexes) {
    if (root->type == OBJ_EXPR_TYPE_AND) {
        return obj_query_index_build_indexed_and(root, indexes);
    } else if (root->type == OBJ_EXPR_TYPE_OR) {
        return obj_query_index_build_indexed_or(root, indexes);
    } else {
        /* not use an index */
        if (!root->tag) {
            return NULL;
        }
        /* compare expressions */
        if (obj_query_index_expr_is_bounds_generating(root)) {
            obj_expr_index_tag_t *index_tag = (obj_expr_index_tag_t *)root->tag;
            obj_index_catalog_entry_t *index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(indexes, index_tag->index);
            obj_query_plan_tree_base_node_t *node = obj_query_index_make_leaf_node(index_entry, index_tag->pos, root);
            obj_query_index_finish_leaf_node(node, index_entry);
            return node;
        }
        obj_assert(0);
    }
    return NULL;
}

/* 
 * and 
 */
static obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_and(obj_expr_base_expr_t *root, obj_array_t *indexes) {
    obj_array_t index_scan_nodes;
    obj_array_init(&index_scan_nodes, sizeof(obj_query_plan_tree_base_node_t *));
    /* try to generate index scan plans */
    obj_bool_t full_indexed = false;
    if (!obj_query_index_process_index_scans(root, indexes, &index_scan_nodes, &full_indexed)) {
        return NULL;
    }
    obj_query_plan_tree_base_node_t *and_result = NULL;
    /* at least one child can use index */
    obj_assert(index_scan_nodes.size >= 1);
    if (index_scan_nodes.size == 1) {
        and_result = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&index_scan_nodes, 0, uintptr_t);
    } else {
        obj_query_plan_tree_and_node_t *and_node = obj_query_plan_tree_and_node_create();
        /* add */
        obj_query_plan_tree_add_children((obj_query_plan_tree_base_node_t *)and_node, &index_scan_nodes);
        and_result = (obj_query_plan_tree_base_node_t *)and_node;
    }
    if (!full_indexed) {
        and_result->filter = root;
    }

    return and_result;
}

/*
 * or. an or is indexed iff all children of or are indexed  
 */
static obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_or(obj_expr_base_expr_t *root, obj_array_t *indexes) {
    obj_array_t index_scan_nodes;
    obj_array_init(&index_scan_nodes, sizeof(obj_query_plan_tree_base_node_t *));
    obj_bool_t full_indexed = false;
    /* try to generate index scan plans */
    if (!obj_query_index_process_index_scans(root, indexes, &index_scan_nodes, &full_indexed)) {
        return NULL;
    }
    /* all children must be indexed */
    if (!full_indexed) {
        return NULL;
    }
    obj_query_plan_tree_base_node_t *or_result = NULL;
    if (index_scan_nodes.size == 1) {
        or_result = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&index_scan_nodes, 0, uintptr_t);
    } else {
        obj_query_plan_tree_or_node_t *or_node = obj_query_plan_tree_or_node_create();
        /* add */
        obj_query_plan_tree_add_children((obj_query_plan_tree_base_node_t *)or_node, &index_scan_nodes);
        or_result = (obj_query_plan_tree_base_node_t *)or_node;
    }
    obj_assert(full_indexed);
    return or_result;
}

static obj_bool_t obj_query_index_should_merge_with_leaf(obj_expr_base_expr_t *expr, obj_query_index_scan_build_state_t *state) {
    obj_query_plan_tree_base_node_t *node = state->current_scan;
    if (node == NULL || expr == NULL) {
        return false;
    }
    /* not use index */
    if (state->index_tag == NULL) {
        return false;
    }
    /* not use same index */
    if (state->cur_index != state->index_tag->index) {
        return false;
    }
    int pos = state->index_tag->pos;
    obj_index_catalog_entry_t *index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(state->indexes, state->cur_index);
    obj_query_plan_tree_node_type_t node_type = node->methods->get_type();
    obj_expr_type_t merge_type = state->root->type;
    obj_assert(node_type == OBJ_QUERY_PLAN_TREE_NODE_TYPE_INDEX_SCAN);
    obj_query_plan_tree_index_scan_node_t *index_scan_node = (obj_query_plan_tree_index_scan_node_t *)node;
    /* deal with index bounds? */
    return true;
}

/* merge expr with current node */
static void obj_query_index_merge_with_leaf(obj_expr_base_expr_t *expr, obj_query_index_scan_build_state_t *state) {
    obj_query_plan_tree_base_node_t *node = state->current_scan;
    obj_assert(node != NULL);
    obj_expr_type_t root_type = state->root->type;
    obj_index_catalog_entry_t *index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(state->indexes, state->cur_index);
    int pos = state->index_tag->pos;
    obj_query_plan_tree_index_scan_node_t *index_scan_node = (obj_query_plan_tree_index_scan_node_t *)node;
    obj_index_bounds_t *bounds_to_fill = NULL;
    /* index bounds to fill */
    bounds_to_fill = &index_scan_node->bounds;
    int i;
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, index_entry->key_pattern);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    for (i = 0; i < pos; i++) {
        obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
    }
    obj_bson_value_t *value = NULL;
    value = (obj_bson_value_t *)obj_bson_iter_value(&iter);
    obj_ordered_interval_list_t *oil = (obj_ordered_interval_list_t *)obj_array_get_index(&bounds_to_fill->fields, pos);
    if (oil->name == NULL) {
        obj_assert(oil->intervals.size == 0);
        obj_index_bounds_translate(expr, key, value, oil);
    } else {
        if (root_type == OBJ_EXPR_TYPE_AND) {
            /* AND */
            obj_index_bounds_translate_and_intersect(expr, key, value, oil);
        } else if (root_type = OBJ_EXPR_TYPE_OR) {
            /* OR */
            obj_index_bounds_translate_and_union(expr, key, value, oil);
        }
    }
}

/*
static void obj_query_index_handle_filter(obj_query_index_scan_build_state_t *state) {
    if (state->root->type == OBJ_EXPR_TYPE_OR) {
        obj_query_index_handle_filter_or(state);
    } else if (state->root->type == OBJ_EXPR_TYPE_AND) {
        obj_query_index_handle_filter_and(state);
    } else {
        obj_assert(0);
    }
}
*/
/*
static void obj_query_index_handle_filter_or(obj_query_index_scan_build_state_t *state) {

}

static void obj_query_index_handle_filter_and(obj_query_index_scan_build_state_t *state) {

}
*/
static obj_query_plan_tree_base_node_t *obj_query_index_make_leaf_node(obj_index_catalog_entry_t *index_entry, int pos, obj_expr_base_expr_t *expr) {
    obj_query_plan_tree_index_scan_node_t *index_scan_node = obj_query_plan_tree_index_scan_node_create(index_entry);
    obj_array_init_size(&index_scan_node->bounds.fields, sizeof(obj_ordered_interval_list_t), index_entry->nfields);
    obj_array_resize(&index_scan_node->bounds.fields, index_entry->nfields);
    int i;
    obj_ordered_interval_list_t *oil = NULL;
    for (i = 0; i < index_entry->nfields; i++) {
        oil = (obj_ordered_interval_list_t *)obj_array_get_index(&index_scan_node->bounds.fields, i);
        /* init array */
        obj_ordered_interval_list_init(oil);
    }
    oil = NULL;
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, index_entry->key_pattern);
    char *key = NULL;
    obj_bson_value_t *value = NULL;
    obj_bson_type_t bson_type;
    oil = (obj_ordered_interval_list_t *)obj_array_get_index(&index_scan_node->bounds.fields, pos);
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    for (i = 0; i < pos; i++) {
        /* safe check */
        obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
    }
    value = obj_bson_iter_value(&iter);
    obj_index_bounds_translate(expr, key, value, oil);
    return (obj_query_plan_tree_base_node_t *)index_scan_node;
}

static void obj_query_index_finish_leaf_node(obj_query_plan_tree_base_node_t *node, obj_index_catalog_entry_t *index_entry) {
    obj_assert(node->methods->get_type() == OBJ_QUERY_PLAN_TREE_NODE_TYPE_INDEX_SCAN);
    obj_query_plan_tree_index_scan_node_t *index_scan_node = (obj_query_plan_tree_index_scan_node_t *)node;
    obj_index_bounds_t *bounds = &index_scan_node->bounds;
    int first_empty;
    obj_ordered_interval_list_t *oil = NULL;
    for (first_empty = 0; first_empty < bounds->fields.size; first_empty++) {
        oil = (obj_ordered_interval_list_t *)obj_array_get_index(&bounds->fields, first_empty);
        if (oil->name == NULL) {
            obj_assert(oil->intervals.size == 0);
            break;
        }
    }
    if (first_empty == bounds->fields.size) {
        /* set direction */
        obj_index_bounds_align_bounds(bounds, index_entry->key_pattern);
        return;
    }
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, index_entry->key_pattern);
    int i;
    char *key = NULL;
    obj_bson_type_t bson_type;
    for (i = 0; i < first_empty; i++) {
        obj_assert(obj_bson_iter_next_internal(&iter, &key, &bson_type));
    }
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        oil = (obj_ordered_interval_list_t *)obj_array_get_index(&bounds->fields, first_empty);
        if (oil->name == NULL) {
            obj_assert(oil->intervals.size == 0);
            /* set interval */
            /* (MIN, MAX) */
            obj_interval_t interval;
            interval.empty = false;
            interval.start_inclusive = false;
            interval.end_inclusive = false;
            interval.start = g_interval_value_min;
            interval.end = g_interval_value_max;
            oil->name = key;
            /* add to list */
            obj_array_push_back(&oil->intervals, &interval);
        }
        ++first_empty;
    }
    obj_assert(first_empty == bounds->fields.size);
    /* set direction */
    obj_index_bounds_align_bounds(bounds, index_entry->key_pattern);
}

static void obj_query_index_finish_and_output_leaf(obj_query_index_scan_build_state_t *state, obj_array_t *out) {
    obj_index_catalog_entry_t *index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(state->indexes, state->cur_index);
    obj_query_index_finish_leaf_node(state->current_scan, index_entry);
    /*
    if (state->root->type == OBJ_EXPR_TYPE_OR) {

    }
    */
    obj_array_push_back(out, &state->current_scan);
}

static obj_bool_t obj_query_index_process_index_scans(obj_expr_base_expr_t *root, obj_array_t *indexes, obj_array_t *out, obj_bool_t *full_indexed) {
    obj_query_index_scan_build_state_t state;
    obj_query_index_scan_build_state_init(&state, root, indexes);
    obj_expr_base_expr_t *child = NULL;
    *full_indexed = true;
    while (state.cur_child < root->methods->num_child(root)) {
        child = root->methods->get_child(root, state.cur_child);
        /* if there is no tag, it's not using an index. because we have sorted children with tags first, stop */
        if (child->tag == NULL) {
            *full_indexed = false;
            break;
        }
        obj_assert(child->tag->type == OBJ_EXPR_TAG_TYPE_INDEX);
        state.index_tag = (obj_expr_index_tag_t *)child->tag;
        obj_assert(state.index_tag->index != -1);
        if (!obj_query_index_expr_is_bounds_generating(child)) {
            /* AND/OR */
            if (!obj_query_index_process_index_scans_subnode(&state, out)) {
                return false;
            }
            continue;
        }
        /* compare expression */
        /* try to merge */
        if (obj_query_index_should_merge_with_leaf(child, &state)) {
            /* can merge */
            obj_assert(state.cur_index == state.index_tag->index);
            /* do merge */
            obj_query_index_merge_with_leaf(child, &state);
            /* obj_query_index_handle_filter(&state); */

        } else {
            /* can't merge */
            if (state.current_scan != NULL) {
                /* add current_scan to output */
                obj_query_index_finish_and_output_leaf(&state, out);
            } else {
                obj_assert(state.cur_index == -1);
            }
            /* reset state */
            obj_query_index_scan_build_state_reset(&state, state.index_tag);
            obj_index_catalog_entry_t *index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(indexes, state.cur_index);
            /* create new */
            state.current_scan = obj_query_index_make_leaf_node(index_entry, state.index_tag->pos, child);
            obj_assert(state.current_scan);
            /* obj_query_index_handle_filter(&state); */
        }
        state.cur_child++;
    }
    if (state.current_scan != NULL) {
        obj_query_index_finish_and_output_leaf(&state, out);
    }
    return true;
}

/* process sub AND/ORs */
static obj_bool_t obj_query_index_process_index_scans_subnode(obj_query_index_scan_build_state_t *state, obj_array_t *out) {
    obj_expr_base_expr_t *root = state->root;
    obj_expr_base_expr_t *child = root->methods->get_child(root, state->cur_child);
    state->cur_child++;
    obj_query_plan_tree_base_node_t *child_node = obj_query_index_build_indexed_data_access(child, state->indexes);
    if (child_node == NULL) {
        return false;
    }
    obj_array_push_back(out, &child_node);
    return true;
}

static void obj_query_index_scan_build_state_init(obj_query_index_scan_build_state_t *state, obj_expr_base_expr_t *root, obj_array_t *indexes) {
    obj_assert(state);
    state->root = root;
    state->indexes = indexes;
    state->current_scan = NULL;
    state->cur_child = 0;
    state->cur_index = -1;
    state->index_tag = NULL;
    /* state->cur_or = NULL; */
}

static void obj_query_index_scan_build_state_reset(obj_query_index_scan_build_state_t *state, obj_expr_index_tag_t *tag) {
    state->current_scan = NULL;
    state->cur_index = tag->index;
    /*
    if (state->root->type == OBJ_EXPR_TYPE_OR) {
        state->cur_or
    }
    */
}


/* generate execution tree */


