#include "obj_core.h"


obj_db_manager_t *g_db_manager;

/* ********** database map methods ********** */

static obj_prealloc_map_methods_t db_map_methods = {
    obj_db_map_hash_func,
    obj_db_map_key_compare,
    obj_db_map_key_free,
    obj_db_map_value_free,
    obj_db_map_key_get,
    obj_db_map_value_get,
    obj_db_map_key_set,
    obj_db_map_value_set,
    NULL,
    NULL
};

static obj_uint64_t obj_db_map_hash_func(const void *key) {
    obj_stringdata_t *db = (obj_stringdata_t *)key;
    return obj_prealloc_map_hash_function(db->data, db->size);
}

static int obj_db_map_key_compare(const void *key1, const void *key2) {
    obj_stringdata_t *db1 = (obj_stringdata_t *)key1;
    obj_stringdata_t *db2 = (obj_stringdata_t *)key2;
    return obj_stringdata_compare(db1, db2);
}

static void obj_db_map_key_free(void *data) {
    obj_db_pair_t *pair = (obj_db_pair_t *)data;
    obj_stringdata_destroy(&pair->db);
}

static void obj_db_map_value_free(void *data) {
    obj_db_pair_t *pair = (obj_db_pair_t *)data;
    /* TODO destroy database handler */
    obj_db_handler_destroy(pair->db_handler);
}

static void *obj_db_map_key_get(void *data) {
    obj_db_pair_t *pair = (obj_db_pair_t *)data;
    return &pair->db;
}

static void *obj_db_map_value_get(void *data) {
    obj_db_pair_t *pair = (obj_db_pair_t *)data;
    return &pair->db_handler;
}

/* must copy string before set key */
static void obj_db_map_key_set(void *data, void *key) {
    obj_db_pair_t *pair = (obj_db_pair_t *)data;
    obj_memcpy(&pair->db, key, sizeof(obj_stringdata_t));
}

static void obj_db_map_value_set(void *data, void *value) {
    obj_db_pair_t *pair = (obj_db_pair_t *)data;
    obj_memcpy(&pair->db_handler, value, sizeof(obj_db_handler_t *));
}

/* ********** collection map methods ********** */

static obj_prealloc_map_methods_t collection_map_methods = {
    obj_collection_map_hash_func,
    obj_collection_map_key_compare,
    obj_collection_map_key_free,
    obj_collection_map_value_free,
    obj_collection_map_key_get,
    obj_collection_map_value_get,
    obj_collection_map_key_set,
    obj_collection_map_value_set,
    NULL,
    NULL
};

static obj_uint64_t obj_collection_map_hash_func(const void *key) {
    obj_stringdata_t *collection = (obj_stringdata_t *)key;
    return obj_prealloc_map_hash_function(collection->data, collection->size);
}

static int obj_collection_map_key_compare(const void *key1, const void *key2) {
    obj_stringdata_t *collection1 = (obj_stringdata_t *)key1;
    obj_stringdata_t *collection2 = (obj_stringdata_t *)key2;
    return obj_stringdata_compare(collection1, collection2);
}

static void obj_collection_map_key_free(void *data) {
    obj_collection_pair_t *pair = (obj_collection_pair_t *)data;
    obj_stringdata_destroy(&pair->collection);
}

static void obj_collection_map_value_free(void *data) {
    obj_collection_pair_t *pair = (obj_collection_pair_t *)data;
    /* TODO destroy collection handler */

}

static void *obj_collection_map_key_get(void *data) {
    obj_collection_pair_t *pair = (obj_collection_pair_t *)data;
    return &pair->collection;
}

static void *obj_collection_map_value_get(void *data) {
    obj_collection_pair_t *pair = (obj_collection_pair_t *)data;
    return &pair->collection_handler;
}

/* must copy string before set key */
static void obj_collection_map_key_set(void *data, void *key) {
    obj_collection_pair_t *pair = (obj_collection_pair_t *)data;
    obj_memcpy(&pair->collection, key, sizeof(obj_stringdata_t));
}

static void obj_collection_map_value_set(void *data, void *value) {
    obj_collection_pair_t *pair = (obj_collection_pair_t *)data;
    obj_memcpy(&pair->collection_handler, value, sizeof(obj_collection_handler_t *));
}

/* ********** database manager methods ********** */

/* init global database manager */
void obj_global_db_manager_init() {
    g_db_manager = obj_db_manager_create();
    if (g_db_manager == NULL) {
        fprintf(stderr, "can't init global database manager\n");
        exit(1);
    }
}

/* create database manager */
obj_db_manager_t *obj_db_manager_create() {
    obj_db_manager_t *db_manager = NULL;
    db_manager = obj_alloc(sizeof(obj_db_manager_t));
    if (db_manager == NULL) {
        return NULL;
    }
    if (!obj_prealloc_map_init(&db_manager->dbs, &db_map_methods, sizeof(obj_db_pair_t))) {
        obj_free(db_manager);
        return NULL;
    }
    return db_manager;
}

/* get database handler */
static obj_db_handler_t *obj_db_manager_get_db_handler(obj_db_manager_t *db_manager, obj_stringdata_t *db_name) {
    obj_db_handler_t *db_handler = NULL;
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&db_manager->dbs, db_name);
    if (entry != NULL) {
        db_handler = *(obj_db_handler_t **)obj_prealloc_map_get_value(&db_manager->dbs, entry);
        return db_handler;
    }
    return NULL;
}

/* remove database handler, clean up resources */
static obj_bool_t obj_db_manager_remove_db_handler(obj_db_manager_t *db_manager, obj_stringdata_t *db_name) {
    obj_prealloc_map_error_code_t code = obj_prealloc_map_delete(&db_manager->dbs, db_name, false);
    return code == OBJ_PREALLOC_MAP_CODE_OK;
}

/* open database */
obj_status_with_t obj_db_manager_open_db(obj_conn_context_t *context, obj_db_manager_t *db_manager, obj_stringdata_t *db_name) {
    /* TODO check lock state. must hold database X lock */
    obj_db_handler_t *db_handler =  obj_db_manager_get_db_handler(db_manager, db_name);
    /* already opened */
    if (db_handler != NULL) {
        return obj_status_with_create(db_handler, "", OBJ_CODE_OK);
    }
    obj_db_catalog_entry_t *entry = g_engine->methods->get_db_catalog_entry(db_name);
    /* database not exists */
    if (entry == NULL) {
        return obj_status_with_create(NULL, "database not exists", OBJ_CODE_DB_NOT_EXISTS);
    }
    db_handler = obj_db_handler_create(db_name, entry);
    if (db_handler == NULL) {
        return obj_status_with_create(NULL, "out of memory, can't create database handler", OBJ_CODE_DB_NOMEM);
    }
    /* register database handler */
    /* copy string */
    obj_stringdata_t db_name_copy = obj_stringdata_copy_stringdata(db_name);
    if (db_name_copy.data == NULL) {
        return obj_status_with_create(NULL, "out of memory", OBJ_CODE_DB_NOMEM);
    }
    if (obj_prealloc_map_add(&db_manager->dbs, &db_name_copy, &db_handler) != OBJ_PREALLOC_MAP_CODE_OK) {
        obj_db_handler_destroy(db_handler);
        return obj_status_with_create(NULL, "out of memory, can't register database handler", OBJ_CODE_DB_NOMEM);
    }
    return obj_status_with_create(db_handler, "", OBJ_CODE_OK);
}

/* open database, if not exists, create a new one */
obj_status_with_t obj_db_manager_open_db_create_if_not_exists(obj_conn_context_t *context, obj_db_manager_t *db_manager, obj_stringdata_t *db_name, obj_bool_t *create) {
    if (create) {
        *create = false;
    }
    /* TODO check lock state. must hold database X lock */
    obj_db_handler_t *db_handler = obj_db_manager_get_db_handler(db_manager, db_name);
    /* already opened */
    if (db_handler != NULL) {
        return obj_status_with_create(db_handler, "", OBJ_CODE_OK);
    }
    obj_db_catalog_entry_t *entry = g_engine->methods->get_or_create_db_catalog_entry(db_name, create);
    if (entry == NULL) {
        return obj_status_with_create(db_handler, "out of memory, can't create database", OBJ_CODE_DB_NOMEM);
    }
    db_handler = obj_db_handler_create(db_name, entry);
    if (db_handler == NULL) {
        return obj_status_with_create(NULL, "out of memory, can't create database handler", OBJ_CODE_DB_NOMEM);
    }
    /* register database handler */
    /* copy string */
    obj_stringdata_t db_name_copy = obj_stringdata_copy_stringdata(db_name);
    if (db_name_copy.data == NULL) {
        return obj_status_with_create(NULL, "out of memory", OBJ_CODE_DB_NOMEM);
    }
    if (obj_prealloc_map_add(&db_manager->dbs, &db_name_copy, &db_handler) != OBJ_PREALLOC_MAP_CODE_OK) {
        obj_db_handler_destroy(db_handler);
        return obj_status_with_create(NULL, "out of memory, can't register database handler", OBJ_CODE_DB_NOMEM);
    }
    return obj_status_with_create(db_handler, "", OBJ_CODE_OK);
}

/* close database */
obj_status_t obj_db_manager_close_db(obj_conn_context_t *context, obj_db_manager_t *db_manager, obj_stringdata_t *db_name) {
    /* TODO check lock state. must hold global X lock */
    obj_db_handler_t *db_handler = NULL;
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&db_manager->dbs, db_name);
    if (entry != NULL) {
        db_handler = *(obj_db_handler_t **)obj_prealloc_map_get_value(&db_manager->dbs, entry);
    }
    if (db_handler == NULL) {
        return obj_status_create("database not opened", OBJ_CODE_DB_NOT_OPENED);
    }
    /* TODO close database operations */

    /* delete database handler */
    obj_prealloc_map_delete_entry(&db_manager->dbs, entry);
    return obj_status_create("", OBJ_CODE_OK);
}

/* close all databases */
obj_status_t obj_db_manager_close_all_db(obj_conn_context_t *context, obj_db_manager_t *db_manager) {
    int i;
    obj_prealloc_map_entry_t *entry = NULL;
    obj_db_handler_t *db_handler = NULL;
    for (i = 0; i < db_manager->dbs.bucket_size; i++) {
        entry = db_manager->dbs.bucket[i];
        while (entry != NULL) {
            db_handler = *(obj_db_handler_t **)obj_prealloc_map_get_value(&db_manager->dbs, entry);
            /* TODO close database operations */

            entry = entry->next;
        }
    }
    obj_prealloc_map_destroy_static(&db_manager->dbs);
    return obj_status_create("", OBJ_CODE_OK);
}




/* ********** database methods ********** */

/* create database handler */
obj_db_handler_t *obj_db_handler_create(obj_stringdata_t *db_name, obj_db_catalog_entry_t *db_entry) {
    obj_db_handler_t *db_handler = obj_alloc(sizeof(obj_db_handler_t));
    if (db_handler == NULL) {
        return NULL;
    }
    db_handler->db_entry = db_entry;
    db_handler->name = *db_name;
    if (!obj_prealloc_map_init(&db_handler->collections, &db_map_methods, sizeof(obj_db_pair_t))) {
        obj_free(db_handler);
        return NULL;
    }
    return db_handler;
}

/* destroy database handler */
void obj_db_handler_destroy(obj_db_handler_t *db_handler) {
    obj_assert(db_handler);
    obj_stringdata_destroy(&db_handler->name);
    obj_prealloc_map_destroy_static(&db_handler->collections);
}

/* remove collection handler, clean up resources */
obj_bool_t obj_db_handler_remove_collection_handler(obj_db_handler_t *db_handler, obj_stringdata_t *full_name) {
    obj_prealloc_map_error_code_t code = obj_prealloc_map_delete(&db_handler->collections, full_name, false);
    return code == OBJ_PREALLOC_MAP_CODE_OK;
}

/* drop database */
void obj_db_handler_drop_database(obj_db_handler_t *db_handler) {
    obj_assert(db_handler);
    /* TODO check lock state */

    /* storage engine drop database */
    g_engine->methods->drop_database(&db_handler->name);
    /* remove from global database manager */
    obj_db_manager_remove_db_handler(g_db_manager, &db_handler->name);
}

/*
obj_collection_handler_t *obj_db_handler_create_collection_handler(obj_stringdata_t *collection_name) {

}
*/

/* create collection. if exists, return old */
obj_collection_handler_t *obj_db_handler_create_collection(obj_db_handler_t *db_handler, obj_stringdata_t *full_name) {
    obj_collection_handler_t *collection_handler = obj_db_handler_get_collection_handler(db_handler, full_name);
    if (collection_handler != NULL) {
        return collection_handler;
    }
}
/*
obj_collection_handler_t *obj_db_handler_get_or_create_collection() {

}
*/
obj_status_t obj_db_handler_drop_collection(obj_db_handler_t *db_handler, obj_stringdata_t *full_name) {
    /* TODO check lock state */
    obj_collection_handler_t *collection_handler = NULL;
    collection_handler = obj_db_handler_get_collection_handler(db_handler, full_name);
    if (!collection_handler) {
        return obj_status_create("", OBJ_CODE_OK);
    }
    /* TODO drop indexes of the collection */

    db_handler->db_entry->methods->drop_collection(full_name);
    /* remove from database handler */
    obj_db_handler_remove_collection_handler(db_handler, full_name);
}

/* fullname: db.coll */
static obj_collection_handler_t *obj_db_handler_get_collection_handler(obj_db_handler_t *db_handler, obj_stringdata_t *full_name) {
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&db_handler->collections, full_name);
    if (entry != NULL) {
        obj_collection_handler_t *collection_handler = *(obj_collection_handler_t **)obj_prealloc_map_get_value(&db_handler->collections, entry);
        return collection_handler;
    }
    return NULL;
}



/* ********** collection methods ********** */

/* create collection handler */
obj_collection_handler_t *obj_collection_handler_create(obj_stringdata_t *full_name) {
    obj_collection_handler_t *collection_handler = obj_alloc(sizeof(obj_collection_handler_t));
    if (collection_handler == NULL) {
        return NULL;
    }

    return collection_handler;
}

/* destroy collection handler */
void obj_collection_handler_destroy(obj_collection_handler_t *collection_handler) {
    obj_assert(collection_handler);

    obj_free(collection_handler);
}

/* insert object */
void obj_collection_handler_insert_object(obj_collection_handler_t *collection_handler) {

}

/* insert objects */
void obj_collection_handler_insert_objects(obj_collection_handler_t *collection_handler) {

}

/* delete objects */
void obj_collection_handler_delete_object(obj_collection_handler_t *collection_handler) {

}

/* update object */
void obj_collection_handler_update_object(obj_collection_handler_t *collection_handler) {

}

/* number of records */
int obj_collection_handler_num_records(obj_collection_handler_t *collection_handler) {
    obj_record_store_t *record_store = collection_handler->record_store;
    return record_store->methods->num_records(record_store);
}

