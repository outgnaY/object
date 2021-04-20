#include "obj_core.h"

static int obj_get_command_processor(char *command_name);
static void obj_process_insert_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_delete_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_update_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_find_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_create_index_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_delete_index_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_create_collection_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_delete_collection_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_create_database_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_delete_database_command(obj_conn_t *c, obj_bson_t *command_bson);



static obj_command_processor_pair_t obj_command_processors[10] = {
    {"insert", obj_process_insert_command},
    {"delete", obj_process_delete_command},
    {"update", obj_process_update_command},
    {"find", obj_process_find_command},
    {"create", obj_process_create_index_command},
    {"delete", obj_process_delete_index_command},
    {"create_collection", obj_process_create_collection_command},
    {"delete_collection", obj_process_delete_collection_command},
    {"create_database", obj_process_create_database_command},
    {"delete_database", obj_process_delete_database_command}
};

/* get command processor */
static int obj_get_command_processor(char *command_name) {
    int lo = 0;
    int hi = OBJ_NELEM(obj_command_processors) - 1;
    int mid;
    int res;
    while (lo < hi) {
        mid = (lo + hi) / 2;
        res = obj_strcmp(command_name, obj_command_processors[mid].name);
        if (res < 0) {
            lo = mid + 1;
        } else if (res > 0) {
            hi = mid - 1;
        } else {
            return mid;
        }
    }
    return -1;
}

void obj_process_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        if (bson_type != OBJ_BSON_TYPE_UTF8)
        /* TODO error reply */
        
        return;
    }
    int index = obj_get_command_processor(key);
    if (index < 0) {
        /* TODO error reply */
        return;
    }
    /* process command */
    obj_command_processors[index].processor(c, command_bson);
}

/* command processors */

/** 
 * insert command
 * {
 *     "insert": <string>
 *     "objects": [<object>]
 * }
 */
static void obj_process_insert_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    char *full_name = value->value.v_utf8.str;
    int full_name_len = value->value.v_utf8.len;

    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        /* TODO reply error */
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_ARRAY) {
        /* TODO reply error */
        return;
    }
    value = obj_bson_iter_value(&iter);
    obj_bson_t objects;
    obj_bson_init_static_with_len(&objects, value->value.v_array.data, value->value.v_array.len);
    obj_bson_iter_init(&iter, &objects);
    
    obj_db_catalog_entry_t *db_entry = NULL;
    obj_collection_catalog_entry_t *collection_entry = NULL;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    obj_collection_lock_t collection_lock;
    char *pos = obj_strchr(full_name, '.');
    if (pos == NULL || (pos - full_name) == full_name_len - 1) {
        /* TODO reply error */
        return;
    }
    *pos = '\0';
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init_with_len(&db_lock, c->context->locker, OBJ_LOCK_MODE_IX, full_name, pos - full_name);
    *pos = '.';
    obj_collection_lock_init_with_len(&collection_lock, c->context->locker, OBJ_LOCK_MODE_X, full_name, full_name_len);
    *pos = '\0';
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    obj_collection_lock_lock(&collection_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, full_name);
    if (db_entry == NULL) {
        /* TODO reply error */
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry == NULL) {
        /* TODO collection not found */
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    /* check type */
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        if (bson_type != OBJ_BSON_TYPE_OBJECT) {
            /* TODO reply error */
            obj_collection_lock_unlock(&collection_lock);
            obj_db_lock_unlock(&db_lock);
            obj_global_lock_unlock(&global_lock);
            return;
        }
        value = obj_bson_iter_value(&iter);
        obj_bson_t object;
        obj_bson_init_static_with_len(&object, value->value.v_object.data, value->value.v_object.len);
        if (!obj_type_checker_check_type(&collection_entry->checker, &object)) {
            /* TODO reply error */
            obj_collection_lock_unlock(&collection_lock);
            obj_db_lock_unlock(&db_lock);
            obj_global_lock_unlock(&global_lock);
            return;
        }
    }
    /* insert */
    obj_insert_objects(collection_entry, &objects);
    obj_collection_lock_unlock(&collection_lock);
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    /* TODO reply */ 

}

/** 
 * delete command
 * {
 *     "delete": <string>
 * 
 */
static void obj_process_delete_command(obj_conn_t *c, obj_bson_t *command_bson) {

}

/** 
 * update command
 * {
 *     "update": <string>
 * 
 * }
 */
static void obj_process_update_command(obj_conn_t *c, obj_bson_t *command_bson) {

}

/** 
 * find command
 * {
 *     "find": <string>
 *     "filter": <object>
 * }
 */
static void obj_process_find_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_status_with_t status_with_qr = obj_query_parse_from_find_cmd(command_bson);
    /* parse error */
    if (status_with_qr.data != 0) {

    }
    obj_status_with_t status_with_sq = obj_query_standardize((obj_query_request_t *)status_with_qr.data);
    obj_standard_query_t *sq = (obj_standard_query_t *)status_with_sq.data;
    obj_collection_catalog_entry_t *collection = NULL;
    /* get collection */

    obj_query_plan_executor_t *executor = obj_get_query_plan_executor_find(collection, sq);
    obj_query_plan_executor_exec_state_t state = OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_ADVANCED;
    obj_record_t *out;
    while ((state = obj_query_plan_executor_get_next(executor, &out)) == OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_ADVANCED) {
        
    }
    
}

/**
 * create index command
 * {
 *     "create_index": <string>
 *     "name": <string>
 *     "pattern": <object>
 * }
 */
static void obj_process_create_index_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    char *full_name = value->value.v_utf8.str;
    int full_name_len = value->value.v_utf8.len;

    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        /* TODO reply error */
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_UTF8) {
        /* TODO reply error */
        return;
    }
    value = obj_bson_iter_value(&iter);
    char *index_name = value->value.v_utf8.str;
    int index_name_len = value->value.v_utf8.len;
    /* copy index name */
    char *index_name_copy = obj_alloc(index_name_len + 1);
    obj_memcpy(index_name_copy, index_name, index_name_len);
    index_name_copy[index_name_len] = '\0';

    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        /* TODO reply error */
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_OBJECT) {
        /* TODO reply error */
        return;
    }
    value = obj_bson_iter_value(&iter);
    obj_bson_t key_pattern;
    obj_bson_t *key_pattern_copy;    
    obj_bson_init_static_with_len(&key_pattern, value->value.v_object.data, value->value.v_object.len);

    obj_db_catalog_entry_t *db_entry = NULL;
    obj_collection_catalog_entry_t *collection_entry = NULL;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    obj_collection_lock_t collection_lock;
    char *pos = obj_strchr(full_name, '.');
    if (pos == NULL || (pos - full_name) == full_name_len - 1) {
        /* TODO reply error */
        return;
    }
    *pos = '\0';

    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init_with_len(&db_lock, c->context->locker, OBJ_LOCK_MODE_IX, full_name, pos - full_name);
    *pos = '.';
    obj_collection_lock_init_with_len(&collection_lock, c->context->locker, OBJ_LOCK_MODE_X, full_name, full_name_len);
    *pos = '\0';
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    obj_collection_lock_lock(&collection_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, full_name);
    if (db_entry == NULL) {
        /* TODO reply error */
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry == NULL) {
        /* TODO collection not found */
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }

    obj_uint8_t *data_copy = obj_alloc(value->value.v_object.len);
    obj_memcpy(data_copy, value->value.v_object.data, value->value.v_object.len);
    key_pattern_copy = obj_bson_create_with_data(data_copy, value->value.v_object.len);
    obj_status_t res = obj_create_index(collection_entry, key_pattern_copy, index_name_copy);
    if (!obj_status_isok(&res)) {
        /* TODO reply error */
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    obj_collection_lock_unlock(&collection_lock);
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    /* TODO reply */
    
}

/**
 * delete index command
 * {
 *     "delete_index": <string>
 *     "name": <string>
 * }
 */
static void obj_process_delete_index_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    char *full_name = value->value.v_utf8.str;
    int full_name_len = value->value.v_utf8.len;

    char *index_name;
    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        /* TODO reply error */
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_UTF8) {
        /* TODO reply error */
        return;
    }
    value = obj_bson_iter_value(&iter);
    index_name = value->value.v_utf8.str;
    obj_db_catalog_entry_t *db_entry = NULL;
    obj_collection_catalog_entry_t *collection_entry = NULL;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    obj_collection_lock_t collection_lock;
    char *pos = obj_strchr(full_name, '.');
    if (pos == NULL || (pos - full_name) == full_name_len - 1) {
        /* TODO reply error */
        return;
    }
    *pos = '\0';
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init_with_len(&db_lock, c->context->locker, OBJ_LOCK_MODE_IX, full_name, pos - full_name);
    *pos = '.';
    obj_collection_lock_init_with_len(&collection_lock, c->context->locker, OBJ_LOCK_MODE_X, full_name, full_name_len);
    *pos = '\0';
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    obj_collection_lock_lock(&collection_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, full_name);
    if (db_entry == NULL) {
        /* TODO db not found */
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry == NULL) {
        /* TODO collection not found */
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    /* delete index */
    if (!obj_delete_index(collection_entry, index_name)) {
        /* TODO reply error */
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }

    obj_collection_lock_unlock(&collection_lock);
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    /* TODO reply */

}

/**
 * create collection command
 * {
 *     "create_collection": <string>
 *     "prototype": <object>
 * }
 */
static void obj_process_create_collection_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    char *full_name = value->value.v_utf8.str;
    int full_name_len = value->value.v_utf8.len;
    char *full_name_copy = (char *)obj_alloc(full_name_len + 1);
    obj_memcpy(full_name_copy, full_name, full_name_len);
    full_name_copy[full_name_len] = '\0';

    obj_bson_t prototype;
    obj_bson_t *prototype_copy = NULL;
    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        /* TODO reply error */
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_OBJECT) {
        /* TODO reply error */
        return;
    }
    value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_OBJECT);
    obj_bson_init_static_with_len(&prototype, value->value.v_object.data, value->value.v_object.len);
    if (!obj_check_type_define(&prototype)) {
        /* TODO reply error */
        return;
    }
    obj_uint8_t *data_copy = obj_alloc(value->value.v_object.len);
    obj_memcpy(data_copy, value->value.v_object.data, value->value.v_object.len);
    prototype_copy = obj_bson_create_with_data(data_copy, value->value.v_object.len);
    obj_db_catalog_entry_t *db_entry = NULL;
    obj_collection_catalog_entry_t *collection_entry = NULL;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    
    char *pos = obj_strchr(full_name, '.');
    if (pos == NULL || (pos - full_name) == full_name_len - 1) {
        /* TODO reply error */
        return;
    }
    /* db.col->db'\0'col */
    *pos = '\0';
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init_with_len(&db_lock, c->context->locker, OBJ_LOCK_MODE_X, full_name, pos - full_name);
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, full_name);
    if (db_entry == NULL) {
        /* TODO db not found */
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry != NULL) {
        /* TODO collection already exists */
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    /* create collection */
    if (!obj_create_collection(db_entry, full_name_copy, prototype_copy)) {
        /* TODO reply error */
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    /* TODO reply */

}

/**
 * delete collection command
 * {
 *     "delete_collection": <string>
 * }
 */
static void obj_process_delete_collection_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    obj_db_catalog_entry_t *db_entry = NULL;
    obj_collection_catalog_entry_t *collection_entry = NULL;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    char *full_name = value->value.v_utf8.str;
    int full_name_len = value->value.v_utf8.len;
    char *pos = obj_strchr(full_name, '.');
    if (pos == NULL || (pos - full_name) == full_name_len - 1) {
        /* TODO reply error */
        return;
    }
    /* db.col->db'\0'col */
    *pos = '\0';
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init_with_len(&db_lock, c->context->locker, OBJ_LOCK_MODE_X, full_name, pos - full_name);
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, full_name);
    if (db_entry == NULL) {
        /* TODO db not found */
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    /* restore */
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry == NULL) {
        /* TODO collection not found */
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    if (!obj_delete_collection(db_entry, pos + 1)) {
        /* TODO reply error */
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        return;
    }
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    /* TODO reply */

}

/**
 * create database command
 * {
 *     "create_database": <string>
 * }
 */
static void obj_process_create_database_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    obj_db_catalog_entry_t *db_entry = NULL;
    obj_global_lock_t global_lock;
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_X);
    obj_global_lock_lock(&global_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, value->value.v_utf8.str);
    if (db_entry != NULL) {
        /* TODO db already exists */
        obj_global_lock_unlock(&global_lock);
        return;
    }
    char *db_name = (char *)obj_alloc(value->value.v_utf8.len + 1);
    obj_memcpy(db_name, value->value.v_utf8.str, value->value.v_utf8.len);
    db_name[value->value.v_utf8.len] = '\0';
    obj_create_db(g_engine, db_name);
    obj_global_lock_unlock(&global_lock);
    /* TODO reply */

}

/**
 * delete database command
 * {
 *     "delete_database": <string>
 * }
 */
static void obj_process_delete_database_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    obj_db_catalog_entry_t *db_entry = NULL;
    obj_global_lock_t global_lock;
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_X);
    obj_global_lock_lock(&global_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, value->value.v_utf8.str);
    if (db_entry == NULL) {
        /* TODO db not found */

        obj_global_lock_unlock(&global_lock);
        return;
    }
    /* delete database */
    obj_delete_db(g_engine, value->value.v_utf8.str);
    obj_global_lock_unlock(&global_lock);
    /* TODO reply */

}