#ifndef OBJ_PLAN_TREE_H
#define OBJ_PLAN_TREE_H

#include "obj_core.h"

/* execute query plan tree */

typedef enum obj_plan_tree_node_type obj_plan_tree_node_type_t;
typedef enum obj_plan_tree_exec_state obj_plan_tree_exec_state_t;

/* nodes */
typedef struct obj_plan_tree_node_methods_s obj_plan_tree_node_methods_t;
typedef struct obj_plan_tree_base_node_s obj_plan_tree_base_node_t;
typedef struct obj_plan_tree_eof_node_s obj_plan_tree_eof_node_t;
/*
typedef struct obj_plan_tree_fetch_node_s obj_plan_tree_fetch_node_t;
*/
typedef enum obj_collection_scan_direction obj_collection_scan_direction_t;
typedef struct obj_plan_tree_collection_scan_params_s obj_plan_tree_collection_scan_params_t;
typedef struct obj_plan_tree_collection_scan_node_s obj_plan_tree_collection_scan_node_t;
typedef struct obj_plan_tree_index_scan_node_s obj_plan_tree_index_scan_node_t;
typedef struct obj_plan_tree_sort_params_s obj_plan_tree_sort_params_t;
typedef struct obj_plan_tree_sort_node_data_item_s obj_plan_tree_sort_node_data_item_t;
typedef struct obj_plan_tree_sort_node_s obj_plan_tree_sort_node_t;
typedef struct obj_plan_tree_projection_node_s obj_plan_tree_projection_node_t;
typedef struct obj_plan_tree_skip_node_s obj_plan_tree_skip_node_t;
typedef struct obj_plan_tree_limit_node_s obj_plan_tree_limit_node_t;


/* plan tree node types */
enum obj_plan_tree_node_type {
    OBJ_PLAN_TREE_NODE_TYPE_EOF,
    OBJ_PLAN_TREE_NODE_TYPE_FETCH,
    OBJ_PLAN_TREE_NODE_TYPE_COLLECTION_SCAN,
    OBJ_PLAN_TREE_NODE_TYPE_INDEX_SCAN,
    OBJ_PLAN_TREE_NODE_TYPE_SORT,
    OBJ_PLAN_TREE_NODE_TYPE_PROJECTION,
    OBJ_PLAN_TREE_NODE_TYPE_SKIP,
    OBJ_PLAN_TREE_NODE_TYPE_LIMIT
};

/* execute state */
enum obj_plan_tree_exec_state {
    OBJ_PLAN_TREE_STATE_ADVANCED,
    OBJ_PLAN_TREE_STATE_NEED_TIME,
    OBJ_PLAN_TREE_STATE_EOF,
    OBJ_PLAN_TREE_STATE_INTERNAL_ERROR
};

/* methods of a plan tree node */
struct obj_plan_tree_node_methods_s {
    obj_plan_tree_exec_state_t (*work)(obj_plan_tree_base_node_t *node, obj_plan_working_set_id_t *out);
    obj_plan_tree_node_type_t (*get_type)();
    obj_bool_t (*is_eof)(obj_plan_tree_base_node_t *node);
};

/* base plan tree node */
struct obj_plan_tree_base_node_s {
    obj_plan_tree_node_methods_t *methods;
    obj_array_t children;
};

/* eof node */
struct obj_plan_tree_eof_node_s {
    obj_plan_tree_base_node_t base;

};

/* fetch node */
/*
struct obj_plan_tree_fetch_node_s {
    obj_plan_tree_base_node_t base;
    obj_plan_working_set_t *ws;
};
*/
/* collection scan direction */
enum obj_collection_scan_direction {
    COLLECTION_SCAN_DIRECTION_FORWARD = 1,
    COLLECTION_SCAN_DIRECTION_BACKWARD = -1
};

/* params of collection scan node */
struct obj_plan_tree_collection_scan_params_s {
    /* scan direction */
    obj_collection_scan_direction_t direction;
};

/* collection scan node */
struct obj_plan_tree_collection_scan_node_s {
    obj_plan_tree_base_node_t base;
    obj_plan_tree_collection_scan_params_t params;
    obj_plan_working_set_t *ws;
    obj_expr_base_t *filter;
};

/* index scan node */
struct obj_plan_tree_index_scan_node_s {
    obj_plan_tree_base_node_t base;

};

/* params of sort node */
struct obj_plan_tree_sort_params_s {
    obj_bson_t *pattern;
};

/* item to sort */
struct obj_plan_tree_sort_node_data_item_s {
    obj_plan_working_set_id_t ws_id;
    /* key pattern. e.x. {"a": 1, "b": -1} */
    obj_bson_t *pattern; 
    /* sort key */
    obj_bson_t *sort_key;
    /* record */

};

/* sort node */
struct obj_plan_tree_sort_node_s {
    obj_plan_tree_base_node_t base;
    obj_plan_tree_sort_params_t params;
    obj_plan_working_set_t *ws;
    /* current memory usage of this node */
    int mem_usage; 
    obj_bool_t sorted;
    /* data to be sorted */
    obj_array_t data;
    /* current position */
    int curr;
};

/* projection node */
struct obj_plan_tree_projection_node_s {
    obj_plan_tree_base_node_t base;

};

/* skip node */
struct obj_plan_tree_skip_node_s {
    obj_plan_tree_base_node_t base;
    obj_plan_working_set_t *ws;
    
    int num_to_skip;
};

/* limit node */
struct obj_plan_tree_limit_node_s {
    obj_plan_tree_base_node_t base;
    obj_plan_working_set_t *ws;
    
    int num_to_return;
};

#endif  /* OBJ_PLAN_TREE_H */