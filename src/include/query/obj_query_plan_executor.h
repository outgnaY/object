#ifndef OBJ_QUERY_PLAN_EXECUTOR_H
#define OBJ_QUERY_PLAN_EXECUTOR_H

#include "obj_core.h"

typedef enum obj_query_plan_executor_exec_state obj_query_plan_executor_exec_state_t;
typedef struct obj_prepare_execution_result_s obj_prepare_execution_result_t;
typedef struct obj_query_plan_executor_s obj_query_plan_executor_t;

enum obj_query_plan_executor_exec_state {
    OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_ADVANCED,
    OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_EOF,
    OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_INTERNAL_ERROR
};

/* prepare execution */
struct obj_prepare_execution_result_s {
    obj_standard_query_t *sq;
    obj_query_plan_tree_base_node_t *plan_tree;
    obj_exec_tree_base_node_t *exec_tree;
};


/* plan executor */
struct obj_query_plan_executor_s {
    obj_collection_catalog_entry_t *collection;
    obj_standard_query_t *sq;
    obj_exec_working_set_t *ws;
    obj_query_plan_tree_base_node_t *plan_tree;
    obj_exec_tree_base_node_t *exec_tree;
};


/* find */
obj_query_plan_executor_t *obj_get_query_plan_executor_find(obj_collection_catalog_entry_t *collection, obj_standard_query_t *sq);

obj_query_plan_executor_exec_state_t obj_query_plan_executor_get_next(obj_query_plan_executor_t *executor, obj_record_t **out);
void obj_query_plan_executor_execute_plan(obj_query_plan_executor_t *executor);

#endif  /* OBJ_QUERY_PLAN_EXECUTOR_H */