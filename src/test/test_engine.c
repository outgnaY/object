#include "obj_core.h"

/* create context, for test */
obj_conn_context_t *create_context() {
    obj_conn_context_t *context = obj_alloc(sizeof(obj_conn_context_t));
    context->locker = obj_locker_create();
}

int main() {
    obj_conn_context_t *context = create_context();
    obj_global_lock_t global_lock1;
    obj_db_lock_t db_lock1;
    obj_collection_lock_t collection_lock1;
    obj_stringdata_t db_name1 = {"db1", 3};
    obj_stringdata_t collection_name1 = {"db1.coll1", 9};
    obj_global_mem_context_init();
    obj_global_lock_manager_init();
    obj_global_engine_init();
    obj_global_db_manager_init
    /* obj_global_lock_init(&global_lock1, context->locker, OBJ_LOCK_MODE_X); */

    obj_global_lock_manager_destroy();
    obj_global_engine_destroy();
    return 0;
}