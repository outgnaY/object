#include "obj_core.h"

/* skiplist methods */
static obj_skiplist_node_t *obj_skiplist_node_create(int level, obj_bson_t *key, obj_record_t *record);
static void obj_skiplist_node_destroy(obj_skiplist_node_t *node);
static int obj_skiplist_random_level();
static obj_bool_t obj_skiplist_locate(obj_skiplist_t *skiplist, obj_bson_t *key, obj_bool_t inclusive, obj_skiplist_node_t **out);
static obj_skiplist_node_t *obj_skiplist_advance_to(obj_skiplist_t *skiplist, obj_skiplist_node_t *current, obj_bson_t *key, obj_bool_t inclusive);
static obj_bool_t obj_skiplist_locate_seek_point(obj_skiplist_t *skiplist, obj_index_seek_point_t *seek_point, obj_skiplist_node_t **out);
static obj_skiplist_node_t *obj_skiplist_advance_to_seek_point(obj_skiplist_t *skiplist, obj_skiplist_node_t *current, obj_index_seek_point_t *seek_point);
static void obj_skiplist_node_dump(obj_skiplist_node_t *node);

/* index methods */
static int obj_index_key_order_get(obj_index_key_order_t order, int i);
static unsigned obj_index_key_order_descending(obj_index_key_order_t order, unsigned mask);
static int obj_index_key_value_compare(obj_bson_value_t *value1, obj_bson_value_t *value2);
static int obj_index_key_compare(obj_bson_t *key1, obj_bson_t *key2, obj_index_key_order_t order);
static int obj_index_key_compare_with_seek_point(obj_bson_t *left, obj_index_seek_point_t *right, obj_index_key_order_t order);

/* index iterator methods */
static obj_bool_t obj_index_iterator_is_eof(obj_index_iterator_t *iter);
static void obj_index_iterator_mark_eof(obj_index_iterator_t *iter);
static obj_bool_t obj_index_iterator_has_end_state(obj_index_iterator_t *iter);
static obj_bool_t obj_index_iterator_at_or_past_end_point_after_seeking(obj_index_iterator_t *iter);
static obj_bool_t obj_index_iterator_at_end_point(obj_index_iterator_t *iter);
static void obj_index_iterator_seek_end_cursor(obj_index_iterator_t *iter);
static obj_index_key_entry_t obj_index_iterator_curr(obj_index_iterator_t *iter);
static void obj_index_iterator_advance(obj_index_iterator_t *iter);


/* ********** skiplist methods ********** */

/* create skiplist */
obj_skiplist_t *obj_skiplist_create(obj_index_key_order_t order) {
    obj_skiplist_t *skiplist = obj_alloc(sizeof(obj_skiplist_t));
    skiplist->level = 1;
    skiplist->length = 0;
    skiplist->head = obj_skiplist_node_create(OBJ_SKIPLIST_MAXLEVEL, NULL, NULL);
    int i;
    for (i = 0; i < OBJ_SKIPLIST_MAXLEVEL; i++) {
        skiplist->head->levels[i].forward = NULL;
    }
    skiplist->head->backward = NULL;
    skiplist->tail = NULL;
    skiplist->order = order;
    return skiplist;
}

/* destroy skiplist */
void obj_skiplist_destroy(obj_skiplist_t *skiplist) {
    obj_skiplist_node_t *node = skiplist->head->levels[0].forward;
    obj_skiplist_node_t *next = NULL;
    obj_free(skiplist->head);
    while (node) {
        next = node->levels[0].forward;
        obj_skiplist_node_destroy(node);
        node = next;
    }
    obj_free(skiplist);
}

/* create skiplist node */
static obj_skiplist_node_t *obj_skiplist_node_create(int level, obj_bson_t *key, obj_record_t *record) {
    obj_skiplist_node_t *node = obj_alloc(sizeof(obj_skiplist_node_t) + level * sizeof(obj_skiplist_level_t));
    node->level = level;
    node->key = key;
    node->record = record;
    return node;
}

/* destroy skiplist node */
static void obj_skiplist_node_destroy(obj_skiplist_node_t *node) {
    obj_free(node);
}


static int obj_skiplist_random_level() {
    int level = 1;
    while ((random() & 0xffff) < (OBJ_SKIPLIST_P * 0xffff)) {
        level += 1;
    }
    return (level < OBJ_SKIPLIST_MAXLEVEL) ? level : OBJ_SKIPLIST_MAXLEVEL;
}

/* insert */
void obj_skiplist_insert(obj_skiplist_t *skiplist, obj_bson_t *key, obj_record_t *record) {
    obj_skiplist_node_t *update[OBJ_SKIPLIST_MAXLEVEL];
    obj_skiplist_node_t *x;
    int i, level;
    x = skiplist->head;
    for (i = skiplist->level - 1; i >= 0; i--) {
        while (x->levels[i].forward && obj_index_key_compare(x->levels[i].forward->key, key, skiplist->order) == -1) {
            x = x->levels[i].forward;
        }
        update[i] = x;
    }
    level = obj_skiplist_random_level();
    if (level > skiplist->level) {
        for (i = skiplist->level; i < level; i++) {
            update[i] = skiplist->head;
        }
        skiplist->level = level;
    }
    x = obj_skiplist_node_create(level, key, record);
    for (i = 0; i < level; i++) {
        x->levels[i].forward = update[i]->levels[i].forward;
        update[i]->levels[i].forward = x;
    }
    x->backward = (update[0] == skiplist->head) ? NULL : update[0];
    if (x->levels[0].forward) {
        x->levels[0].forward->backward = x;
    } else {
        skiplist->tail = x;
    }
    skiplist->length++;
    /* return x; */
}

/* delete */



/* from start */
static obj_bool_t obj_skiplist_locate(obj_skiplist_t *skiplist, obj_bson_t *key, obj_bool_t inclusive, obj_skiplist_node_t **out) {
    obj_skiplist_node_t *node = skiplist->head;
    int i;
    if (inclusive) {
        for (i = skiplist->level - 1; i >= 0; i--) {
            /* < seek point */
            while (node->levels[i].forward && obj_index_key_compare(node->levels[i].forward->key, key, skiplist->order) == -1) {
                node = node->levels[i].forward;
            }
        }
    } else {
        for (i = skiplist->level - 1; i >= 0; i--) {
            /* <= seek point */
            while (node->levels[i].forward && obj_index_key_compare(node->levels[i].forward->key, key, skiplist->order) <= 0) {
                node = node->levels[i].forward;
            }
        }
    }
    
    /* maybe NULL */
    *out = node->levels[0].forward;
    if (*out == NULL) {
        return false;
    }
    if (obj_index_key_compare((*out)->key, key, skiplist->order) == 0) {
        if (inclusive) {
            return true;
        }
        return false;
    }
    return false;
}

/* advance to */
static obj_skiplist_node_t *obj_skiplist_advance_to(obj_skiplist_t *skiplist, obj_skiplist_node_t *current, obj_bson_t *key, obj_bool_t inclusive) {
    obj_skiplist_node_t *node = current;
    obj_bson_visit_print_visit(node->key);
    int i;
    if (inclusive) {
        for (i = current->level - 1; i >= 0; i--) {
            while (node->levels[i].forward && obj_index_key_compare(node->levels[i].forward->key, key, skiplist->order) == -1) {
                node = node->levels[i].forward;
            }
        }
    } else {
        for (i = current->level - 1; i >= 0; i--) {
            while (node->levels[i].forward && obj_index_key_compare(node->levels[i].forward->key, key, skiplist->order) <= 0) {
                node = node->levels[i].forward;
            }
        }
    }
    
    /* maybe NULL */
    return node->levels[0].forward;
}



/* start from head */
static obj_bool_t obj_skiplist_locate_seek_point(obj_skiplist_t *skiplist, obj_index_seek_point_t *seek_point, obj_skiplist_node_t **out) {
    obj_skiplist_node_t *node = skiplist->head;
    int i;
    for (i = skiplist->level - 1; i >= 0; i--) {
        /* < seek point */
        while (node->levels[i].forward && obj_index_key_compare_with_seek_point(node->levels[i].forward->key, seek_point, skiplist->order) == -1) {
            node = node->levels[i].forward;
        }
    }
    /* maybe NULL */
    *out = node->levels[0].forward;
    if (*out == NULL) {
        return false;
    }
    if (obj_index_key_compare_with_seek_point((*out)->key, seek_point, skiplist->order) == 0) {
        return true;
    }
    return false;
}

/* 
 * find smallest value greater-equal than specified
 * start from current location 
 */
static obj_skiplist_node_t *obj_skiplist_advance_to_seek_point(obj_skiplist_t *skiplist, obj_skiplist_node_t *current, obj_index_seek_point_t *seek_point) {
    obj_skiplist_node_t *node = current;
    int i;
    for (i = current->level - 1; i >= 0; i--) {
        /* < seek point */
        while (node->levels[i].forward && obj_index_key_compare_with_seek_point(node->levels[i].forward->key, seek_point, skiplist->order) == -1) {
            node = node->levels[i].forward;
        }
    }
    /* maybe NULL */
    return node->levels[0].forward;
}

void obj_skiplist_dump(obj_skiplist_t *skiplist) {
    printf("********** skiplist **********\n");
    obj_skiplist_node_t *node = skiplist->head->levels[0].forward;
    int i = 0;
    while (node) {
        printf("%d:\n", i);
        obj_skiplist_node_dump(node);
        node = node->levels[0].forward;
        i++;
    }
}

static void obj_skiplist_node_dump(obj_skiplist_node_t *node) {
    printf("key:\n");
    obj_bson_visit_print_visit(node->key);
}



/* ********** index methods ********** */

/* 
 * for key pattern {a: 1, b: -1} 
 * get(0) == 1; get(1) == -1
 */
static int obj_index_key_order_get(obj_index_key_order_t order, int i) {
    return ((1 << i) & order) ? -1 : 1;
}

static unsigned obj_index_key_order_descending(obj_index_key_order_t order, unsigned mask) {
    return order & mask;
}

static int obj_index_key_value_compare(obj_bson_value_t *value1, obj_bson_value_t *value2) {
    obj_bson_type_t type1 = value1->type;
    obj_bson_type_t type2 = value2->type;
    if (type1 >= OBJ_BSON_TYPE_MIN) {
        if (type1 ==OBJ_BSON_TYPE_MIN) {
            if (type2 >= OBJ_BSON_TYPE_MIN) {
                /* type2 special */
                if (type2 == OBJ_BSON_TYPE_MIN) {
                    return 0;
                } else {
                    return -1;
                }
            } else {
                /* type2 not special */
                return -1;
            }
        } else {
            if (type2 >= OBJ_BSON_TYPE_MIN) {
                if (type2 == OBJ_BSON_TYPE_MIN) {
                    return 1;
                } else {
                    return 0;
                }
            } else {
                /* type2 not special */
                return 1;
            }
        }
        
    } else if (type2 >= OBJ_BSON_TYPE_MIN) {
        /* type1 not special, type2 special */
        if (type2  == OBJ_BSON_TYPE_MIN) {
            return 1;
        } else {
            return -1;
        }
    }
    switch (type1) {
        case OBJ_BSON_TYPE_DOUBLE: {
            if (value1->value.v_double < value2->value.v_double) {
                return -1;
            } else if (value1->value.v_double > value2->value.v_double) {
                return 1;
            } else {
                return 0;
            }
        }
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
        case OBJ_BSON_TYPE_INT32: {
            if (value1->value.v_int32 < value2->value.v_int32) {
                return -1;
            } else if (value1->value.v_int32 > value2->value.v_int32) {
                return 1;
            } else {
                return 0;
            }
        }
        case OBJ_BSON_TYPE_INT64: {
            if (value1->value.v_int64 < value2->value.v_int64) {
                return -1;
            } else if (value1->value.v_int64 > value2->value.v_int64) {
                return 1;
            } else {
                return 0;
            }
        }
        default:
            obj_assert(0);
    }
}

/* compare index keys */
static int obj_index_key_compare(obj_bson_t *key1, obj_bson_t *key2, obj_index_key_order_t order) {
    obj_bson_iter_t iter1;
    obj_bson_iter_t iter2;
    obj_bson_value_t *value1;
    obj_bson_value_t *value2;
    obj_bson_iter_init(&iter1, key1);
    obj_bson_iter_init(&iter2, key2);
    unsigned mask = 1;
    obj_bool_t res1;
    obj_bool_t res2;
    while (true) {
        res1 = obj_bson_iter_next(&iter1);
        res2 = obj_bson_iter_next(&iter2);
        if (!res1) {
            obj_assert(!res2);
            break;
        }
        value1 = obj_bson_iter_value(&iter1);
        value2 = obj_bson_iter_value(&iter2);
        int x = obj_index_key_value_compare(value1, value2);
        if (obj_index_key_order_descending(order, mask)) {
            x = -x;
        }
        if (x != 0) {
            return x;
        }
        mask <<= 1;
    }

    return 0;

}

/* compare index key with index seek point */
static int obj_index_key_compare_with_seek_point(obj_bson_t *left, obj_index_seek_point_t *right, obj_index_key_order_t order) {
    obj_assert(right->key_suffix.size == right->suffix_inclusive.size);
    obj_bson_iter_t iter_left;
    obj_bson_iter_t iter_right;
    obj_bson_value_t *value_left;
    obj_bson_value_t *value_right;
    obj_bson_iter_init(&iter_left, left);
    obj_bson_iter_init(&iter_right, right->key_prefix);
    unsigned mask = 1;
    int i;
    for (i = 0; i < right->prefix_len; i++) {
        obj_bson_iter_next(&iter_left);
        obj_bson_iter_next(&iter_right);
        value_left = obj_bson_iter_value(&iter_left);
        value_right = obj_bson_iter_value(&iter_right);
        int x = obj_index_key_value_compare(value_left, value_right);
        if (obj_index_key_order_descending(order, mask)) {
            x = -x;
        }
        if (x != 0) {
            return x;
        }

        mask <<= 1;
    }
    /* equal prefix */
    if (right->prefix_exclusive) {
        return -1;
    }
    for (; i < right->key_suffix.size; i++) {
        obj_bson_iter_next(&iter_left);
        value_right = (obj_bson_value_t *)obj_array_get_index_value(&right->key_suffix, i, uintptr_t);
        value_left = obj_bson_iter_value(&iter_left);
        int x = obj_index_key_value_compare(value_left, value_right);
        if (obj_index_key_order_descending(order, mask)) {
            x = -x;
        }
        if (x != 0) {
            return x;
        }
        if (!obj_array_get_index_value(&right->suffix_inclusive, i, obj_bool_t)) {
            return -1;
        }
        mask <<= 1;
    }
    return 0;

}


/* ********** index iterator methods ********** */

obj_index_iterator_t *obj_index_iterator_create(obj_index_catalog_entry_t *index_entry) {
    obj_index_iterator_t *iter = obj_alloc(sizeof(obj_index_iterator_t));
    iter->end_state.key = NULL;
    iter->end_state.node = NULL;
    iter->skiplist = index_entry->skiplist;
    /* init: head->next */
    iter->cur_node = index_entry->skiplist->head->levels[0].forward;
    return iter;
}

/* is eof */
static inline obj_bool_t obj_index_iterator_is_eof(obj_index_iterator_t *iter) {
    return iter->cur_node == NULL;
}

/* mark eof */
static inline void obj_index_iterator_mark_eof(obj_index_iterator_t *iter) {
    printf("mark eof\n");
    iter->cur_node = NULL;
}

static inline obj_bool_t obj_index_iterator_has_end_state(obj_index_iterator_t *iter) {
    return iter->end_state.key != NULL && iter->end_state.node != NULL;
}

/* check end point */
static inline obj_bool_t obj_index_iterator_at_or_past_end_point_after_seeking(obj_index_iterator_t *iter) {
    if (!obj_index_iterator_has_end_state(iter)) {
        return false;
    }
    if (obj_index_iterator_is_eof(iter)) {
        return true;
    }
    /* compare */
    int cmp = obj_index_key_compare(iter->cur_node->key, iter->end_state.key, iter->skiplist->order);
    printf("cmp = %d\n", cmp);
    return iter->end_state.inclusive ? cmp > 0 : cmp >= 0;
}

/* at end */
static inline obj_bool_t obj_index_iterator_at_end_point(obj_index_iterator_t *iter) {
    return obj_index_iterator_has_end_state(iter) && iter->cur_node == iter->end_state.node;
}

/* set end position */
inline void obj_index_iterator_set_end_position(obj_index_iterator_t *iter, obj_bson_t *key, obj_bool_t inclusive) {
    iter->end_state.key = key;
    iter->end_state.inclusive = inclusive;
    obj_index_iterator_seek_end_cursor(iter);
}

/* seek end cursor */
static inline void obj_index_iterator_seek_end_cursor(obj_index_iterator_t *iter) {
    obj_skiplist_locate(iter->skiplist, iter->end_state.key, iter->end_state.inclusive, &iter->end_state.node);
    printf("end node\n");
    obj_skiplist_node_dump(iter->end_state.node);
}

static inline obj_index_key_entry_t obj_index_iterator_curr(obj_index_iterator_t *iter) {
    obj_index_key_entry_t kv = {NULL, NULL};
    if (iter->cur_node != NULL) {
        kv.key = iter->cur_node->key;
        kv.record = iter->cur_node->record;
    }
    return kv;
}

/* seek */
obj_index_key_entry_t obj_index_iterator_seek(obj_index_iterator_t *iter, obj_bson_t *key, obj_bool_t inclusive) {
    if (obj_index_iterator_is_eof(iter)) {
        return obj_index_iterator_curr(iter);
    }
    obj_bson_visit_print_visit(iter->cur_node->key);
    int cmp = obj_index_key_compare(iter->cur_node->key, key, iter->skiplist->order);
    if (cmp < 0) {
        /* from current position */
        iter->cur_node = obj_skiplist_advance_to(iter->skiplist, iter->cur_node, key, inclusive);
    } else if (cmp > 0) {
        /* from start */
        obj_skiplist_locate(iter->skiplist, key, inclusive, &iter->cur_node);
    } else {
        /* do nothing */
    }
    if (obj_index_iterator_at_or_past_end_point_after_seeking(iter)) {
        obj_index_iterator_mark_eof(iter);
    }
    return obj_index_iterator_curr(iter);

}

/* seek with seek point */
obj_index_key_entry_t obj_index_iterator_seek_with_seek_point(obj_index_iterator_t *iter, obj_index_seek_point_t *seek_point) {
    /* reach end */
    if (obj_index_iterator_is_eof(iter)) {
        return obj_index_iterator_curr(iter);
    }
    int cmp = obj_index_key_compare_with_seek_point(iter->cur_node->key, seek_point, iter->skiplist->order);
    if (cmp < 0) {
        /* from current position */
        iter->cur_node = obj_skiplist_advance_to_seek_point(iter->skiplist, iter->cur_node, seek_point);
    } else if (cmp > 0) {
        /* from start */
        obj_skiplist_locate_seek_point(iter->skiplist, seek_point, &iter->cur_node);
    } else {
        /* do nothing */
    }
    if (obj_index_iterator_at_or_past_end_point_after_seeking(iter)) {
        obj_index_iterator_mark_eof(iter);
    }
    return obj_index_iterator_curr(iter);
}

/* get next */
obj_index_key_entry_t obj_index_iterator_next(obj_index_iterator_t *iter) {
    /* reach end */
    if (obj_index_iterator_is_eof(iter)) {
        return obj_index_iterator_curr(iter);
    }
    /* advance */
    obj_index_iterator_advance(iter);
    if (obj_index_iterator_at_end_point(iter)) {
        obj_index_iterator_mark_eof(iter);
    }
    return obj_index_iterator_curr(iter);
}

/* advance one step */
static void obj_index_iterator_advance(obj_index_iterator_t *iter) {
    iter->cur_node = iter->cur_node->levels[0].forward;
}    


