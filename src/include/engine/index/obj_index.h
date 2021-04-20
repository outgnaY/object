#ifndef OBJ_SKIPLIST_H
#define OBJ_SKIPLIST_H

#include "obj_core.h"

/* forward declaration */
typedef struct obj_index_catalog_entry_s obj_index_catalog_entry_t;

/* skiplist */
typedef struct obj_skiplist_level_s obj_skiplist_level_t;
typedef struct obj_skiplist_node_s obj_skiplist_node_t;
typedef struct obj_skiplist_s obj_skiplist_t;

/* index */
typedef unsigned obj_index_key_order_t;
typedef enum obj_index_key_state obj_index_key_state_t;
typedef struct obj_index_s obj_index_t;
typedef struct obj_index_seek_point_s obj_index_seek_point_t;
typedef struct obj_index_key_entry_s obj_index_key_entry_t;
typedef struct obj_index_iterator_end_state_s obj_index_iterator_end_state_t;
typedef struct obj_index_iterator_s obj_index_iterator_t;

/* ********** skiplist ********** */

struct obj_skiplist_level_s {
    obj_skiplist_node_t *forward;
};

struct obj_skiplist_node_s {
    /* key */
    obj_bson_t *key;
    /* value */
    obj_record_t *record;
    obj_skiplist_node_t *backward;
    int level;
    obj_skiplist_level_t levels[];
};

struct obj_skiplist_s {
    int level;
    int length;
    obj_skiplist_node_t *head;
    obj_skiplist_node_t *tail;
    obj_index_key_order_t order;
};


/* ********** index ********** */

enum obj_index_key_state {
    OBJ_INDEX_KEY_STATE_VALID,
    OBJ_INDEX_KEY_STATE_MUST_ADVANCE,
    OBJ_INDEX_KEY_STATE_DONE
};

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
    obj_bson_t *key;
    obj_record_t *record;
};

/* end */
struct obj_index_iterator_end_state_s {
    obj_bson_t *key;
    obj_skiplist_node_t *node;
    obj_bool_t inclusive;
};

/* iterator */
struct obj_index_iterator_s {
    obj_skiplist_t *skiplist;
    obj_skiplist_node_t *cur_node;
    obj_index_iterator_end_state_t end_state;
};


#define OBJ_SKIPLIST_P 0.25
#define OBJ_SKIPLIST_MAXLEVEL 32

/* skiplist */
obj_skiplist_t *obj_skiplist_create(obj_index_key_order_t order);
void obj_skiplist_destroy(obj_skiplist_t *skiplist);
void obj_skiplist_insert(obj_skiplist_t *skiplist, obj_bson_t *key, obj_record_t *record);
void obj_skiplist_dump(obj_skiplist_t *skiplist);

/* index */
obj_bson_t *obj_get_index_key(obj_bson_t *object, obj_bson_t *key_pattern);
obj_bool_t obj_index_type_is_valid(obj_bson_type_t bson_type);
obj_bool_t obj_index_key_pattern_is_valid(obj_bson_t *prototype, obj_bson_t *key_pattern, obj_index_key_order_t *order, int *nfields);
obj_index_key_order_t obj_index_key_order_get(obj_bson_t *key_pattern);

/* index iterator */
obj_index_iterator_t *obj_index_iterator_create(obj_index_catalog_entry_t *index_entry);
void obj_index_iterator_set_end_position(obj_index_iterator_t *iter, obj_bson_t *key, obj_bool_t inclusive);
obj_index_key_entry_t obj_index_iterator_seek(obj_index_iterator_t *iter, obj_bson_t *key, obj_bool_t inclusive);
obj_index_key_entry_t obj_index_iterator_seek_with_seek_point(obj_index_iterator_t *iter, obj_index_seek_point_t *seek_point);
obj_index_key_entry_t obj_index_iterator_next(obj_index_iterator_t *iter);



#endif  /* OBJ_SKIPLIST_H */