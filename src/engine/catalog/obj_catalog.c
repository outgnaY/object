#include "obj_core.h"


static obj_uint64_t obj_db_catalog_entry_map_hash_func(void *key);
static int obj_db_catalog_entry_map_key_compare(void *key1, void *key2);
static void obj_db_catalog_entry_map_key_free(void *data);
static void obj_db_catalog_entry_map_value_free(void *data);
static void *obj_db_catalog_entry_map_key_get(void *data);
static void *obj_db_catalog_entry_map_value_get(void *data);
static void obj_db_catalog_entry_map_key_set(void *data, void *key);
static void obj_db_catalog_entry_map_value_set(void *data, void *value);


static obj_uint64_t obj_collection_catalog_entry_map_hash_func(void *key);
static int obj_collection_catalog_entry_map_key_compare(void *key1, void *key2);
static void obj_collection_catalog_entry_map_key_free(void *data);
static void obj_collection_catalog_entry_map_value_free(void *data);
static void *obj_collection_catalog_entry_map_key_get(void *data);
static void *obj_collection_catalog_entry_map_value_get(void *data);
static void obj_collection_catalog_entry_map_key_set(void *data, void *key);
static void obj_collection_catalog_entry_map_value_set(void *data, void *value);


/* database catalog entry map methods */
obj_prealloc_map_methods_t db_catalog_entry_map_methods = {
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

static obj_uint64_t obj_db_catalog_entry_map_hash_func(void *key) {
    char *db = *(char **)key;
    return obj_prealloc_map_hash_function(db, obj_strlen(db));
}

static int obj_db_catalog_entry_map_key_compare(void *key1, void *key2) {
    char *db1 = *(char **)key1;
    char *db2 = *(char **)key2;
    return obj_strcmp(db1, db2);
}

static void obj_db_catalog_entry_map_key_free(void *data) {
    obj_db_catalog_entry_pair_t *pair = (obj_db_catalog_entry_pair_t *)data;
    obj_free(&pair->db);
}

static void obj_db_catalog_entry_map_value_free(void *data) {
    obj_db_catalog_entry_pair_t *pair = (obj_db_catalog_entry_pair_t *)data;

}

static void *obj_db_catalog_entry_map_key_get(void *data) {
    obj_db_catalog_entry_pair_t *pair = (obj_db_catalog_entry_pair_t *)data;
    return &pair->db;
}

static void *obj_db_catalog_entry_map_value_get(void *data) {
    obj_db_catalog_entry_pair_t *pair = (obj_db_catalog_entry_pair_t *)data;
    return &pair->entry;
}

/* must copy before set */
static void obj_db_catalog_entry_map_key_set(void *data, void *key) {
    obj_db_catalog_entry_pair_t *pair = (obj_db_catalog_entry_pair_t *)data;
    obj_memcpy(&pair->db, *(char **)key, sizeof(char *));
}

static void obj_db_catalog_entry_map_value_set(void *data, void *value) {
    obj_db_catalog_entry_pair_t *pair = (obj_db_catalog_entry_pair_t *)data;
    obj_memcpy(&pair->entry, value, sizeof(obj_db_catalog_entry_t *));
}


/* collection catalog entry map methods */
static obj_prealloc_map_methods_t collection_catalog_entry_map_methods = {
    obj_collection_catalog_entry_map_hash_func,
    obj_collection_catalog_entry_map_key_compare,
    obj_collection_catalog_entry_map_key_free,
    obj_collection_catalog_entry_map_value_free,
    obj_collection_catalog_entry_map_key_get,
    obj_collection_catalog_entry_map_value_get,
    obj_collection_catalog_entry_map_key_set,
    obj_collection_catalog_entry_map_value_set,
    NULL,
    NULL
};


static obj_uint64_t obj_collection_catalog_entry_map_hash_func(void *key) {
    char *collection = *(char **)key;
    return obj_prealloc_map_hash_function(collection, obj_strlen(collection));
}

static int obj_collection_catalog_entry_map_key_compare(void *key1, void *key2) {
    char *collection1 = *(char **)key1;
    char *collection2 = *(char **)key2;
    return obj_strcmp(collection1, collection2);
}

static void obj_collection_catalog_entry_map_key_free(void *data) {
    obj_collection_catalog_entry_pair_t *pair = (obj_collection_catalog_entry_pair_t *)data;
    obj_free(&pair->collection);
}

static void obj_collection_catalog_entry_map_value_free(void *data) {
    obj_collection_catalog_entry_pair_t *pair = (obj_collection_catalog_entry_pair_t *)data;
}

static void *obj_collection_catalog_entry_map_key_get(void *data) {
    obj_collection_catalog_entry_pair_t *pair = (obj_collection_catalog_entry_pair_t *)data;
    return &pair->collection;
}

static void *obj_collection_catalog_entry_map_value_get(void *data) {
    obj_collection_catalog_entry_pair_t *pair = (obj_collection_catalog_entry_pair_t *)data;
    return &pair->entry;
}

static void obj_collection_catalog_entry_map_key_set(void *data, void *key) {
    obj_collection_catalog_entry_pair_t *pair = (obj_collection_catalog_entry_pair_t *)data;
    obj_memcpy(&pair->collection, *(char **)key, sizeof(char *));
}

static void obj_collection_catalog_entry_map_value_set(void *data, void *value) {
    obj_collection_catalog_entry_pair_t *pair = (obj_collection_catalog_entry_pair_t *)data;
    obj_memcpy(&pair->entry, value, sizeof(obj_collection_catalog_entry_t *));
}


/* ********** database catalog entry methods  ********** */

/* create database catalog entry */
obj_db_catalog_entry_t *obj_db_catalog_entry_create() {
    obj_db_catalog_entry_t *db_entry = obj_alloc(sizeof(obj_db_catalog_entry_t));
    obj_prealloc_map_init(&db_entry->collections, &collection_catalog_entry_map_methods, sizeof(obj_db_catalog_entry_pair_t));
    return db_entry;
}

/* destroy database catalog entry */
void obj_db_catalog_entry_destroy(obj_db_catalog_entry_t *db_entry) {
    obj_prealloc_map_destroy_static(&db_entry->collections);
    obj_free(db_entry);
}


/* ********** collection catalog entry methods ********** */

/* create collection catalog entry */
obj_collection_catalog_entry_t *obj_collection_catalog_entry_create() {
    obj_collection_catalog_entry_t *collection_entry = obj_alloc(sizeof(obj_collection_catalog_entry_t));
    collection_entry->record_store = obj_record_store_create();
    return collection_entry;
}

/* destroy collection catalog entry */
void obj_colleciton_catalog_entry_destroy(obj_collection_catalog_entry_t *collection_entry) {
    /* no iterators exist */
    obj_assert(obj_record_store_iterator_num(collection_entry->record_store) == 0);
    /* destroy record store */
    obj_record_store_destroy(collection_entry->record_store);
    /* TODO clear other structures */
    obj_free(collection_entry);
}