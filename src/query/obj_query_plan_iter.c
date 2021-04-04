#include "obj_core.h"

/* expr->memo_id map methods */
static obj_uint64_t obj_expr_id_map_hash_func(void *key);
static int obj_expr_id_map_key_compare(void *key1, void *key2);
static void *obj_expr_id_map_key_get(void *data);
static void *obj_expr_id_map_value_get(void *data);
static void obj_expr_id_map_key_set(void *data, void *key);
static void obj_expr_id_map_value_set(void *data, void *value);

/* index_id->expr_list map methods */
static obj_uint64_t obj_index_expr_list_map_hash_func(void *key);
static int obj_index_expr_list_map_key_compare(void *key1, void *key2);
static void obj_index_expr_list_map_value_free(void *data);
static void *obj_index_expr_list_map_key_get(void *data);
static void *obj_index_expr_list_map_value_get(void *data);
static void obj_index_expr_list_map_key_set(void *data, void *key);
static void obj_index_expr_list_map_value_set(void *data, void *value);

static void obj_query_plan_iter_node_destroy(void *ptr);
static void obj_query_plan_iter_and_node_init(obj_query_plan_iter_base_node_t *node);
static void obj_query_plan_iter_or_node_init(obj_query_plan_iter_base_node_t *node);
static void obj_query_plan_iter_and_iter_state_init(obj_query_plan_iter_and_iter_state_t *state);
static void obj_query_plan_iter_and_iter_state_destroy_static(void *ptr);
static void obj_query_plan_iter_index_assignment_init(obj_query_plan_iter_index_assignment_t *index_assign);
static void obj_query_plan_iter_index_assignment_destroy_static(void *ptr);
static void obj_query_plan_iter_allocate_node(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr, obj_query_plan_iter_base_node_t **node, int *id);
static void obj_query_plan_iter_tag_for_sort(obj_expr_base_expr_t *expr);
static obj_bool_t obj_query_plan_iter_prep_memo(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr);
static obj_bool_t obj_query_plan_iter_prep_sub_nodes(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr, obj_array_t *subnodes_out);
static void obj_query_plan_iter_tag_memo(obj_query_plan_iter_t *pi, int id);
static int obj_query_plan_iter_memo_id_for_expr(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr);
static void obj_query_plan_iter_assign_predicate(obj_query_plan_iter_index_assignment_t *index_assignment, obj_expr_base_expr_t *pred, int position);
static void obj_query_plan_iter_one_index(obj_query_plan_iter_t *pi, obj_prealloc_map_t *index_to_first, obj_prealloc_map_t *index_to_not_first, obj_array_t *subnodes, obj_query_plan_iter_and_node_t *and_node);
static void obj_query_plan_iter_get_indexed_preds(obj_expr_base_expr_t *expr, obj_array_t *indexed_preds);
static int obj_query_plan_iter_get_position(obj_index_catalog_entry_t *index_entry, obj_expr_base_expr_t *pred);
static obj_bool_t obj_query_plan_iter_next_memo(obj_query_plan_iter_t *pi, int id);


/* expr->memo_id */

static obj_prealloc_map_methods_t expr_id_map_methods = {
    obj_expr_id_map_hash_func,
    obj_expr_id_map_key_compare,
    NULL,
    NULL,
    obj_expr_id_map_key_get,
    obj_expr_id_map_value_get,
    obj_expr_id_map_key_set,
    obj_expr_id_map_value_set,
    NULL,
    NULL
};
 
static obj_uint64_t obj_expr_id_map_hash_func(void *key) {
    return obj_prealloc_map_hash_function(key, sizeof(obj_expr_base_expr_t *));
}

static int obj_expr_id_map_key_compare(void *key1, void *key2) {
    obj_expr_base_expr_t *expr1 = *(obj_expr_base_expr_t **)key1;
    obj_expr_base_expr_t *expr2 = *(obj_expr_base_expr_t **)key2;
    if (expr1 - expr2 < 0) {
        return -1;
    } else if (expr1 - expr2 > 0) {
        return 1;
    }
    return 0;
}

static void *obj_expr_id_map_key_get(void *data) {
    obj_expr_id_pair_t *pair = (obj_expr_id_pair_t *)data;
    return &pair->expr;
}

static void *obj_expr_id_map_value_get(void *data) {
    obj_expr_id_pair_t *pair = (obj_expr_id_pair_t *)data;
    return &pair->id;
}

static void obj_expr_id_map_key_set(void *data, void *key) {
    obj_expr_id_pair_t *pair = (obj_expr_id_pair_t *)data;
    obj_memcpy(&pair->expr, key, sizeof(obj_expr_base_expr_t *));
}

static void obj_expr_id_map_value_set(void *data, void *value) {
    obj_expr_id_pair_t *pair = (obj_expr_id_pair_t *)data;
    obj_memcpy(&pair->id, value, sizeof(int));
}

/* index_id->expr_list map */

static obj_prealloc_map_methods_t index_expr_list_map_methods = {
    obj_index_expr_list_map_hash_func,
    obj_index_expr_list_map_key_compare,
    NULL,
    obj_index_expr_list_map_value_free,
    obj_index_expr_list_map_key_get,
    obj_index_expr_list_map_value_get,
    obj_index_expr_list_map_key_set,
    obj_index_expr_list_map_value_set,
    NULL,
    NULL
};

static obj_uint64_t obj_index_expr_list_map_hash_func(void *key) {
    return obj_prealloc_map_hash_function(key, sizeof(int));
}

static int obj_index_expr_list_map_key_compare(void *key1, void *key2) {
    int index_id1 = *(int *)key1;
    int index_id2 = *(int *)key2;
    return index_id1 - index_id2;
}

static void obj_index_expr_list_map_value_free(void *data) {
    obj_index_expr_list_pair_t *pair = (obj_index_expr_list_pair_t *)data;
    obj_array_destroy_static(&pair->expr_list);
}

static void *obj_index_expr_list_map_key_get(void *data) {
    obj_index_expr_list_pair_t *pair = (obj_index_expr_list_pair_t *)data;
    return &pair->index_id;
}

static void *obj_index_expr_list_map_value_get(void *data) {
    obj_index_expr_list_pair_t *pair = (obj_index_expr_list_pair_t *)data;
    return &pair->expr_list;
}

static void obj_index_expr_list_map_key_set(void *data, void *key) {
    obj_index_expr_list_pair_t *pair = (obj_index_expr_list_pair_t *)data;
    obj_memcpy(&pair->index_id, key, sizeof(int));
}

static void obj_index_expr_list_map_value_set(void *data, void *value) {
    obj_index_expr_list_pair_t *pair = (obj_index_expr_list_pair_t *)data;
    obj_memcpy(&pair->expr_list, value, sizeof(obj_array_t));
}




/* init query plan iter */
void obj_query_plan_iter_init(obj_query_plan_iter_t *pi, obj_array_t *indexes, obj_expr_base_expr_t *root) {
    pi->indexes = indexes;
    pi->root = root;
    /* expr->memo_id */
    obj_prealloc_map_init(&pi->expr_to_id, &expr_id_map_methods, sizeof(obj_expr_id_pair_t));
    /* memo_id->iter_node */
    obj_array_init(&pi->id_to_node, sizeof(obj_query_plan_iter_base_node_t *));
    /* set free */
    obj_array_set_free(&pi->id_to_node, obj_query_plan_iter_node_destroy);
    pi->done = !obj_query_plan_iter_prep_memo(pi, pi->root);
    /* free all tags, will replace relevant_tag with index_tag */
    obj_expr_reset_tag(root);
}

/* destroy query plan iter */
void obj_query_plan_iter_destroy_static(obj_query_plan_iter_t *pi) {
    /* expr->memo_id map */
    obj_prealloc_map_destroy_static(&pi->expr_to_id);
    /* memo_id->iter_node array */
    obj_array_destroy_static(&pi->id_to_node);
}

static void obj_query_plan_iter_node_destroy(void *ptr) {
    obj_query_plan_iter_base_node_t *base_node = *(obj_query_plan_iter_base_node_t **)ptr;
    if (base_node->node_type == OBJ_QUERY_PLAN_TREE_NODE_TYPE_AND) {
        obj_array_destroy_static(&base_node->and_node.choices);
        obj_free(base_node);
    } else {
        obj_array_destroy_static(&base_node->or_node.subnodes);
        obj_free(base_node);
    }
}

static void obj_query_plan_iter_and_node_init(obj_query_plan_iter_base_node_t *node) {
    node->node_type = OBJ_QUERY_PLAN_ITER_NODE_TYPE_AND;
    obj_query_plan_iter_and_node_t *and_node = &node->and_node;
    obj_array_init(&and_node->choices, sizeof(obj_query_plan_iter_and_iter_state_t));
    obj_array_set_free(&and_node->choices, obj_query_plan_iter_and_iter_state_destroy_static);
}

static void obj_query_plan_iter_or_node_init(obj_query_plan_iter_base_node_t *node) {
    node->node_type = OBJ_QUERY_PLAN_ITER_NODE_TYPE_OR;
    obj_query_plan_iter_or_node_t *or_node = &node->or_node;
    obj_array_init(&or_node->subnodes, sizeof(int));
}

static void obj_query_plan_iter_and_iter_state_init(obj_query_plan_iter_and_iter_state_t *state) {
    obj_array_init(&state->subnodes, sizeof(int));
    obj_array_init(&state->assignments, sizeof(obj_query_plan_iter_index_assignment_t));
    obj_array_set_free(&state->assignments, obj_query_plan_iter_index_assignment_destroy_static);
}

static void obj_query_plan_iter_and_iter_state_destroy_static(void *ptr) {
    obj_query_plan_iter_and_iter_state_t *state = (obj_query_plan_iter_and_iter_state_t *)ptr;
    obj_array_destroy_static(&state->subnodes);
    obj_array_destroy_static(&state->assignments);
}

static void obj_query_plan_iter_index_assignment_init(obj_query_plan_iter_index_assignment_t *index_assign) {
    obj_array_init(&index_assign->positions, sizeof(int));
    obj_array_init(&index_assign->preds, sizeof(obj_expr_base_expr_t *));
}

static void obj_query_plan_iter_index_assignment_destroy_static(void *ptr) {
    obj_query_plan_iter_index_assignment_t *index_assign = (obj_query_plan_iter_index_assignment_t *)ptr;
    obj_array_destroy_static(&index_assign->positions);
    obj_array_destroy_static(&index_assign->preds);
}

static void obj_query_plan_iter_allocate_node(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr, obj_query_plan_iter_base_node_t **node, int *id) {
    int new_id = pi->id_to_node.size;
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&pi->expr_to_id, &expr);
    obj_assert(entry == NULL);
    /* add */
    obj_prealloc_map_add(&pi->expr_to_id, &expr, &new_id);
    obj_query_plan_iter_base_node_t *new_node = obj_alloc(sizeof(obj_query_plan_iter_base_node_t));
    obj_array_push_back(&pi->id_to_node, &new_node);
    *node = new_node;
    *id = new_id;
}

/* 
 * Entrance: get next tagged tree 
 */
obj_expr_base_expr_t *obj_query_plan_iter_get_next(obj_query_plan_iter_t *pi) {
    if (pi->done) {
        return NULL;
    }
    /* clear tags */
    obj_expr_reset_tag(pi->root);
    int memo_id = obj_query_plan_iter_memo_id_for_expr(pi, pi->root);
    obj_query_plan_iter_tag_memo(pi, memo_id);
    /* move to next */
    pi->done = obj_query_plan_iter_next_memo(pi, memo_id);
    obj_query_plan_iter_tag_for_sort(pi->root);
    
    return pi->root;
}


static void obj_query_plan_iter_tag_for_sort(obj_expr_base_expr_t *expr) {
    obj_expr_index_tag_t *my_index_tag = NULL;
    obj_expr_index_tag_t *child_tag = NULL;
    int i;
    obj_expr_base_expr_t *child = NULL;
    if (!obj_query_index_expr_can_use_index(expr)) {
        for (i = 0; i < expr->methods->num_child(expr); i++) {
            child = expr->methods->get_child(expr, i);
            obj_query_plan_iter_tag_for_sort(child);
            child_tag = (obj_expr_index_tag_t *)child->tag;
            if (child_tag != NULL) {
                if (!my_index_tag || my_index_tag->index > child_tag->index) {
                    my_index_tag = child_tag;
                }
            }
        }
        if (my_index_tag) {
            expr->tag = (obj_expr_tag_t *)obj_expr_index_tag_compound_create(my_index_tag->index, my_index_tag->pos);
            obj_assert(expr->tag);
        }
    }
}



static obj_bool_t obj_query_plan_iter_prep_memo(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr) {
    if (expr->type == OBJ_EXPR_TYPE_OR) {
        /* for an OR to be indexed, all its children must be indexed */
        int i;
        obj_expr_base_expr_t *child = NULL;
        for (i = 0; i < expr->methods->num_child(expr); i++) {
            child = expr->methods->get_child(expr, i);
            if (!obj_query_plan_iter_prep_memo(pi, child)) {
                return false;
            }
        }
        /* all children are indexed */
        int new_id;
        obj_query_plan_iter_base_node_t *new_node = NULL;
        obj_query_plan_iter_allocate_node(pi, expr, &new_node, &new_id);
        obj_query_plan_iter_or_node_init(new_node);
        for (i = 0; i < expr->methods->num_child(expr); i++) {
            child = expr->methods->get_child(expr, i);
            int child_id = obj_query_plan_iter_memo_id_for_expr(pi, child);
            /* add */
            obj_array_push_back(&new_node->or_node.subnodes, &child_id);
        }
        return true;
    } else {
        /* AND or compare expression */
        obj_assert(expr->type == OBJ_EXPR_TYPE_AND || (expr->type >= OBJ_EXPR_TYPE_EQ && expr->type <= OBJ_EXPR_TYPE_GTE));
        /* [index_id->[expr*]] */
        obj_prealloc_map_t index_to_first;
        obj_prealloc_map_t index_to_not_first;
        obj_prealloc_map_init(&index_to_first, &index_expr_list_map_methods, sizeof(obj_index_expr_list_pair_t));
        obj_prealloc_map_init(&index_to_not_first, &index_expr_list_map_methods, sizeof(obj_index_expr_list_pair_t));
        /* [id] */
        obj_array_t subnodes;
        obj_array_init(&subnodes, sizeof(int));
        /* [expr*] */
        obj_array_t indexed_preds;
        obj_array_init(&indexed_preds, sizeof(obj_expr_base_expr_t *));
        obj_query_plan_iter_get_indexed_preds(expr, &indexed_preds);
        if (!obj_query_plan_iter_prep_sub_nodes(pi, expr, &subnodes)) {
            return false;
        }
        int i, j;
        obj_expr_base_expr_t *child = NULL;
        obj_expr_relevant_tag_t *rt = NULL;
        obj_prealloc_map_entry_t *entry = NULL;
        int index_id;
        obj_bool_t create;
        for (i = 0; i < indexed_preds.size; i++) {
            child = (obj_expr_base_expr_t *)obj_array_get_index_value(&indexed_preds, i, uintptr_t);
            rt = (obj_expr_relevant_tag_t *)child->tag;
            for (j = 0; j < rt->first.size; j++) {
                index_id = obj_array_get_index_value(&rt->first, j, int);
                /* add to index_to_first */
                entry = obj_prealloc_map_find_add_key_if_not_exists(&index_to_first, &index_id, &create);
                obj_array_t *expr_arr = obj_prealloc_map_get_value(&index_to_first, entry);
                if (create) {
                    obj_array_init(expr_arr, sizeof(obj_expr_base_expr_t *));
                }
                obj_array_push_back(expr_arr, &child);
                
            }
            for (j = 0; j < rt->not_first.size; j++) {
                index_id = obj_array_get_index_value(&rt->not_first, j, int);
                /* add to index_to_not_first */
                entry = obj_prealloc_map_find_add_key_if_not_exists(&index_to_not_first, &index_id, &create);
                
                obj_array_t *expr_arr = obj_prealloc_map_get_value(&index_to_first, entry);
                if (create) {
                    obj_array_init(expr_arr, sizeof(obj_expr_base_expr_t *));
                }
                obj_array_push_back(expr_arr, &child);
                
            }
        }
        /* no way to use index */
        if (index_to_first.size == 0 && index_to_not_first.size == 0 && subnodes.size == 0) {
            return false;
        }
        obj_query_plan_iter_base_node_t *new_node = NULL;
        int new_id;
        obj_query_plan_iter_allocate_node(pi, expr, &new_node, &new_id);
        obj_query_plan_iter_and_node_init(new_node);
        obj_query_plan_iter_one_index(pi, &index_to_first, &index_to_not_first, &subnodes, &new_node->and_node);
        /* clean */
        obj_prealloc_map_destroy_static(&index_to_first);
        obj_prealloc_map_destroy_static(&index_to_not_first);
        obj_array_destroy_static(&subnodes);
        obj_array_destroy_static(&indexed_preds);
        /* can use index */
        return new_node->and_node.choices.size != 0;
    }
    return false;
}

/* ORs */
static obj_bool_t obj_query_plan_iter_prep_sub_nodes(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr, obj_array_t *subnodes_out) {
    int i;
    obj_expr_base_expr_t *child = NULL;
    for (i = 0; i < expr->methods->num_child(expr); i++) {
        child = expr->methods->get_child(expr, i);
        if (child->type == OBJ_EXPR_TYPE_OR) {
            if (obj_query_plan_iter_prep_memo(pi, child)) {
                int child_id = obj_query_plan_iter_memo_id_for_expr(pi, child);
                /* output subnode */
                obj_array_push_back(subnodes_out, &child_id);
            }
        } else if (child->type == OBJ_EXPR_TYPE_AND) {
            obj_query_plan_iter_prep_sub_nodes(pi, child, subnodes_out);
        }
    }
    return true;
}

/* tag memo tree */
static void obj_query_plan_iter_tag_memo(obj_query_plan_iter_t *pi, int id) {
    obj_query_plan_iter_base_node_t *node = NULL;
    node = (obj_query_plan_iter_base_node_t *)obj_array_get_index_value(&pi->id_to_node, id, uintptr_t);
    obj_assert(node != NULL);
    if (node->node_type == OBJ_QUERY_PLAN_ITER_NODE_TYPE_OR) {
        obj_query_plan_iter_or_node_t *or_node = &node->or_node;
        int i, sub_id;
        for (i = 0; i < or_node->subnodes.size; i++) {
            sub_id = obj_array_get_index_value(&or_node->subnodes, i, int);
            obj_query_plan_iter_tag_memo(pi, sub_id);
        }
    } else {
        obj_query_plan_iter_and_node_t *and_node = &node->and_node;
        obj_assert(and_node->counter < and_node->choices.size);
        obj_query_plan_iter_and_iter_state_t *state = (obj_query_plan_iter_and_iter_state_t *)obj_array_get_index(&and_node->choices, and_node->counter);
        int i, j, sub_id;
        for (i = 0; i < state->subnodes.size; i++) {
            sub_id = obj_array_get_index_value(&state->subnodes, i, int);
            obj_query_plan_iter_tag_memo(pi, sub_id);
        }
        obj_query_plan_iter_index_assignment_t *assign = NULL;
        obj_expr_base_expr_t *pred = NULL;
        int position;
        for (i = 0; i < state->assignments.size; i++) {
            assign = obj_array_get_index(&state->assignments, i);
            /* tag tree */
            for (j = 0; j < assign->preds.size; j++) {
                pred = (obj_expr_base_expr_t *)obj_array_get_index_value(&assign->preds, j, uintptr_t);
                position = obj_array_get_index_value(&assign->positions, j, int);
                obj_assert(pred->tag == NULL);
                /* set tag */
                pred->tag = (obj_expr_tag_t *)obj_expr_index_tag_compound_create(assign->index, position);
                obj_assert(pred->tag);
            }
        }
    }
}

static int obj_query_plan_iter_memo_id_for_expr(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr) {
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&pi->expr_to_id, &expr);
    obj_assert(entry != NULL);
    return *(int *)obj_prealloc_map_get_value(&pi->expr_to_id, entry);
}

/* assign predicate */
static void obj_query_plan_iter_assign_predicate(obj_query_plan_iter_index_assignment_t *index_assignment, obj_expr_base_expr_t *pred, int position) {
    /* add index assignments */
    obj_array_push_back(&index_assignment->preds, &pred);
    obj_array_push_back(&index_assignment->positions, &position);
}


/* 
 * generate index assignment 
 */
static void obj_query_plan_iter_one_index(obj_query_plan_iter_t *pi, obj_prealloc_map_t *index_to_first, obj_prealloc_map_t *index_to_not_first, obj_array_t *subnodes, obj_query_plan_iter_and_node_t *and_node) {
    int i, j;
    /* add subnodes */
    int sub_id;
    for (i = 0; i < subnodes->size; i++) {
        obj_query_plan_iter_and_iter_state_t state;
        /* init state */
        obj_query_plan_iter_and_iter_state_init(&state);
        sub_id = obj_array_get_index_value(subnodes, i, int);
        obj_array_push_back(&state.subnodes, &sub_id);
        obj_array_push_back(&and_node->choices, &state);
    }
    /* for each first, assign predicate */
    obj_index_catalog_entry_t *index_entry = NULL;
    obj_prealloc_map_entry_t *first_entry = NULL;
    obj_prealloc_map_entry_t *not_first_entry = NULL;
    obj_array_t *first_preds = NULL;
    obj_array_t *not_first_preds = NULL;
    obj_expr_base_expr_t *pred = NULL;
    int index_id;
    
    for (i = 0; i < index_to_first->bucket_size; i++) {
        first_entry = index_to_first->bucket[i];
        while (first_entry != NULL) {
            index_id = *(int *)obj_prealloc_map_get_key(index_to_first, first_entry);
            first_preds = (obj_array_t *)obj_prealloc_map_get_value(index_to_first, first_entry);
            obj_assert(first_preds != NULL);
            index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(pi->indexes, index_id);
            obj_query_plan_iter_index_assignment_t index_assign;
            obj_query_plan_iter_index_assignment_init(&index_assign);
            index_assign.index = index_id;
            /* assign first */
            for (j = 0; j < first_preds->size; j++) {
                pred = (obj_expr_base_expr_t *)obj_array_get_index_value(first_preds, j, uintptr_t);
                obj_query_plan_iter_assign_predicate(&index_assign, pred, 0);
            }
            /* assign second */
            not_first_entry = obj_prealloc_map_find(index_to_not_first, &index_id);
            if (not_first_entry != NULL) {
                not_first_preds = (obj_array_t *)obj_prealloc_map_get_value(index_to_not_first, not_first_entry);
                for (j = 0; j < not_first_preds->size; j++) {
                    pred = (obj_expr_base_expr_t *)obj_array_get_index_value(not_first_preds, j, uintptr_t);
                    obj_query_plan_iter_assign_predicate(&index_assign, pred, obj_query_plan_iter_get_position(index_entry, pred));
                }
            }
            /* output assignment */
            obj_assert(index_assign.preds.size > 0);
            obj_query_plan_iter_and_iter_state_t state;
            /* init state */
            obj_query_plan_iter_and_iter_state_init(&state);
            obj_array_push_back(&state.assignments, &index_assign);
            obj_array_push_back(&and_node->choices, &state);
            first_entry = first_entry->next;
        }
    }

}

/* get child predicates which can use index. called by AND and leaf expr node */
static void obj_query_plan_iter_get_indexed_preds(obj_expr_base_expr_t *expr, obj_array_t *indexed_preds) {
    if (obj_query_index_expr_can_use_index(expr)) {
        obj_expr_relevant_tag_t *rt = (obj_expr_relevant_tag_t *)expr->tag;
        /* add */
        obj_array_push_back(indexed_preds, &expr);
    } else if (expr->type == OBJ_EXPR_TYPE_AND) {
        int i;
        obj_expr_base_expr_t *child = NULL;
        for (i = 0; i < expr->methods->num_child(expr); i++) {
            child = expr->methods->get_child(expr, i);
            obj_query_plan_iter_get_indexed_preds(child, indexed_preds);
        }
    }
}

/* find pred's path position in index */
static int obj_query_plan_iter_get_position(obj_index_catalog_entry_t *index_entry, obj_expr_base_expr_t *pred) {
    obj_assert(pred->tag != NULL);
    obj_expr_relevant_tag_t *rt = (obj_expr_relevant_tag_t *)pred->tag;
    int position = 0;
    obj_bson_iter_t iter;
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_init(&iter, index_entry->key_pattern);
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        if (obj_strcmp(key, rt->path) == 0) {
            return position;
        }
        position++;
    }
    obj_assert(0);
}

/* move to next state */
static obj_bool_t obj_query_plan_iter_next_memo(obj_query_plan_iter_t *pi, int id) {
    obj_query_plan_iter_base_node_t *node = NULL;
    node = (obj_query_plan_iter_base_node_t *)obj_array_get_index_value(&pi->id_to_node, id, uintptr_t);
    obj_assert(node != NULL);
    if (node->node_type == OBJ_QUERY_PLAN_ITER_NODE_TYPE_OR) {
        obj_query_plan_iter_or_node_t *or_node = &node->or_node;
        /* or_node->counter++; */
        int i;
        for (i = 0; i < or_node->subnodes.size; i++) {
            int sub_id = obj_array_get_index_value(&or_node->subnodes, i, int);
            if (!obj_query_plan_iter_next_memo(pi, sub_id)) {
                return false;
            }
        }
        /* reach end */
        return true;
    } else {
        /* AND */
        obj_query_plan_iter_and_node_t *and_node = &node->and_node;
        int i;
        obj_query_plan_iter_and_iter_state_t *state = (obj_query_plan_iter_and_iter_state_t *)obj_array_get_index(&and_node->choices, i);
        for (i = 0; i < state->subnodes.size; i++) {
            int sub_id = obj_array_get_index_value(&state->subnodes, i, int);
            if (!obj_query_plan_iter_next_memo(pi, sub_id)) {
                return false;
            }
        }
        and_node->counter++;
        if (and_node->counter < and_node->choices.size) {
            return false;
        }
        and_node->counter = 0;
        return true;
    }
}

