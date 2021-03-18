#ifndef OBJ_QUERY_INDEX_H
#define OBJ_QUERY_INDEX_H

#include "obj_core.h"

/* about how to index for query */

typedef struct obj_query_index_entry_s obj_query_index_entry_t;
typedef struct obj_query_index_scan_build_state_s obj_query_index_scan_build_state_t;
typedef struct obj_query_index_ordered_interval_list_s obj_query_index_ordered_interval_list_t;




struct obj_query_index_entry_s {
    int nfields;
    obj_bson_t *key_pattern;
};


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

void obj_query_index_get_fields(obj_expr_base_expr_t *root, obj_set_t *out);
void obj_query_index_find_relevant_indexes(obj_set_t *fields, obj_array_t *all_indexes, obj_array_t *out);

#endif  /* OBJ_QUERY_INDEX_H */