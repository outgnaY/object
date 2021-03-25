#include "obj_core.h"


/* ********** skiplist methods ********** */

/* create skiplist */
obj_skiplist_t *obj_skiplist_create() {
    obj_skiplist_t *skiplist = obj_alloc(sizeof(obj_skiplist_t));
    skiplist->level = 1;
    skiplist->length = 0;
    obj_skiplist_node_t *node = obj_skiplist_node_create();
    skiplist->head = obj_skiplist_node_create();
    int i;
    for (i = 0; i < OBJ_SKIPLIST_MAXLEVEL; i++) {
        skiplist->head->level[i].forward = NULL;
    }
    skiplist->head->backward = NULL;
    skiplist->tail = NULL;
    return skiplist;
}

/* destroy skiplist */
void obj_skiplist_destroy(obj_skiplist_t *skiplist) {
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

/* create skiplist node */
obj_skiplist_node_t *obj_skiplist_node_create() {
    obj_skiplist_node_t *node = obj_alloc(sizeof(obj_skiplist_node_t));
    /* TODO set data */
    return node;
}

/* destroy skiplist node */
void obj_skiplist_node_destroy(obj_skiplist_node_t *node) {
    /* TODO destroy data */

    obj_free(node);
}


int obj_skiplist_random_level() {
    int level = 1;
    while ((random() & 0xffff) < (OBJ_SKIPLIST_P * 0xffff)) {
        level += 1;;
    }
    return (level < OBJ_SKIPLIST_MAXLEVEL) ? level : OBJ_SKIPLIST_MAXLEVEL;
}



/* ********** index methods ********** */


