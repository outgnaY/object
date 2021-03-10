#include "obj_core.h"

/* create context, for test */
obj_conn_context_t *create_context() {
    obj_conn_context_t *context = obj_alloc(sizeof(obj_conn_context_t));
    context->locker = obj_locker_create();
    return context;
}

void *fn1(void *arg) {
    obj_conn_context_t *context = (obj_conn_context_t *)arg;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    obj_stringdata_t db_name = {"db1", 3};
    obj_bool_t just_create;
    int i;
    obj_global_lock_init(&global_lock, context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init(&db_lock, context->locker, OBJ_LOCK_MODE_X, &db_name);
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    obj_status_with_t status_with1 = obj_db_manager_open_db_create_if_not_exists(context, g_db_manager, &db_name, &just_create);
    printf("open db if not exists %s, result %d, just create %d\n", db_name.data, status_with1.code, just_create);
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
}

void *fn2(void *arg) {
    obj_conn_context_t *context = (obj_conn_context_t *)arg;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    obj_stringdata_t db_name = {"db1", 3};
    obj_bool_t just_create;
    int i;
    obj_global_lock_init(&global_lock, context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init(&db_lock, context->locker, OBJ_LOCK_MODE_X, &db_name);
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    obj_status_with_t status_with1 = obj_db_manager_open_db_create_if_not_exists(context, g_db_manager, &db_name, &just_create);
    printf("open db if not exists %s, result %d, just create %d\n", db_name.data, status_with1.code, just_create);
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
}

int main() {
    obj_global_mem_context_init();
    obj_global_lock_manager_init();
    obj_global_engine_init();
    obj_global_db_manager_init();
    obj_conn_context_t *context1 = create_context();
    obj_conn_context_t *context2 = create_context();
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, fn1, context1);
    pthread_create(&tid2, NULL, fn2, context2);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    obj_db_manager_dump(g_db_manager);

    obj_global_lock_manager_destroy();
    obj_global_engine_destroy();
    obj_global_db_manager_destroy();

    return 0;
}

/*
int main() {
    obj_db_handler_t *db_handler;
    obj_global_mem_context_init();
    obj_global_lock_manager_init();
    obj_global_engine_init();
    obj_global_db_manager_init();
    obj_conn_context_t *context = create_context();
    obj_global_lock_t global_lock1;
    obj_db_lock_t db_lock1;
    obj_collection_lock_t collection_lock1;
    obj_stringdata_t db_name1 = {"db1", 3};
    obj_stringdata_t db_name2 = {"db2", 3};
    obj_stringdata_t collection_name1 = {"db1.coll1", 9};

    obj_global_lock_init(&global_lock1, context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init(&db_lock1, context->locker, OBJ_LOCK_MODE_X, &db_name1);
    obj_global_lock_lock(&global_lock1);
    obj_db_lock_lock(&db_lock1);
    
    obj_status_with_t status_with1 = obj_db_manager_open_db(context, g_db_manager, &db_name1);
    printf("open db %s, result %d\n", db_name1.data, status_with1.code);
    obj_bool_t just_create;
    obj_status_with_t status_with2 = obj_db_manager_open_db_create_if_not_exists(context, g_db_manager, &db_name1, &just_create);
    printf("open db if not exists %s, result %d, just create %d\n", db_name1.data, status_with2.code, just_create);
    obj_db_manager_dump(g_db_manager);
    obj_status_t status1 = obj_db_manager_close_db(context, g_db_manager, &db_name2);
    printf("close db %s, result %d\n", db_name2.data, status1.code);
    obj_status_t status2 = obj_db_manager_close_db(context, g_db_manager, &db_name1);
    printf("close db %s, result %d\n", db_name1.data, status2.code);
    obj_db_manager_dump(g_db_manager);

    obj_db_lock_unlock(&db_lock1);
    obj_global_lock_unlock(&global_lock1);

    obj_global_lock_manager_destroy();
    obj_global_engine_destroy();
    obj_global_db_manager_destroy();

    return 0;
}
*/