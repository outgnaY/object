#include "obj_core.h"

/* create database */
void obj_cmd_create_database(obj_conn_context_t *context, char *name) {
    /*
    obj_global_lock_t global_lock;
    
    obj_global_lock_init(&global_lock, context->locker, OBJ_LOCK_MODE_IX);
    obj_global_lock_lock(&global_lock);

    obj_global_lock_unlock(&global_lock);
    */
}

/* drop database */
void obj_cmd_drop_database(obj_conn_context_t *context, char *name) {
    /*
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    obj_global_lock_init(&global_lock, context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init(&db_lock, context->locker, OBJ_LOCK_MODE_X, db);
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);

    obj_global_lock_unlock(&global_lock);
    */
}

/* create collection */
void obj_cmd_create_collection(obj_conn_context_t *context, char *name) {
    
}

/* drop collection */
void obj_cmd_drop_collection(obj_conn_context_t *context, char *name) {
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    obj_namespace_string_t nss;
    /* TODO get namespace string */
    obj_stringdata_t db_name = obj_namespace_string_get_db(&nss);
    /* require IX global lock and X database lock */
    obj_global_lock_init(&global_lock, context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init(&db_lock, context->locker, OBJ_LOCK_MODE_X, &db_name);
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    /* drop collection */

    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
}

