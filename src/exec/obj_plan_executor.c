#include "obj_core.h"

/* query plan executor */

/* get query plan executor */
obj_status_with_t obj_plan_executor_get_executor(obj_collection_handler_t *collection_handler, ) {
    /* make working set for plann tree */
    obj_plan_working_set_t *ws = obj_plan_working_set_create();
    obj_status_with_t prepare_res = obj_plan_executor_prepare_execution(collection_handler, ws, );
    if (prepare_res.code != OBJ_CODE_OK) {
        return prepare_res;
    }
    
    return ;
}

/* prepare for execution */
obj_status_with_t obj_plan_executor_prepare_execution(obj_collection_handler_t *collection_handler, obj_plan_working_set_t *ws, ) {

}