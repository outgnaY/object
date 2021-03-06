#include "obj_core.h"

static obj_prealloc_map_methods_t db_catalog_entry_map_methods = {
    obj_db_catalog_entry_map_hash_func,
    obj_db_catalog_entry_map_key_compare,
    obj_db_catalog_entry_map_key_free,
    obj_db_catalog_entry_map_value_free,
    obj_db_catalog_entry_map_key_get,
    obj_db_catalog_entry_map_value_get,
    obj_db_catalog_entry_map_key_set,
    obj_db_catalog_entry_map_value_set,
    NULL,
    NULL
};

static obj_uint64_t obj_db_catalog_entry_map_hash_func(const void *key) {
    obj_stringdata_t *db = (obj_stringdata_t *)key;
    return obj_prealloc_map_hash_function(db->data, db->size);
}

static int obj_db_catalog_entry_map_key_compare(const void *key1, const void *key2) {
    obj_stringdata_t *db1 = (obj_stringdata_t *)key1;
    obj_stringdata_t *db2 = (obj_stringdata_t *)key2;
    return obj_stringdata_compare(db1, db2);
}

static void obj_db_catalog_entry_map_key_free(void *data) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    obj_stringdata_destroy(&pair->db);
}

static void obj_db_catalog_entry_map_value_free(void *data) {
    
}

static void *obj_db_catalog_entry_map_key_get(void *data) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    return &pair->db;
}

static void *obj_db_catalog_entry_map_value_get(void *data) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    return &pair->entry;
}

/* must copy before set key */
static void obj_db_catalog_entry_map_key_set(void *data, void *key) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    obj_memcpy(&pair->db, key, sizeof(obj_stringdata_t));
}

static void obj_db_catalog_entry_map_value_set(void *data, void *value) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    obj_memcpy(&pair->entry, value, sizeof(obj_v1_db_catalog_entry_t *));
}

static obj_prealloc_map_methods_t collection_catalog_entry_map_methods = {
    obj_collection_catalog_entry_map_hash_func,
    obj_collection_catalog_entry_map_key_compare,
    obj_collection_catalog_entry_map_key_free,
    obj_collection_catalog_entry_map_value_free,
    obj_collecton_catalog_entry_map_key_get,
    obj_collection_catalog_entry_map_value_get,
    obj_collection_catalog_entry_map_key_set,
    obj_collection_catalog_entry_map_value_set,
    NULL,
    NULL
};


static obj_uint64_t obj_collection_catalog_entry_map_hash_func(const void *key) {
    obj_stringdata_t *collection = (obj_stringdata_t *)key;
    return obj_prealloc_map_hash_function(collection->data, collection->size);
}

static int obj_collection_catalog_entry_map_key_compare(const void *key1, const void *key2) {
    obj_stringdata_t *collection1 = (obj_stringdata_t *)key1;
    obj_stringdata_t *collection2 = (obj_stringdata_t *)key2;
    return obj_stringdata_compare(collection1, collection2);
}

static void obj_collection_catalog_entry_map_key_free(void *data) {
    obj_collection_catalog_pair_t *pair = (obj_collection_catalog_pair_t *)data;
    obj_stringdata_destroy(&pair->collection);
}

static void obj_collection_catalog_entry_map_value_free(void *data) {
    obj_collection_catalog_pair_t *pair = (obj_collection_catalog_pair_t *)data;
    obj_v1_collection_catalog_entry_t *entry = pair->entry;
    obj_v1_record_store_t *record_store =entry->record_store;
    obj_v1_record_store_destroy(record_store);
}

static void *obj_collecton_catalog_entry_map_key_get(void *data) {
    obj_collection_catalog_pair_t *pair = (obj_collection_catalog_pair_t *)data;
    return &pair->collection;
}

static void *obj_collection_catalog_entry_map_value_get(void *data) {
    obj_collection_catalog_pair_t *pair = (obj_collection_catalog_pair_t *)data;
    return &pair->entry;
}

/* must copy before set key */
static void obj_collection_catalog_entry_map_key_set(void *data, void *key) {
    obj_collection_catalog_pair_t *pair = (obj_collection_catalog_pair_t *)data;
    obj_memcpy(&pair->collection, key, sizeof(obj_stringdata_t));
}

static void obj_collection_catalog_entry_map_value_set(void *data, void *value) {
    obj_collection_catalog_pair_t *pair = (obj_collection_catalog_pair_t *)data;
    obj_memcpy(&pair->entry, value, sizeof(obj_v1_collection_catalog_entry_t *));
}

/* database catalog entry related methods */

/* create database catalog entry */
obj_v1_db_catalog_entry_t *obj_v1_catalog_db_catalog_entry_create() {
    obj_v1_db_catalog_entry_t *entry = obj_alloc(sizeof(obj_v1_db_catalog_entry_t));
    if (entry == NULL) {
        return NULL;
    }
    obj_prealloc_map_init(&entry->collections, &db_catalog_entry_map_methods, sizeof(obj_collection_catalog_pair_t));
    return entry;
}

obj_v1_db_catalog_entry_t *obj_v1_catalog_db_catalog_entry_get(obj_prealloc_map_t *map, obj_stringdata_t *db) {
    obj_v1_db_catalog_entry_t *db_entry = NULL;
    obj_prealloc_map_entry_t *entry = NULL;
    entry = obj_prealloc_map_find(map, db);
    if (entry == NULL) {
        return NULL;
    }
    db_entry = *(obj_v1_db_catalog_entry_t **)obj_db_catalog_entry_map_value_get(entry->data);
    return db_entry;
}



