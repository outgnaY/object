#ifndef OBJ_QUERY_PLAN_TREE_H
#define OBJ_QUERY_PLAN_TREE_H

#include "obj_core.h"

/* output by query planner, corresponding to exec tree */

typedef enum obj_query_plan_tree_node_type obj_query_plan_tree_node_type_t;
typedef struct obj_query_plan_tree_node_methods_s obj_query_plan_tree_node_methods_t;
/* base node */
typedef struct obj_query_plan_tree_base_node_s obj_query_plan_tree_base_node_t;
/* concrete node */
typedef struct obj_query_plan_tree_and_node_s obj_query_plan_tree_and_node_t;
typedef struct obj_query_plan_tree_or_node_s obj_query_plan_tree_or_node_t;
typedef struct obj_query_plan_tree_collection_scan_node_s obj_query_plan_tree_collection_scan_node_t;
typedef struct obj_query_plan_tree_index_scan_node_s obj_query_plan_tree_index_scan_node_t;

typedef struct obj_query_plan_tree_projection_node_s obj_query_plan_tree_projection_node_t;
typedef struct obj_query_plan_tree_sort_node_s obj_query_plan_tree_sort_node_t;

typedef struct obj_query_plan_tree_skip_node_s obj_query_plan_tree_skip_node_t;
typedef struct obj_query_plan_tree_limit_node_s obj_query_plan_tree_limit_node_t;

enum obj_query_plan_tree_node_type {
    OBJ_QUERY_PLAN_TREE_NODE_TYPE_AND,
    OBJ_QUERY_PLAN_TREE_NODE_TYPE_OR,
    OBJ_QUERY_PLAN_TREE_NODE_TYPE_COLLECTION_SCAN,
    OBJ_QUERY_PLAN_TREE_NODE_TYPE_INDEX_SCAN,
    OBJ_QUERY_PLAN_TREE_NODE_TYPE_PROJECTION,
    OBJ_QUERY_PLAN_TREE_NODE_TYPE_SORT,
    OBJ_QUERY_PLAN_TREE_NODE_TYPE_SKIP,
    OBJ_QUERY_PLAN_TREE_NODE_TYPE_LIMIT
};

/* methods of query plan tree node */
struct obj_query_plan_tree_node_methods_s {
    obj_query_plan_tree_node_type_t (*get_type)();
    void (*dump)(obj_query_plan_tree_base_node_t *node);
};

struct obj_query_plan_tree_base_node_s {
    obj_query_plan_tree_node_methods_t *methods;
    obj_array_t children;
    obj_bson_t *filter;
};

/* and node */
struct obj_query_plan_tree_and_node_s {
    obj_query_plan_tree_base_node_t base;
};

/* or node */
struct obj_query_plan_tree_or_node_s {
    obj_query_plan_tree_base_node_t base;
};

/* collection scan node */
struct obj_query_plan_tree_collection_scan_node_s {
    obj_query_plan_tree_base_node_t base;
    /* collection scan direction */
    int direction;

};

/* index scan node */
struct obj_query_plan_tree_index_scan_node_s {
    obj_query_plan_tree_base_node_t base;
    /* index scan direction */
    int direction;
    /* index key pattern */
    obj_query_index_entry_t index_entry;
    /* index bounds */
    obj_index_bounds_t bounds;
};


/* projection node */
struct obj_query_plan_tree_projection_node_s {
    obj_query_plan_tree_base_node_t base;

};

/* sort node */
struct obj_query_plan_tree_sort_node_s {
    obj_query_plan_tree_base_node_t base;
    obj_bson_t *pattern;
};



/* skip node */
struct obj_query_plan_tree_skip_node_s {
    obj_query_plan_tree_base_node_t base;
    int skip;
};

/* limit node */
struct obj_query_plan_tree_limit_node_s {
    obj_query_plan_tree_base_node_t base;
    int limit;
};

void obj_query_plan_tree_dump(obj_query_plan_tree_base_node_t *root, int skip);
obj_bool_t obj_query_plan_tree_add_children(obj_query_plan_tree_base_node_t *root, obj_array_t *children);

/* and node */
obj_query_plan_tree_and_node_t *obj_query_plan_tree_and_node_create();
/* or node */
obj_query_plan_tree_or_node_t *obj_query_plan_tree_or_node_create();
/* collection scan node */
obj_query_plan_tree_collection_scan_node_t *obj_query_plan_tree_collection_scan_node_create();
/* index scan node */
obj_query_plan_tree_index_scan_node_t *obj_query_plan_tree_index_scan_node_create(obj_query_index_entry_t *index_entry);
/* projection node */
obj_query_plan_tree_projection_node_t *obj_query_plan_tree_projection_node_create();
/* sort node */
obj_query_plan_tree_sort_node_t *obj_query_plan_tree_sort_node_create();
/* skip node */
obj_query_plan_tree_skip_node_t *obj_query_plan_tree_skip_node_create();
/* limit node */
obj_query_plan_tree_limit_node_t *obj_query_plan_tree_limit_node_create();

#endif  /* OBJ_QUERY_PLAN_TREE_H */