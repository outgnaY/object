#ifndef OBJ_EXEC_TREE_H
#define OBJ_EXEC_TREE_H

#include "obj_core.h"

/* exec tree */

typedef enum obj_exec_tree_node_type obj_exec_tree_node_type_t;
typedef enum obj_exec_tree_exec_state obj_exec_tree_exec_state_t;

/* nodes */
typedef struct obj_exec_tree_node_methods_s obj_exec_tree_node_methods_t;
typedef struct obj_exec_tree_base_node_s obj_exec_tree_base_node_t;
typedef struct obj_record_wsid_pair_s obj_record_wsid_pair_t;
typedef struct obj_exec_tree_and_node_s obj_exec_tree_and_node_t;
typedef struct obj_exec_tree_or_node_s obj_exec_tree_or_node_t;
typedef struct obj_exec_tree_collection_scan_node_s obj_exec_tree_collection_scan_node_t;
typedef struct obj_exec_tree_index_scan_node_s obj_exec_tree_index_scan_node_t;
typedef struct obj_exec_tree_sort_node_data_item_s obj_exec_tree_sort_node_data_item_t;
typedef struct obj_exec_tree_sort_node_s obj_exec_tree_sort_node_t;
typedef struct obj_exec_tree_projection_node_s obj_exec_tree_projection_node_t;
typedef struct obj_exec_tree_skip_node_s obj_exec_tree_skip_node_t;
typedef struct obj_exec_tree_limit_node_s obj_exec_tree_limit_node_t;
typedef struct obj_exec_tree_eof_node_s obj_exec_tree_eof_node_t;

/* exec tree node types */
enum obj_exec_tree_node_type {
    OBJ_EXEC_TREE_NODE_TYPE_AND,
    OBJ_EXEC_TREE_NODE_TYPE_OR,
    OBJ_EXEC_TREE_NODE_TYPE_COLLECTION_SCAN,
    OBJ_EXEC_TREE_NODE_TYPE_INDEX_SCAN,
    OBJ_EXEC_TREE_NODE_TYPE_PROJECTION,
    OBJ_EXEC_TREE_NODE_TYPE_SORT,
    OBJ_EXEC_TREE_NODE_TYPE_SKIP,
    OBJ_EXEC_TREE_NODE_TYPE_LIMIT,
    OBJ_EXEC_TREE_NODE_TYPE_EOF,
};

/* execute state */
enum obj_exec_tree_exec_state {
    OBJ_EXEC_TREE_STATE_ADVANCED,
    OBJ_EXEC_TREE_STATE_NEED_TIME,
    OBJ_EXEC_TREE_STATE_EOF,
    OBJ_EXEC_TREE_STATE_INTERNAL_ERROR
};

/* methods of a exec tree node */
struct obj_exec_tree_node_methods_s {
    obj_exec_tree_exec_state_t (*work)(obj_exec_tree_base_node_t *node, obj_exec_working_set_id_t *out);
    obj_exec_tree_node_type_t (*get_type)();
    obj_bool_t (*is_eof)(obj_exec_tree_base_node_t *node);
};

/* base exec tree node */
struct obj_exec_tree_base_node_s {
    obj_exec_tree_node_methods_t *methods;
    obj_array_t children;
};

struct obj_record_wsid_pair_s {
    obj_record_t *record;
    obj_exec_working_set_id_t wsid;
};

/* and node */
struct obj_exec_tree_and_node_s {
    obj_exec_tree_base_node_t base;
    obj_exec_working_set_t *ws;
    int current_child;
    obj_prealloc_map_t data_map;
    obj_set_t seen_map;
    obj_bool_t hashing_children;
    obj_array_t look_ahead_results;
};

/* or node */
struct obj_exec_tree_or_node_s {
    obj_exec_tree_base_node_t base;
    obj_exec_working_set_t *ws;
    int current_child;
    /* keep records we have already seen */
    obj_set_t seen;
};

/* collection scan node */
struct obj_exec_tree_collection_scan_node_s {
    obj_exec_tree_base_node_t base;
    int direction;
    obj_exec_working_set_t *ws;
    obj_expr_base_expr_t *filter;
    obj_collection_catalog_entry_t *collection;
    obj_record_store_iterator_t *iter;
    obj_bool_t end;
};

/* index scan node */
struct obj_exec_tree_index_scan_node_s {
    obj_exec_tree_base_node_t base;

};

/* item to sort */
struct obj_exec_tree_sort_node_data_item_s {
    obj_exec_working_set_id_t ws_id;
    /* key pattern. e.x. {"a": 1, "b": -1} */
    obj_bson_t *pattern; 
    /* record */
    obj_record_t *record;
};

/* sort node */
struct obj_exec_tree_sort_node_s {
    obj_exec_tree_base_node_t base;
    obj_bson_t *pattern;
    obj_exec_working_set_t *ws;
    /* current memory usage of this node */
    /* int mem_usage; */
    /* if the data is already sorted */
    obj_bool_t sorted;
    /* data to be sorted */
    obj_array_t data;
    int curr;
};

/* projection node */
struct obj_exec_tree_projection_node_s {
    obj_exec_tree_base_node_t base;
    obj_exec_working_set_t *ws;
    obj_bson_t *projection;
};

/* skip node */
struct obj_exec_tree_skip_node_s {
    obj_exec_tree_base_node_t base;
    obj_exec_working_set_t *ws;
    int num_to_skip;
};

/* limit node */
struct obj_exec_tree_limit_node_s {
    obj_exec_tree_base_node_t base;
    obj_exec_working_set_t *ws;
    int num_to_return;
};

/* eof node */
struct obj_exec_tree_eof_node_s {
    obj_exec_tree_base_node_t base;
};

#endif  /* OBJ_EXEC_TREE_H */