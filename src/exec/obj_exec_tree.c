#include "obj_core.h"

/* execute query plan tree */

/* record set methods, for or node */

static obj_prealloc_map_methods_t record_set_methods = {
    obj_record_set_hash_func,
    obj_record_set_key_compare,
    NULL,
    NULL,
    obj_record_set_key_get,
    NULL,
    obj_record_set_key_set,
    NULL,
    NULL,
    NULL
};

static obj_uint64_t obj_record_set_hash_func(void *key) {
    obj_record_t *record = *(obj_record_t **)key;
    return obj_set_hash_function(record, sizeof(obj_record_t *));
}

static int obj_record_set_key_compare(void *key1, void *key2) {
    obj_record_t *record1 = *(obj_record_t **)key1;
    obj_record_t *record2 = *(obj_record_t **)key2;
    if (record1 - record2 < 0) {
        return -1;
    } else if (record1 - record2 > 0) {
        return 1;
    } else {
        return 0;
    }
}

static void *obj_record_set_key_get(void *data) {
    return data;
}

static void obj_record_set_key_set(void *data, void *key) {
    obj_memcpy(data, key, sizeof(obj_record_t *));
}





/* for and node */

static obj_prealloc_map_methods_t record_wsid_map_methods = {
    obj_record_wsid_map_hash_func,
    obj_record_wsid_map_key_compare,
    NULL,
    NULL,
    obj_record_wsid_map_key_get,
    obj_record_wsid_map_value_get,
    obj_record_wsid_map_key_set,
    obj_record_wsid_map_value_set,
    NULL,
    NULL
};

static obj_uint64_t obj_record_wsid_map_hash_func(void *key) {
    return obj_prealloc_map_hash_function(key, sizeof(obj_record_t *));
}

static int obj_record_wsid_map_key_compare(void *key1, void *key2) {
    obj_record_t *record1 = *(obj_record_t **)key1;
    obj_record_t *record2 = *(obj_record_t **)key2;
    if (record1 - record2 < 0) {
        return -1;
    } else if (record1 - record2 > 0) {
        return 1;
    } else {
        return 0;
    }
}

static void *obj_record_wsid_map_key_get(void *data) {
    obj_record_wsid_pair_t *pair = (obj_record_wsid_pair_t *)data;
    return &pair->record;
}

static void *obj_record_wsid_map_value_get(void *data) {
    obj_record_wsid_pair_t *pair = (obj_record_wsid_pair_t *)data;
    return &pair->wsid;
}

static void obj_record_wsid_map_key_set(void *data, void *key) {
    obj_record_wsid_pair_t *pair = (obj_record_wsid_pair_t *)data;
    obj_memcpy(&pair->record, key, sizeof(obj_record_t *));
}

static void obj_record_wsid_map_value_set(void *data, void *value) {
    obj_record_wsid_pair_t *pair = (obj_record_wsid_pair_t *)data;
    obj_memcpy(&pair->wsid, value, sizeof(obj_exec_working_set_id_t));
}


static void obj_exec_tree_init_base(obj_exec_tree_base_node_t *base, obj_exec_tree_node_methods_t *methods) {
    obj_array_init(&base->children, sizeof(obj_exec_tree_base_node_t *));
    base->methods = methods;
}


/* TODO: stats */

/* for exec tree node which has only one child */
static obj_exec_tree_base_node_t *obj_exec_tree_get_child(obj_exec_tree_base_node_t *node) {
    obj_array_t *children = &node->children;
    obj_assert(children->size == 1);
    obj_exec_tree_base_node_t *child = NULL;
    child = (obj_exec_tree_base_node_t *)obj_array_get_index_value(children, 0, uintptr_t);
    return child;
}


/* ********** and node ********** */

static obj_exec_tree_node_methods_t obj_exec_tree_and_node_methods = {
    obj_exec_tree_and_node_work,
    obj_exec_tree_and_node_get_type,
    obj_exec_tree_and_node_is_eof
};

obj_exec_tree_and_node_t *obj_exec_tree_and_node_create(obj_exec_working_set_t *ws) {
    obj_exec_tree_and_node_t *and_node = obj_alloc(sizeof(obj_exec_tree_and_node_t));
    obj_exec_tree_init_base((obj_exec_tree_base_node_t *)and_node, &obj_exec_tree_and_node_methods);
    and_node->ws = ws;
    and_node->current_child = 0;
    obj_prealloc_map_init(&and_node->data_map, &record_wsid_map_methods, sizeof(obj_record_wsid_pair_t));
    obj_set_init(&and_node->seen_map, &record_set_methods, sizeof(obj_record_t *));
    and_node->hashing_children = true;
    obj_array_init(&and_node->look_ahead_results, sizeof(obj_exec_working_set_id_t));
    return and_node;
}

/* and node work() */
obj_exec_tree_exec_state_t obj_exec_tree_and_node_work(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out) {
    obj_exec_tree_and_node_t *and_node = (obj_exec_tree_and_node_t *)node;
    if (node->methods->is_eof(node)) {
        return OBJ_EXEC_TREE_STATE_EOF;
    }
    if (and_node->look_ahead_results.size == 0) {
        obj_array_resize(&and_node->look_ahead_results, node->children.size);
        int i, j;
        int invalid_id = OBJ_EXEC_WORKING_SET_INVALID_ID;
        for (i = 0; i < node->children.size; i++) {
            obj_array_set_index(&and_node->look_ahead_results, i, &invalid_id);
        }
        obj_exec_tree_base_node_t *child = NULL;
        obj_exec_tree_exec_state_t child_state;
        obj_exec_working_set_id_t *child_id = NULL;
        for (i = 0; i < node->children.size; i++) {
            child = (obj_exec_tree_base_node_t *)obj_array_get_index_value(&node->children, i, uintptr_t);
            child_id = (obj_exec_working_set_id_t *)obj_array_get_index(&and_node->look_ahead_results, i);
            for (j = 0; j < 10; j++) {
                child_state = child->methods->work(child, child_id);
                if (child_state == OBJ_EXEC_TREE_STATE_EOF) {
                    /* a child reach eof. end */
                    and_node->hashing_children = false;
                    /* clear map */
                    obj_prealloc_map_delete_all(&and_node->data_map);
                    return OBJ_EXEC_TREE_STATE_EOF;
                } else if (child_state == OBJ_EXEC_TREE_STATE_ADVANCED) {
                    /* stop looking at this child */
                    break;
                } else if (child_state == OBJ_EXEC_TREE_STATE_INTERNAL_ERROR) {
                    /* TODO error */
                    return child_state;
                }
            }
        }
        /* leave works next */
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    }
    if (and_node->hashing_children) {
        if (and_node->current_child == 0) {
            return obj_exec_tree_and_node_read_first_child(and_node, out);
        } else if (and_node->current_child < node->children.size - 1) {
            return obj_exec_tree_and_node_hash_other_children(and_node, out);
        } else {
            /* would not hash last child */
            and_node->hashing_children = false;
        }
    }
    /* last child */
    obj_assert(and_node->data_map.size != 0);
    /* work on last child */
    obj_exec_tree_exec_state_t child_state = obj_exec_tree_and_node_work_child(and_node, node->children.size - 1, out);
    if (child_state != OBJ_EXEC_TREE_STATE_ADVANCED) {
        return child_state;
    }
    obj_exec_working_set_member_t *member = obj_exec_working_set_get(and_node->ws, *out);
    /* intersect */
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&and_node->data_map, &member->record);
    if (entry == NULL) {
        /* not found */
        obj_exec_working_set_free(and_node->ws, *out);
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    } else {
        obj_exec_working_set_id_t id = *(obj_exec_working_set_id_t *)obj_prealloc_map_get_value(&and_node->data_map, entry);
        obj_prealloc_map_delete_entry(&and_node->data_map, entry);
        obj_exec_working_set_free(and_node->ws, *out);
        *out = id;
        return OBJ_EXEC_TREE_STATE_ADVANCED;
    }

}

static obj_exec_tree_exec_state_t obj_exec_tree_and_node_read_first_child(obj_exec_tree_and_node_t *and_node, obj_exec_working_set_id_t *out) {
    obj_exec_working_set_id_t id = OBJ_EXEC_WORKING_SET_INVALID_ID;
    obj_exec_tree_exec_state_t child_state = obj_exec_tree_and_node_work_child(and_node, 0, &id);
    if (child_state == OBJ_EXEC_TREE_STATE_ADVANCED) {
        obj_exec_working_set_member_t *member = obj_exec_working_set_get(and_node->ws, id);
        /* add to datamap */
        obj_prealloc_map_add(&and_node->data_map, &member->record, &id);
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    } else if (child_state == OBJ_EXEC_TREE_STATE_EOF) {
        /* done reading child 0 */
        and_node->current_child = 1;
        /* first is empty, stop */
        if (and_node->data_map.size == 0) {
            and_node->hashing_children = false;
            return OBJ_EXEC_TREE_STATE_EOF;
        }
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    } else if (child_state == OBJ_EXEC_TREE_STATE_INTERNAL_ERROR) {
        /* error occurred */
        return child_state;
    }
}

static obj_exec_tree_exec_state_t obj_exec_tree_and_node_hash_other_children(obj_exec_tree_and_node_t *and_node, obj_exec_working_set_id_t *out) {
    obj_exec_working_set_id_t id = OBJ_EXEC_WORKING_SET_INVALID_ID;
    obj_exec_tree_exec_state_t child_state = obj_exec_tree_and_node_work_child(and_node, and_node->current_child, &id);
    if (child_state == OBJ_EXEC_TREE_STATE_ADVANCED) {
        obj_exec_working_set_member_t *member = obj_exec_working_set_get(and_node->ws, id);
        /* check in datamap */
        if (!obj_prealloc_map_find(&and_node->data_map, &member->record)) {
            /* not found in previous */
        } else {
            /* add */
            obj_set_add(&and_node->seen_map, &member->record);
        }
        obj_exec_working_set_free(and_node->ws, id);
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    } else if (child_state == OBJ_EXEC_TREE_STATE_EOF) {
        and_node->current_child++;
        /* maintain datamap */
        int i;
        obj_record_t *record = NULL;
        obj_prealloc_map_entry_t *entry = NULL, *next_entry = NULL;
        for (i = 0; i < and_node->data_map.bucket_size; i++) {
            entry = and_node->data_map.bucket[i];
            while (entry) {
                next_entry = entry->next;
                record = *(obj_record_t **)obj_prealloc_map_get_key(&and_node->data_map, entry);
                /* if not in seen_map, remove */
                if (!obj_set_find(&and_node->seen_map, &record)) {
                    obj_prealloc_map_delete_entry(&and_node->data_map, entry);
                }
                entry = next_entry;
            }
        }
        obj_set_delete_all(&and_node->seen_map);
        if (and_node->data_map.size == 0) {
            and_node->hashing_children = false;
            return OBJ_EXEC_TREE_STATE_EOF;
        }
        if (and_node->current_child == and_node->base.children.size) {
            and_node->hashing_children = false;
        }
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    } else if (child_state == OBJ_EXEC_TREE_STATE_INTERNAL_ERROR) {
        /* error occurred */
        return child_state;
    }
}

static obj_exec_tree_exec_state_t obj_exec_tree_and_node_work_child(obj_exec_tree_and_node_t *and_node, int child_index, obj_exec_working_set_id_t *out) {
    obj_exec_working_set_id_t child_id = obj_array_get_index_value(&and_node->look_ahead_results, child_index, obj_exec_working_set_id_t);
    if (child_id != OBJ_EXEC_WORKING_SET_INVALID_ID) {
        *out = obj_array_get_index_value(&and_node->look_ahead_results, child_index, obj_exec_working_set_id_t);
        int invalid_id = OBJ_EXEC_WORKING_SET_INVALID_ID;
        obj_array_set_index(&and_node->look_ahead_results, child_index, &invalid_id);
        return OBJ_EXEC_TREE_STATE_ADVANCED;
    } else {
        obj_exec_tree_base_node_t *child = obj_array_get_index_value(&and_node->base.children, child_index, uintptr_t);
        return child->methods->work(child, out);
    }
}

/* get type */
obj_exec_tree_node_type_t obj_exec_tree_and_node_get_type() {
    return OBJ_EXEC_TREE_NODE_TYPE_AND;
}


/* is eof */
obj_bool_t obj_exec_tree_and_node_is_eof(obj_exec_tree_base_node_t *node) {
    obj_exec_tree_and_node_t *and_node = (obj_exec_tree_and_node_t *)node;
    if (and_node->look_ahead_results.size == 0) {
        return false;
    }
    if (and_node->hashing_children) {
        return false;
    }
    if (and_node->data_map.size == 0) {
        return true;
    }
    /* we are done when the last child is done */
    obj_exec_working_set_id_t last_child_id = obj_array_get_index_value(&and_node->look_ahead_results, node->children.size - 1, obj_exec_working_set_id_t);
    obj_exec_tree_base_node_t *last_child = (obj_exec_tree_base_node_t *)obj_array_get_index_value(&node->children, node->children.size - 1, uintptr_t);
    return last_child == OBJ_EXEC_WORKING_SET_INVALID_ID && last_child->methods->is_eof(last_child);
}



/* ********** or node ********** */

static obj_exec_tree_node_methods_t obj_exec_tree_or_node_methods = {
    obj_exec_tree_or_node_work,
    obj_exec_tree_or_node_get_type,
    obj_exec_tree_or_node_is_eof
};


obj_exec_tree_or_node_t *obj_exec_tree_or_node_create(obj_exec_working_set_t *ws) {
    obj_exec_tree_or_node_t *or_node = obj_alloc(sizeof(obj_exec_tree_or_node_t));
    obj_exec_tree_init_base((obj_exec_tree_base_node_t *)or_node, &obj_exec_tree_or_node_methods);
    or_node->current_child = 0;
    or_node->ws = ws;
    /* init set */
    obj_set_init(&or_node->seen, &record_set_methods, sizeof(obj_record_t *));
    return or_node;
}


/* or node work() */
obj_exec_tree_exec_state_t obj_exec_tree_or_node_work(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out) {
    if (node->methods->is_eof(node)) {
        return OBJ_EXEC_TREE_STATE_EOF;
    }
    obj_exec_tree_or_node_t *or_node = (obj_exec_tree_or_node_t *)node;
    obj_exec_working_set_id_t id = OBJ_EXEC_WORKING_SET_INVALID_ID;
    obj_exec_tree_base_node_t *child = (obj_exec_tree_base_node_t *)obj_array_get_index_value(&node->children, or_node->current_child, uintptr_t);
    obj_exec_tree_exec_state_t child_state = child->methods->work(child, &id);
    if (child_state == OBJ_EXEC_TREE_STATE_ADVANCED) {
        obj_exec_working_set_member_t *member = obj_exec_working_set_get(or_node->ws, id);
        /* use the set to remove duplicates */
        if (obj_set_find(&or_node->seen, &member->record)) {
            /* have seen before */
            obj_exec_working_set_free(or_node->ws, id);
            /* try again */
            return OBJ_EXEC_TREE_STATE_NEED_TIME;
        } else {
            /* obj_exec_working_set_free(or_node->ws, id); */
            obj_set_add(&or_node->seen, &member->record);
        }
    } else if (child_state == OBJ_EXEC_TREE_STATE_EOF) {
        or_node->current_child++;
        if (node->methods->is_eof(node)) {
            return OBJ_EXEC_TREE_STATE_EOF;
        } else {
            return OBJ_EXEC_TREE_STATE_NEED_TIME;
        }
    } else if (child_state == OBJ_EXEC_TREE_STATE_INTERNAL_ERROR) {
        /* TODO error */
    } 
    return child_state;
}

/* get type */
obj_exec_tree_node_type_t obj_exec_tree_or_node_get_type() {
    return OBJ_EXEC_TREE_NODE_TYPE_OR;
}


/* is eof */
obj_bool_t obj_exec_tree_or_node_is_eof(obj_exec_tree_base_node_t *node) {
    obj_exec_tree_or_node_t *or_node = (obj_exec_tree_or_node_t *)node;
    /* if we are still working on one child */
    return or_node->current_child >= node->children.size;
}


/* ********** collection scan node ********** */

static obj_exec_tree_node_methods_t obj_exec_tree_collection_scan_node_methods = {
    obj_exec_tree_collection_scan_node_work,
    obj_exec_tree_collection_scan_node_get_type,
    obj_exec_tree_collection_scan_node_is_eof
};

obj_exec_tree_collection_scan_node_t *obj_exec_tree_collection_scan_node_create(obj_exec_working_set_t *ws, obj_expr_base_expr_t *filter, int direction, obj_collection_catalog_entry_t *collection) {
    obj_exec_tree_collection_scan_node_t *collection_scan_node = obj_alloc(sizeof(obj_exec_tree_collection_scan_node_t));
    obj_exec_tree_init_base((obj_exec_tree_base_node_t *)collection_scan_node, &obj_exec_tree_collection_scan_node_methods);
    collection_scan_node->direction = direction;
    collection_scan_node->ws = ws;
    collection_scan_node->filter = filter;
    collection_scan_node->collection = collection;
    collection_scan_node->iter = NULL;
    collection_scan_node->end = false;
    return collection_scan_node;
}


/* collection scan node work() */
static obj_exec_tree_exec_state_t obj_exec_tree_collection_scan_node_work(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out) {
    obj_exec_tree_collection_scan_node_t *collection_scan_node = (obj_exec_tree_collection_scan_node_t *)node;
    obj_record_t *record;
    if (collection_scan_node->iter == NULL) {
        collection_scan_node->iter = obj_record_store_iterator_create(collection_scan_node->collection->record_store, collection_scan_node->direction);
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    }
    record = obj_record_store_iterator_next(collection_scan_node->iter);
    if (record == NULL) {
        collection_scan_node->end = true;
        return OBJ_EXEC_TREE_STATE_EOF;
    }
    obj_exec_working_set_id_t id = obj_exec_working_set_allocate(collection_scan_node->ws);
    obj_exec_working_set_member_t *member = obj_exec_working_set_get(collection_scan_node->ws, id);
    member->record = record;
    return obj_exec_tree_collection_scan_node_return_if_matches(collection_scan_node->ws, member, id, out);
}

static obj_exec_tree_node_type_t obj_exec_tree_collection_scan_node_get_type() {
    return OBJ_EXEC_TREE_NODE_TYPE_COLLECTION_SCAN;
}

static obj_exec_tree_exec_state_t obj_exec_tree_collection_scan_node_return_if_matches(obj_exec_working_set_t *ws, obj_exec_working_set_member_t *member, obj_exec_working_set_id_t id, obj_exec_working_set_id_t *out) {
    if () {
        *out = id;
        return OBJ_EXEC_TREE_STATE_ADVANCED;
    }else {
        obj_exec_working_set_free(ws, id);
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    }
}

/* is eof */
static obj_bool_t obj_exec_tree_collection_scan_node_is_eof(obj_exec_tree_base_node_t *node) {
    obj_exec_tree_collection_scan_node_t *collection_scan_node = (obj_exec_tree_collection_scan_node_t *)node;
    return collection_scan_node->end == true;
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

static obj_exec_tree_node_methods_t obj_exec_tree_sort_node_methods = {

};

obj_exec_tree_sort_node_t *obj_exec_tree_sort_node_create(obj_exec_working_set_t *ws, obj_exec_tree_base_node_t *child, obj_bson_t *pattern) {
    obj_exec_tree_sort_node_t *sort_node = obj_alloc(sizeof(obj_exec_tree_sort_node_t));
    obj_exec_tree_init_base((obj_exec_tree_base_node_t *)sort_node, &obj_exec_tree_sort_node_methods);
    /* add child */
    obj_array_push_back(&sort_node->base.children, &child);
    sort_node->ws = ws;
    sort_node->pattern = pattern;
    sort_node->sorted = false;

    return sort_node;
}   



/* compare */
static int obj_exec_tree_sort_node_data_item_compare(const void *data_item1, const void *data_item2) {
    obj_exec_tree_sort_node_data_item_t *sort_node_data_item1 = (obj_exec_tree_sort_node_data_item_t *)data_item1;
    obj_exec_tree_sort_node_data_item_t *sort_node_data_item2 = (obj_exec_tree_sort_node_data_item_t *)data_item2;
    /* TODO optimize */
    
    return obj_bson_compare(sort_node_data_item1->sort_key, sort_node_data_item2->sort_key, sort_node_data_item1->pattern);
}

/* sort node work() */
static obj_exec_tree_exec_state_t obj_exec_tree_sort_node_work(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out) {
    obj_exec_tree_sort_node_t *sort_node = (obj_exec_tree_sort_node_t *)node;
    /* check memory usage */

    /* all works are done, return eof */
    if (node->methods->is_eof(node)) {
        return OBJ_EXEC_TREE_STATE_EOF;
    }
    
    /* if not sorted yet */
    if (!sort_node->sorted) {
        obj_exec_working_set_id_t id = OBJ_EXEC_WORKING_SET_INVALID_ID;
        obj_exec_tree_base_node_t *child = obj_plan_tree_get_child(node);
        obj_exec_tree_exec_state_t state = child->methods->work(child, &id);
        if (state == OBJ_EXEC_TREE_STATE_ADVANCED) {
            obj_exec_working_set_member_t *member = obj_exec_working_set_get(sort_node->ws, id);
            obj_exec_tree_sort_node_data_item_t data_item;
            data_item.ws_id = id;
            /* TODO set sort key */
            data_item.record = member->record;
            /* set pattern */
            data_item.pattern = sort_node->pattern;
            /* add to data set */
            obj_array_push_back(&sort_node->data, &data_item);
            return OBJ_EXEC_TREE_STATE_NEED_TIME;
        } else if (state == OBJ_EXEC_TREE_STATE_EOF) {
            /* child done. do the sort */
            obj_array_sort(&sort_node->data, obj_exec_tree_sort_node_data_item_compare);
            /* set to begin */
            sort_node->curr = 0;
            sort_node->sorted = true;
            return OBJ_EXEC_TREE_STATE_NEED_TIME;
        } else if (state == OBJ_EXEC_TREE_STATE_INTERNAL_ERROR) {
            /* TODO error occurred */
        }

        return state;
    }
    /* already sorted */
    obj_assert(sort_node->sorted);
    /* we haven't returned all results */
    obj_assert(sort_node->data.size != sort_node->curr);
    obj_exec_tree_sort_node_data_item_t *data_item = (obj_exec_tree_sort_node_data_item_t *)obj_array_get_index(&sort_node->data, sort_node->curr);
    *out = data_item->ws_id;
    sort_node->curr++;
    return OBJ_EXEC_TREE_STATE_ADVANCED;
}

/* get type */
static obj_exec_tree_node_type_t obj_exec_tree_sort_node_get_type() {
    return OBJ_EXEC_TREE_NODE_TYPE_SORT;
}

/* is eof */
static obj_bool_t obj_exec_tree_sort_node_is_eof(obj_exec_tree_base_node_t *node) {
    obj_exec_tree_sort_node_t *sort_node = (obj_exec_tree_sort_node_t *)node;
    obj_exec_tree_base_node_t *child = obj_exec_tree_get_child(node);
    /* 
     * done if:
     * 1. child has no more results 
     * 2. results from child are sorted
     * 3. we have returned all results
     */
    return child->methods->is_eof(child) && sort_node->sorted && sort_node->data.size == sort_node->curr;
}

/* ********** projection node ********** */

static obj_exec_tree_node_methods_t obj_exec_tree_projection_node_methods = {
    obj_exec_tree_projection_node_work,
    obj_exec_tree_projection_node_get_type,
    obj_exec_tree_projection_node_is_eof
};

obj_exec_tree_projection_node_t *obj_exec_tree_projection_node_create(obj_exec_working_set_t *ws, obj_exec_tree_base_node_t *child, obj_bson_t *projection) {
    obj_exec_tree_projection_node_t *projection_node = (obj_exec_tree_projection_node_t *)obj_alloc(sizeof(obj_exec_tree_projection_node_t));
    obj_exec_tree_init_base((obj_exec_tree_base_node_t *)projection_node, &obj_exec_tree_projection_node_methods);
    projection_node->projection = projection;
    projection_node->ws = ws;
    /* add child */
    obj_array_push_back(&projection_node->base.children, &child);
    return projection_node;
}

/* projection node work() */
static obj_exec_tree_exec_state_t obj_exec_tree_projection_node_work(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out) {
    obj_exec_tree_projection_node_t *projection_node = (obj_exec_tree_projection_node_t *)node;
    obj_exec_working_set_id_t id = OBJ_EXEC_WORKING_SET_INVALID_ID;
    obj_exec_tree_base_node_t *child = obj_exec_tree_get_child(node);
    obj_exec_tree_exec_state_t state = child->methods->work(child, &id);
    if (state == OBJ_EXEC_TREE_STATE_ADVANCED) {
        obj_exec_working_set_member_t *member = obj_exec_working_set_get(projection_node->ws, id);
        /* TODO projection */
        *out = id;
    } else if (state == OBJ_EXEC_TREE_STATE_INTERNAL_ERROR) {
        /* TODO error occurred */
    }
    return state;
}

/* get type */
static obj_exec_tree_node_type_t obj_exec_tree_projection_node_get_type() {
    return OBJ_EXEC_TREE_NODE_TYPE_PROJECTION;
}


/* is eof */
static obj_bool_t obj_exec_tree_projection_node_is_eof(obj_exec_tree_base_node_t *node) {
    obj_exec_tree_base_node_t *child = obj_exec_tree_get_child(node);
    return child->methods->is_eof(child);
}

/* ********** skip node ********** */

static obj_exec_tree_node_methods_t obj_exec_tree_skip_node_methods = {
    obj_exec_tree_skip_node_work,
    obj_exec_tree_skip_node_get_type,
    obj_exec_tree_skip_node_is_eof
};


obj_exec_tree_skip_node_t *obj_exec_tree_skip_node_create(obj_exec_working_set_t *ws, obj_exec_tree_base_node_t *child, int skip) {
    obj_exec_tree_skip_node_t *skip_node = (obj_exec_tree_skip_node_t *)obj_alloc(sizeof(obj_exec_tree_skip_node_t));
    obj_exec_tree_init_base((obj_exec_tree_base_node_t *)skip_node, &obj_exec_tree_skip_node_methods);
    /* add child */
    obj_array_push_back(&skip_node->base.children, &child);
    skip_node->ws = ws;
    skip_node->num_to_skip = skip;
    return skip_node;
}


/* skip node work() */
static obj_exec_tree_exec_state_t obj_exec_tree_skip_node_work(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out) {
    obj_exec_tree_skip_node_t *skip_node = (obj_exec_tree_skip_node_t *)node;
    obj_exec_working_set_id_t id = OBJ_EXEC_WORKING_SET_INVALID_ID;
    obj_exec_tree_base_node_t *child = obj_exec_tree_get_child(node);
    obj_exec_tree_exec_state_t state = child->methods->work(child, &id);
    if (state == OBJ_EXEC_TREE_STATE_ADVANCED) {
        /* skip if need */
        if (skip_node->num_to_skip > 0) {
            --skip_node->num_to_skip;
            obj_exec_working_set_free(skip_node->ws, id);
            return OBJ_EXEC_TREE_STATE_NEED_TIME;
        }
        *out = id;
        /* try again */
        return OBJ_EXEC_TREE_STATE_NEED_TIME;
    } else if (state == OBJ_EXEC_TREE_STATE_INTERNAL_ERROR) {
        /* TODO error occurred */

    }
    return state;
}

/* get type */
static obj_exec_tree_node_type_t obj_exec_tree_skip_node_get_type() {
    return OBJ_EXEC_TREE_NODE_TYPE_SKIP;
}

/* is eof */
static obj_bool_t obj_exec_tree_skip_node_is_eof(obj_exec_tree_base_node_t *node) {
    obj_exec_tree_skip_node_t *skip_node = (obj_exec_tree_skip_node_t *)node;
    obj_exec_tree_base_node_t *child = obj_plan_tree_get_child(node);
    return child->methods->is_eof(child);
}

/* ********** limit node ********** */

static obj_exec_tree_node_methods_t obj_exec_tree_limit_node_methods = {
    obj_exec_tree_limit_node_work,
    obj_exec_tree_limit_node_get_type,
    obj_exec_tree_limit_node_is_eof
};

obj_exec_tree_limit_node_t *obj_exec_tree_limit_node_create(obj_exec_working_set_t *ws, obj_exec_tree_base_node_t *child, int limit) {
    obj_exec_tree_limit_node_t *limit_node = (obj_exec_tree_limit_node_t *)obj_alloc(sizeof(obj_exec_tree_limit_node_t));
    obj_exec_tree_init_base((obj_exec_tree_base_node_t *)limit_node, &obj_exec_tree_limit_node_methods);
    /* add child */
    obj_array_push_back(&limit_node->base.children, &child);
    limit_node->ws = ws;
    limit_node->num_to_return = limit;
    return limit_node;
}


/* limit node work() */
static obj_exec_tree_exec_state_t obj_exec_tree_limit_node_work(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out) {
    obj_exec_tree_limit_node_t *limit_node = (obj_exec_tree_limit_node_t *)node;
    if (limit_node->num_to_return == 0) {
        return OBJ_EXEC_TREE_STATE_EOF;
    }
    obj_exec_working_set_id_t id = OBJ_EXEC_WORKING_SET_INVALID_ID;
    obj_exec_tree_base_node_t *child = obj_exec_tree_get_child(node);
    obj_exec_tree_exec_state_t state = child->methods->work(child, &id);
    if (state == OBJ_EXEC_TREE_STATE_ADVANCED) {
        /* returned one */
        *out = id;
        --limit_node->num_to_return;
    } else if (state == OBJ_EXEC_TREE_STATE_INTERNAL_ERROR) {
        /* TODO error occurred */
    }

    return state;
}

/* get type */
static obj_exec_tree_node_type_t obj_exec_tree_limit_node_get_type() {
    return OBJ_EXEC_TREE_NODE_TYPE_LIMIT;
}

/* is eof */
static obj_bool_t obj_exec_tree_limit_node_is_eof(obj_exec_tree_base_node_t *node) {
    obj_exec_tree_limit_node_t *limit_node = (obj_exec_tree_limit_node_t *)node;
    obj_exec_tree_base_node_t *child = obj_exec_tree_get_child(node);
    return limit_node->num_to_return == 0 || child->methods->is_eof(child);
}

/* ********** eof node ********** */

static obj_exec_tree_node_methods_t obj_exec_tree_eof_node_methods = {
    obj_exec_tree_eof_node_work,
    obj_exec_tree_eof_node_get_type,
    obj_exec_tree_eof_node_is_eof
};

obj_exec_tree_eof_node_t *obj_exec_tree_eof_node_create() {
    obj_exec_tree_eof_node_t *eof_node = (obj_exec_tree_eof_node_t *)obj_alloc(sizeof(obj_exec_tree_eof_node_t));
    obj_exec_tree_init_base((obj_exec_tree_base_node_t *)eof_node, &obj_exec_tree_eof_node_methods);
    return eof_node;
}


/* eof node work() */
static obj_exec_tree_exec_state_t obj_exec_tree_eof_node_work(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out) {
    return OBJ_EXEC_TREE_STATE_EOF;
}

/* get type */
static obj_exec_tree_node_type_t obj_exec_tree_eof_node_get_type() {
    return OBJ_EXEC_TREE_NODE_TYPE_EOF;
}

/* is eof */
static obj_bool_t obj_exec_tree_eof_node_is_eof(obj_exec_tree_base_node_t *node) {
    return true;
}