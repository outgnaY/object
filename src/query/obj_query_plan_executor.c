#include "obj_core.h"


static obj_prepare_execution_result_t obj_prepare_execution(obj_collection_catalog_entry_t *collection, obj_exec_working_set_t *ws, obj_standard_query_t *sq);
static obj_query_plan_executor_t *obj_get_query_plan_executor(obj_collection_catalog_entry_t *collection, obj_standard_query_t *sq);

/* prepare execution */
static obj_prepare_execution_result_t obj_prepare_execution(obj_collection_catalog_entry_t *collection, obj_exec_working_set_t *ws, obj_standard_query_t *sq) {
    obj_assert(collection != NULL);
    obj_prepare_execution_result_t result = {sq, NULL, NULL};
    /* plan tree */
    obj_status_with_t status_with_plan = obj_query_planner_plan(sq, collection);
    if (!obj_status_isok(&status_with_plan)) {
        return result;
    }
    obj_query_plan_tree_base_node_t *plan_tree = (obj_query_plan_tree_base_node_t *)status_with_plan.data;
    result.plan_tree = plan_tree;
    /* exec tree */
    obj_exec_tree_base_node_t *exec_tree = obj_query_plan_tree_build_exec_tree(plan_tree, sq, ws);
    result.exec_tree = exec_tree;
    return result;
}

/* get query plan executor */
static obj_query_plan_executor_t *obj_get_query_plan_executor(obj_collection_catalog_entry_t *collection, obj_standard_query_t *sq) {
    obj_exec_working_set_t *ws = obj_exec_working_set_create();
    obj_prepare_execution_result_t execution_result = obj_prepare_execution(collection, ws, sq);
    if (execution_result.exec_tree == NULL) {
        return NULL;
    }
    obj_query_plan_executor_t *executor = obj_alloc(sizeof(obj_query_plan_executor_t));
    executor->collection = collection;
    executor->sq = sq;
    executor->ws = ws;
    executor->plan_tree = execution_result.plan_tree;
    executor->exec_tree = execution_result.exec_tree;
    return executor;
}


/* get executor for find */
obj_query_plan_executor_t *obj_get_query_plan_executor_find(obj_collection_catalog_entry_t *collection, obj_standard_query_t *sq) {
    return obj_get_query_plan_executor(collection, sq);
}


/* TODO get executor for update */


/* TODO get executor for delete */


/* for find command */
obj_query_plan_executor_exec_state_t obj_query_plan_executor_get_next(obj_query_plan_executor_t *executor, obj_record_t **out) {
    obj_query_plan_executor_exec_state_t state;
    obj_exec_working_set_id_t id = OBJ_EXEC_WORKING_SET_INVALID_ID;
    obj_exec_tree_base_node_t *root = executor->exec_tree;
    obj_exec_tree_exec_state_t code;
    obj_exec_working_set_member_t *member = NULL;
    while (true) {
        code = root->methods->work(root, &id);
        if (code == OBJ_EXEC_TREE_STATE_ADVANCED) {
            member = obj_exec_working_set_get(executor->ws, id);
            if (out) {
                *out = member->record;
            }
            return OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_ADVANCED;
        } else if (code == OBJ_EXEC_TREE_STATE_EOF) {
            return OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_EOF;
        } else if (code == OBJ_EXEC_TREE_STATE_NEED_TIME) {
            /* continue */
        } else {
            /* internal error */
            return OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_INTERNAL_ERROR;
        }
    }
}

/* execute plan, when we don't care about output */
void obj_query_plan_executor_execute_plan(obj_query_plan_executor_t *executor) {
    obj_query_plan_executor_exec_state_t state = OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_ADVANCED;
    obj_record_t *out;
    while (state == OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_ADVANCED) {
        state = obj_query_plan_executor_get_next(executor, &out);
    }
}

