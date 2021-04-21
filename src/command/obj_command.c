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
static void obj_process_list_databases_command(obj_conn_t *c, obj_bson_t *command_bson);
static void obj_process_list_collections_command(obj_conn_t *c, obj_bson_t *command_bson);


static obj_command_processor_pair_t obj_command_processors[] = {
    {"create_collection", obj_process_create_collection_command},
    {"create_database", obj_process_create_database_command},
    {"create_index", obj_process_create_index_command},
    {"delete", obj_process_delete_command},
    {"delete_collection", obj_process_delete_collection_command},
    {"delete_database", obj_process_delete_database_command},
    {"delete_index", obj_process_delete_index_command},
    {"find", obj_process_find_command},
    {"insert", obj_process_insert_command},
    {"list_collections", obj_process_list_collections_command},
    {"list_databases", obj_process_list_databases_command},
    {"update", obj_process_update_command}
};

/* get command processor */
static int obj_get_command_processor(char *command_name) {
    int lo = 0;
    int hi = OBJ_NELEM(obj_command_processors) - 1;
    int mid;
    int res;
    while (lo <= hi) {
        mid = (lo + hi) / 2;
        res = obj_strcmp(command_name, obj_command_processors[mid].name);
        if (res < 0) {
            hi = mid - 1;
        } else if (res > 0) {
            lo = mid + 1;
        } else {
            return mid;
        }
    }
    return -1;
}

void obj_process_command(obj_conn_t *c, obj_bson_t *command_bson) {
    printf("process command\n");
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {        
        /* TODO reply error */
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_UTF8) {
        /* TODO reply error */
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
    printf("create index\n");
    obj_bson_t *reply = obj_bson_create();
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
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_COMMAND_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_UTF8) {
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_COMMAND_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    value = obj_bson_iter_value(&iter);
    char *index_name = value->value.v_utf8.str;
    int index_name_len = value->value.v_utf8.len;

    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_COMMAND_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_OBJECT) {
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_COMMAND_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
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
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    /* copy index name */
    char *index_name_copy = obj_alloc(index_name_len + 1);
    obj_memcpy(index_name_copy, index_name, index_name_len);
    index_name_copy[index_name_len] = '\0';
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
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_NOT_EXISTS);
        goto clean;
    }
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry == NULL) {
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_ALREADY_EXISTS);
        goto clean;
    }

    obj_uint8_t *data_copy = obj_alloc(value->value.v_object.len);
    obj_memcpy(data_copy, value->value.v_object.data, value->value.v_object.len);
    key_pattern_copy = obj_bson_create_with_data(data_copy, value->value.v_object.len);
    obj_status_t res = obj_create_index(collection_entry, key_pattern_copy, index_name_copy);
    if (!obj_status_is_ok(&res)) {
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, res.code);
        goto clean;
    }
    obj_collection_lock_unlock(&collection_lock);
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    obj_bson_append_int32(reply, "code", 4, OBJ_CODE_OK);
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
    return;
clean:
    obj_conn_add_reply(c, reply);
    obj_free(index_name_copy);
}

/**
 * delete index command
 * {
 *     "delete_index": <string>
 *     "name": <string>
 * }
 */
static void obj_process_delete_index_command(obj_conn_t *c, obj_bson_t *command_bson) {
    printf("delete index\n");
    obj_bson_t *reply = obj_bson_create();
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
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_COMMAND_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_UTF8) {
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_COMMAND_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
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
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
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
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_NOT_EXISTS);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry == NULL) {
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_ALREADY_EXISTS);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    /* delete index */
    if (!obj_delete_index(collection_entry, index_name)) {
        obj_collection_lock_unlock(&collection_lock);
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_INDEX_NOT_FOUND);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }

    obj_collection_lock_unlock(&collection_lock);
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    obj_bson_append_int32(reply, "code", 4, OBJ_CODE_OK);
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
}

/**
 * create collection command
 * {
 *     "create_collection": <string>
 *     "prototype": <object>
 * }
 */
static void obj_process_create_collection_command(obj_conn_t *c, obj_bson_t *command_bson) {
    printf("create collection\n");
    obj_bson_t *reply = obj_bson_create();
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    char *full_name = value->value.v_utf8.str;
    int full_name_len = value->value.v_utf8.len;

    obj_bson_t prototype;
    obj_bson_t *prototype_copy = NULL;
    if (!obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_COMMAND_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    if (bson_type != OBJ_BSON_TYPE_OBJECT) {
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_COMMAND_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_OBJECT);
    obj_bson_init_static_with_len(&prototype, value->value.v_object.data, value->value.v_object.len);
    if (!obj_check_type_define(&prototype)) {
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_TYPE_DEFINE_INVALID);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    obj_db_catalog_entry_t *db_entry = NULL;
    obj_collection_catalog_entry_t *collection_entry = NULL;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    
    char *pos = obj_strchr(full_name, '.');
    if (pos == NULL || (pos - full_name) == full_name_len - 1) {
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    char *full_name_copy = (char *)obj_alloc(full_name_len + 1);
    obj_memcpy(full_name_copy, full_name, full_name_len);
    full_name_copy[full_name_len] = '\0';
    obj_uint8_t *data_copy = obj_alloc(value->value.v_object.len);
    obj_memcpy(data_copy, value->value.v_object.data, value->value.v_object.len);
    prototype_copy = obj_bson_create_with_data(data_copy, value->value.v_object.len);
    /* db.col->db'\0'col */
    *pos = '\0';
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init_with_len(&db_lock, c->context->locker, OBJ_LOCK_MODE_X, full_name, pos - full_name);
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, full_name);
    if (db_entry == NULL) {
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_NOT_EXISTS);
        goto clean;
    }
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry != NULL) {
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_ALREADY_EXISTS);
        goto clean;
    }
    /* create collection */
    if (!obj_create_collection(db_entry, full_name_copy, prototype_copy)) {
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_ALREADY_EXISTS);
        goto clean;
    }
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    obj_bson_append_int32(reply, "code", 4, OBJ_CODE_OK);
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
    return;
clean:
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
    obj_free(full_name_copy);
    obj_bson_destroy(prototype_copy);
    obj_free(data_copy);
}

/**
 * delete collection command
 * {
 *     "delete_collection": <string>
 * }
 */
static void obj_process_delete_collection_command(obj_conn_t *c, obj_bson_t *command_bson) {
    printf("delete collection\n");
    obj_bson_t *reply = obj_bson_create();
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
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_WRONG_FORMAT);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
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
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_NOT_EXISTS);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    /* restore */
    *pos = '.';
    collection_entry = obj_collection_catalog_entry_get(db_entry, full_name);
    if (collection_entry == NULL) {
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_NOT_EXISTS);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    if (!obj_delete_collection(db_entry, full_name)) {
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_COLLECTION_NOT_EXISTS);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    obj_bson_append_int32(reply, "code", 4, OBJ_CODE_OK);
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
}

/**
 * create database command
 * {
 *     "create_database": <string>
 * }
 */
static void obj_process_create_database_command(obj_conn_t *c, obj_bson_t *command_bson) {
    printf("create database\n");
    obj_bson_t *reply = obj_bson_create();
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
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_ALREADY_EXISTS);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    char *db_name = (char *)obj_alloc(value->value.v_utf8.len + 1);
    obj_memcpy(db_name, value->value.v_utf8.str, value->value.v_utf8.len);
    db_name[value->value.v_utf8.len] = '\0';
    obj_create_db(g_engine, db_name);
    obj_global_lock_unlock(&global_lock);
    obj_bson_append_int32(reply, "code", 4, OBJ_CODE_OK);
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
}

/**
 * delete database command
 * {
 *     "delete_database": <string>
 * }
 */
static void obj_process_delete_database_command(obj_conn_t *c, obj_bson_t *command_bson) {
    obj_bson_t *reply = obj_bson_create();
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
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_NOT_EXISTS);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    /* delete database */
    obj_delete_db(g_engine, value->value.v_utf8.str);
    obj_global_lock_unlock(&global_lock);
    obj_bson_append_int32(reply, "code", 4, OBJ_CODE_OK);
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
}

/**
 * list databases command
 * {
 *     "list_databases": ""
 * }
 */
static void obj_process_list_databases_command(obj_conn_t *c, obj_bson_t *command_bson) {
    printf("list databases\n");
    obj_bson_t *reply = obj_bson_create();
    obj_bson_t array;
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    /* obj_bson_append_int32(reply, "code", 4, 0); */
    obj_bson_append_array_begin(reply, "data", 4, &array);
    int i;
    obj_prealloc_map_entry_t *entry = NULL;
    char *db_name;
    obj_global_lock_t global_lock;
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_S);
    obj_global_lock_lock(&global_lock);
    for (i = 0; i < g_engine->map.bucket_size; i++) {
        entry = g_engine->map.bucket[i];
        while (entry != NULL) {
            db_name = *(char **)obj_prealloc_map_get_key(&g_engine->map, entry);
            obj_bson_append_utf8(&array, "", 0, db_name, obj_strlen(db_name));
            entry = entry->next;
        }
    }
    obj_global_lock_unlock(&global_lock);
    obj_bson_append_array_end(reply, &array);
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
}

/**
 * list collections command
 * {
 *     "list_collections": <string>
 * }
 */
static void obj_process_list_collections_command(obj_conn_t *c, obj_bson_t *command_bson) {
    printf("list collections\n");
    obj_bson_t *reply = obj_bson_create();
    obj_bson_t array;
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, command_bson);
    char *key = NULL;
    obj_bson_type_t bson_type;
    obj_bson_iter_next_internal(&iter, &key, &bson_type);
    obj_bson_value_t *value = obj_bson_iter_value(&iter);
    obj_assert(value->type == OBJ_BSON_TYPE_UTF8);
    obj_bson_append_array_begin(reply, "data", 4, &array);
    int i;
    obj_prealloc_map_entry_t *entry = NULL;
    obj_db_catalog_entry_t *db_entry = NULL;
    char *collection_name = NULL;
    obj_global_lock_t global_lock;
    obj_db_lock_t db_lock;
    obj_global_lock_init(&global_lock, c->context->locker, OBJ_LOCK_MODE_IS);
    obj_db_lock_init(&db_lock, c->context->locker, OBJ_LOCK_MODE_S, value->value.v_utf8.str);
    obj_global_lock_lock(&global_lock);
    obj_db_lock_lock(&db_lock);
    db_entry = obj_db_catalog_entry_get(g_engine, value->value.v_utf8.str);
    if (db_entry == NULL) {
        obj_db_lock_unlock(&db_lock);
        obj_global_lock_unlock(&global_lock);
        obj_bson_append_int32(reply, "code", 4, OBJ_CODE_DB_NOT_EXISTS);
        obj_conn_add_reply(c, reply);
        obj_bson_destroy(reply);
        return;
    }
    for (i = 0; i < db_entry->collections.bucket_size; i++) {
        entry = db_entry->collections.bucket[i];
        while (entry != NULL) {
            collection_name = *(char **)obj_prealloc_map_get_key(&db_entry->collections, entry);
            obj_bson_append_utf8(&array, "", 0, collection_name, obj_strlen(collection_name));
            entry = entry->next;
        }
    }
    obj_db_lock_unlock(&db_lock);
    obj_global_lock_unlock(&global_lock);
    obj_bson_append_array_end(reply, &array);
    obj_conn_add_reply(c, reply);
    obj_bson_destroy(reply);
}