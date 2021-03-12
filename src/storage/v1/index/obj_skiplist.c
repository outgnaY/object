#include "obj_core.h"

/* create a skiplist */
obj_skiplist_t *obj_skiplist_create() {
    int i;
    obj_skiplist_t *skiplist = NULL;
    skiplist = obj_alloc(sizeof(obj_skiplist_t));
    if (skiplist == NULL) {
        return NULL;
    }
    skiplist->level = 1;
    skiplist->length = 0;
    skiplist->head = obj_skiplist_node_create(OBJ_SKIPLIST_MAX_LEVEL);
}

/* create a skiplist node */
static obj_skiplist_node_t *obj_skiplist_node_create(int level, obj_index_key_t *index_key, obj_v1_record_t *record) {
    obj_skiplist_node_t *node = NULL;
    node = obj_alloc(sizeof(obj_skiplist_node_t));
    if (node == NULL) {
        return NULL;
    }
    /* init node */
    node->index_key = index_key;
    node->record = record;
    return node;
}

/* destroy a skiplist */
void obj_skiplist_destroy(obj_skiplist_t *skiplist) {
    obj_assert(skiplist);
    obj_skiplist_node_t *node = skiplist->head->level[0].forward;
    obj_skiplist_node_t *next = NULL;
    obj_free(skiplist->head);
    while (node) {
        next = node->level[0].forward;
        obj_skiplist_node_destroy(node);
        node = next;
    }
    obj_free(skiplist);
}

/* destroy a skiplist node */
static void obj_skiplist_node_destroy(obj_skiplist_node_t *node) {
    obj_assert(node);
    /* destroy index key */
    obj_index_key_destroy(node->index_key);
    obj_free(node);
}

/* generate random level */
static int obj_skiplist_random_level() {
    int level = 1;
    while ((random() & 0xffff) < (OBJ_SKIPLIST_P * 0xffff)) {
        level += 1;
    }
    return (level < OBJ_SKIPLIST_MAX_LEVEL) ? level : OBJ_SKIPLIST_MAX_LEVEL;
}

/* insert into skiplist */
void obj_skiplist_insert(obj_skiplist_t *skiplist, obj_index_key_t *index_key, obj_v1_record_t *record) {

}

/* delete from skiplist */


