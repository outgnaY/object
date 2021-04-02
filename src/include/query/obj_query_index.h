#ifndef OBJ_QUERY_INDEX_H
#define OBJ_QUERY_INDEX_H

#include "obj_core.h"

/* about how to index for query */

/* forward declaration */
typedef struct obj_query_plan_tree_base_node_s obj_query_plan_tree_base_node_t;

typedef struct obj_query_index_scan_build_state_s obj_query_index_scan_build_state_t;
typedef struct obj_query_index_ordered_interval_list_s obj_query_index_ordered_interval_list_t;




struct obj_query_index_scan_build_state_s {
    obj_expr_base_expr_t *root;
    obj_array_t *indexes;
    int cur_child;
    /* plan tree node currently constructing */
    obj_query_plan_tree_base_node_t *current_scan;
    int cur_index;
    obj_expr_index_tag_t *index_tag;
    /* obj_expr_base_expr_t *cur_or; */
};


obj_bool_t obj_query_index_expr_can_use_index(obj_expr_base_expr_t *expr);
obj_bool_t obj_query_index_expr_is_bounds_generating(obj_expr_base_expr_t *expr);
void obj_query_index_get_fields(obj_expr_base_expr_t *root, obj_set_t *out);
void obj_query_index_find_relevant_indexes(obj_set_t *fields, obj_array_t *all_indexes, obj_array_t *out);
void obj_query_index_rate_indexes(obj_expr_base_expr_t *root, obj_array_t *indexes);
/* methods for indexed plan building */
obj_query_plan_tree_base_node_t *obj_query_index_build_indexed_data_access(obj_expr_base_expr_t *root, obj_array_t *indexes);



#endif  /* OBJ_QUERY_INDEX_H */