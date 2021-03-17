#ifndef OBJ_QUERY_INDEX_H
#define OBJ_QUERY_INDEX_H

#include "obj_core.h"

/* about how to index for query */

typedef struct obj_query_index_scan_build_state_s obj_query_index_scan_build_state_t;

struct obj_query_index_scan_build_state_s {
    obj_expr_base_expr_t *root;
    obj_array_t *indexes;
    int cur_child;
    /* plan tree node currently constructing */
    obj_query_plan_tree_base_node_t *current_scan;
    int cur_index;
    obj_expr_index_tag_t *index_tag;
};

void obj_query_index_get_fields(obj_expr_base_expr_t *root, obj_set_t *out);
void obj_query_index_find_relevant_indexes(obj_set_t *fields, obj_array_t *all_indexes, obj_array_t *out);

#endif  /* OBJ_QUERY_INDEX_H */