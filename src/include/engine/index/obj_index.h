#ifndef OBJ_SKIPLIST_H
#define OBJ_SKIPLIST_H

#include "obj_core.h"


typedef struct obj_skiplist_node_s obj_skiplist_node_t;
typedef struct obj_skiplist_level_s obj_skiplist_level_t;
typedef struct obj_skiplist_s obj_skiplist_t;

typedef struct obj_index_s obj_index_t;
typedef struct obj_index_seek_point_s obj_index_seek_point_t;
typedef struct obj_index_key_entry_s obj_index_key_entry_t;

/* ********** skiplist ********** */

struct obj_skiplist_node_s {
    /* key */

    /* value */
    obj_skiplist_node_t *backward;
    obj_skiplist_level_t level[];
};

struct obj_skiplist_level_s {
    obj_skiplist_node_t *forward;
};

struct obj_skiplist_s {
    int level;
    int length;
    obj_skiplist_node_t *head;
    obj_skiplist_node_t *tail;
};


/* ********** index ********** */

struct obj_index_s {
    obj_skiplist_t *skiplist;
};

/* index seek point describe the position to seek */
struct obj_index_seek_point_s {
    obj_bson_t *key_prefix;
    int prefix_len;
    obj_bool_t prefix_exclusive;
    obj_array_t key_suffix;
    obj_array_t suffix_inclusive;
};

/* wrapper for k-v pair */
struct obj_index_key_entry_s {
    
};


#define OBJ_SKIPLIST_P 0.25
#define OBJ_SKIPLIST_MAXLEVEL 32



/* ********** skiplist methods ********** */



/* ********** index methods ********** */



#endif  /* OBJ_SKIPLIST_H */