#include "obj_core.h"


 



/* init query plan iter */
void obj_query_plan_iter_init(obj_query_plan_iter_t *pi, obj_array_t *indexes, obj_expr_base_expr_t *root) {
    pi->indexes = indexes;
    pi->root = root;
    /*  */

    obj_array_init(&pi->id_to_node, sizeof(obj_query_plan_iter_base_node_t *));
    /* set free */
    obj_array_set_free(&pi->id_to_node, obj_query_plan_iter_node_destroy);
    pi->done = obj_query_plan_iter_prep_memo(pi, pi->root);
}

/* destroy query plan iter */
void obj_query_plan_iter_destroy(obj_query_plan_iter_t *pi) {
    obj_array_destroy_static(&pi->id_to_node);
}

static obj_query_plan_iter_node_destroy(obj_query_plan_iter_base_node_t *base_node) {
    if (base_node->node_type == OBJ_QUERY_PLAN_TREE_NODE_TYPE_AND) {
        obj_array_destroy_static(&base_node->and_node.choices);
        obj_free(base_node);
    } else {
        obj_array_destroy_static(&base_node->or_node.subnodes);
        obj_free(base_node);
    }
}

static void obj_query_plan_iter_and_node_init(obj_query_plan_iter_and_node_t *and_node) {
    obj_array_init(&and_node->choices, sizeof(obj_query_plan_iter_and_iter_state_t));
    obj_array_set_free(&and_node->choices, obj_query_plan_iter_and_iter_state_destroy_static);
}

static void obj_query_plan_iter_or_node_init(obj_query_plan_iter_or_node_t *or_node) {
    obj_array_init(&or_node->subnodes, sizeof(int));
}

static void obj_query_plan_iter_and_iter_state_init(obj_query_plan_iter_and_iter_state_t *state) {
    obj_array_init(&state->subnodes, sizeof(int));
    obj_array_init(&state->assignments, sizeof(obj_query_plan_iter_index_assignment_t));
    obj_array_set_free(&state->assignments, obj_query_plan_iter_index_assignment_destroy_static);
}

static void obj_query_plan_iter_and_iter_state_destroy_static(obj_query_plan_iter_and_iter_state_t *state) {
    obj_array_destroy_static(&state->subnodes);
    obj_array_destroy_static(&state->assignments);
}

static void obj_query_plan_iter_index_assignment_init(obj_query_plan_iter_index_assignment_t *index_assign) {
    obj_array_init(&index_assign->positions, sizeof(int));
    obj_array_init(&index_assign->preds, sizeof(obj_expr_base_expr_t *));
}

static void obj_query_plan_iter_index_assignment_destroy_static(obj_query_plan_iter_index_assignment_t *index_assign) {
    obj_array_destroy_static(&index_assign->positions);
    obj_array_destroy_static(&index_assign->preds);
}

void obj_query_plan_iter_allocate_node(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr, obj_query_plan_iter_base_node_t **node, int *id) {
    int new_id = pi->id_to_node.size;
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&pi->expr_to_id, &expr);
    obj_assert(entry == NULL);
    /* add */
    obj_prealloc_map_add(&pi->expr_to_id, &expr, &new_id);
    obj_array_reserve(&pi->id_to_node, new_id + 1);
    obj_query_plan_iter_base_node_t *new_node = obj_alloc(sizeof(obj_query_plan_iter_base_node_t));
    obj_array_set_index(&pi->id_to_node, new_id, &new_node);
    *id = new_id;
}

/* 
 * Entrance: get next tagged tree 
 */
obj_expr_base_expr_t *obj_query_plan_iter_get_next(obj_query_plan_iter_t *pi) {
    if (pi->done) {
        return NULL;
    }
    obj_query_plan_iter_tag_memo(pi, obj_query_plan_iter_memo_id_for_expr(pi, pi->root));
    obj_query_plan_iter_tag_for_sort(pi->root);
    /* reset tags */
    obj_expr_reset_tag(pi->root);
    pi->done = obj_query_plan_iter_next_memo(pi, obj_query_plan_iter_memo_id_for_expr(pi, pi->root));
    return pi->root;
}


void obj_query_plan_iter_tag_for_sort(obj_expr_base_expr_t *expr) {
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
            obj_expr_set_tag(expr, obj_expr_index_tag_compound_create(my_index_tag->index, my_index_tag->pos));
        }
    }
}



obj_bool_t obj_query_plan_iter_prep_memo(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr) {
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
        new_node->or_node.counter = 0;
        obj_query_plan_iter_allocate_node(pi, expr, &new_node, &new_id);
        if (new_node == NULL) {
            return false;
        }
        new_node->node_type = OBJ_QUERY_PLAN_ITER_NODE_TYPE_OR;
        obj_array_init(&new_node->or_node.subnodes, sizeof(int));
        for (i = 0; i < expr->methods->num_child(expr); i++) {
            child = expr->methods->get_child(expr, i);
            if (!obj_array_init(&new_node->or_node.subnodes, sizeof(int))) {
                return false;
            }
            int child_id = obj_query_plan_iter_memo_id_for_expr(pi, expr);
            /* add */
            obj_array_push_back(&new_node->or_node.subnodes, &child_id);
        }
        
    } else {
        /* AND or compare expression */
        obj_assert(expr->type == OBJ_EXPR_TYPE_AND || (expr->type >= OBJ_EXPR_TYPE_EQ && expr->type <= OBJ_EXPR_TYPE_GTE));
        obj_prealloc_map_t index_to_first;
        obj_prealloc_map_t index_to_not_first;
        obj_prealloc_map_init(&index_to_first, , sizeof());
        obj_prealloc_map_init();
        /* [id] */
        obj_array_t subnodes;
        obj_array_init();
        /* [expr*] */
        obj_array_t indexed_preds;
        obj_array_init();
        obj_query_plan_iter_get_indexed_preds(expr, &indexed_preds);
        if (!obj_query_plan_iter_prep_sub_nodes(pi, expr, &subnodes)) {
            return false;
        }
        int i, j;
        obj_expr_base_expr_t *child = NULL;
        obj_expr_relevant_tag_t *rt = NULL;
        for (i = 0; i < indexed_preds.size; i++) {
            child = (obj_expr_base_expr_t *)obj_array_get_index_value(&indexed_preds, i, uintptr_t);
            rt = (obj_expr_relevant_tag_t *)child->tag;
            for (j = 0; j < rt->first.size; j++) {
                /* add to index_to_first */

            }
            for (j = 0; j < rt->not_first.size; j++) {
                /* add to index_to_not_first */

            }
        }
        /* no way to use index */
        if (index_to_first.size == 0 && index_to_not_first.size == 0 && subnodes.size == 0) {
            return false;
        }
        obj_query_plan_iter_base_node_t *new_node = NULL;
        int new_id;
        obj_query_plan_iter_allocate_node(pi, expr, &new_node, &new_id);
        new_node->node_type = OBJ_QUERY_PLAN_ITER_NODE_TYPE_AND;
        if (new_node == NULL) {
            return false;
        }
        obj_array_init(&new_node->and_node.choices, sizeof(obj_query_plan_iter_and_iter_state_t));
        obj_query_plan_iter_one_index(pi, &index_to_first, &index_to_not_first, &subnodes, &new_node->and_node);
        /* can use index */
        return new_node->and_node.choices.size != 0;
    }
    return false;
}

/* ORs */
obj_bool_t obj_query_plan_iter_prep_sub_nodes(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr, obj_array_t *subnodes_out) {
    int i;
    obj_expr_base_expr_t *child = NULL;
    for (i = 0; i < expr->methods->num_child(expr); i++) {
        child = expr->methods->get_child(expr, i);
        if (child->type == OBJ_EXPR_TYPE_OR) {
            if (obj_query_plan_iter_prep_memo(child)) {
                int child_id = obj_query_plan_iter_memo_id_for_expr(pi, child);
                /* output subnode */
                obj_array_push_back(subnodes_out, &child_id);
            }
        } else if (child->type == OBJ_EXPR_TYPE_AND) {
            obj_query_plan_iter_prep_sub_nodes(child, subnodes_out);
        }
    }
    return true;
}


void obj_query_plan_iter_tag_memo(obj_query_plan_iter_t *pi, int id) {
    
}

int obj_query_plan_iter_memo_id_for_expr(obj_query_plan_iter_t *pi, obj_expr_base_expr_t *expr) {
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&pi->expr_to_id, &expr);
    obj_assert(entry != NULL);
    return *(int **)obj_prealloc_map_get_value(&pi->expr_to_id, entry);
}

/* assign predicate */
void obj_query_plan_iter_assign_predicate(obj_query_plan_iter_index_assignment_t *index_assignment, obj_expr_base_expr_t *pred, int position) {
    /* add index assignments */
    obj_array_push_back(&index_assignment->preds, &pred);
    obj_array_push_back(&index_assignment->positions, &position);
}


/* 
 * generate index assignment 
 */
void obj_query_plan_iter_one_index(obj_query_plan_iter_t *pi, obj_prealloc_map_t *index_to_first, obj_prealloc_map_t *index_to_not_first, obj_array_t *subnodes, obj_query_plan_iter_and_node_t *and_node) {
    int i, j;
    /* add subnodes */
    for (i = 0; i < subnodes->size; i++) {
        obj_query_plan_iter_and_iter_state_t *state;
        obj_query_plan_iter_and_iter_state_t *sub_state = (obj_query_plan_iter_and_iter_state_t *)obj_array_get_index(subnodes, i);
        obj_array_push_back(&state->subnodes, sub_state);
        
    }
    /* for each first, assign predicate */
    obj_query_index_entry_t *index_entry = NULL;
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
            index_entry = (obj_query_index_entry_t *)obj_array_get_index(pi->indexes, index_id);
            obj_query_plan_iter_index_assignment_t index_assign;
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
            obj_array_push_back(&state.assignments, &index_assign);
            obj_array_push_back(&and_node->choices, &state);
            first_entry = first_entry->next;
        }
    }

}

/* get child predicates which can use index */
void obj_query_plan_iter_get_indexed_preds(obj_expr_base_expr_t *expr, obj_array_t *indexed_preds) {
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

int obj_query_plan_iter_get_position(obj_query_index_entry_t *index_entry, obj_expr_base_expr_t *predicate) {
    obj_assert(predicate->tag != NULL);
    obj_expr_relevant_tag_t *rt = (obj_expr_relevant_tag_t *)predicate->tag;
    int position = 0;
    obj_bson_iter_t iter;
    const char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_init(&iter, index_entry->key_pattern);
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        if (obj_strcmp(key, rt->path.data) == 0) {
            return position;
        }
        position++;
    }
    obj_assert(0);
}

/* move to next state */
obj_bool_t obj_query_plan_iter_next_memo(obj_query_plan_iter_t *pi, int id) {
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

