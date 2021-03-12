#ifndef OBJ_SKIPLIST_H
#define OBJ_SKIPLIST_h

#include "obj_core.h"

typedef struct obj_skiplist_node_s obj_skiplist_node_t;
typedef struct obj_skiplist_level_s obj_skiplist_level_t;
typedef struct obj_skiplist_s obj_skiplist_t;


struct obj_skiplist_level_s {
    obj_skiplist_node_t *forward;
    int span;
};

struct obj_skiplist_node_s {
    obj_skiplist_node_t *backward;
    /* key */
    obj_index_key_t *index_key;
    /* value */
    obj_v1_record_t *record;
    obj_skiplist_level_t level[];
};

struct obj_skiplist_s {
    obj_skiplist_node_t *head;
    obj_skiplist_node_t *tail;
    int length;
    int level;
    /* compare keys */
    /* int (*compare)(const void *key1, const void *key2); */
};

#define OBJ_SKIPLIST_MAX_LEVEL 32
#define OBJ_SKIPLIST_P 0.25

#endif  /* OBJ_SKIPLIST_H */