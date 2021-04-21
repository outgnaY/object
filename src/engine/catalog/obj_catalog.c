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

static obj_db_catalog_entry_t *obj_db_catalog_entry_create();
static void obj_db_catalog_entry_destroy(obj_db_catalog_entry_t *db_entry);

static obj_collection_catalog_entry_t *obj_collection_catalog_entry_create(obj_bson_t *prototype);
static void obj_collection_catalog_entry_destroy(obj_collection_catalog_entry_t *collection_entry);
static void obj_index_catalog_entry_destroy(obj_index_catalog_entry_t *index_entry);


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
    obj_free(pair->db);
}

static void obj_db_catalog_entry_map_value_free(void *data) {
    obj_db_catalog_entry_pair_t *pair = (obj_db_catalog_entry_pair_t *)data;
    obj_db_catalog_entry_destroy(pair->entry);
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
    obj_memcpy(&pair->db, key, sizeof(char *));
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
    obj_free(pair->collection);
}

static void obj_collection_catalog_entry_map_value_free(void *data) {
    obj_collection_catalog_entry_pair_t *pair = (obj_collection_catalog_entry_pair_t *)data;
    obj_collection_catalog_entry_destroy(pair->entry);
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
    obj_memcpy(&pair->collection, key, sizeof(char *));
}

static void obj_collection_catalog_entry_map_value_set(void *data, void *value) {
    obj_collection_catalog_entry_pair_t *pair = (obj_collection_catalog_entry_pair_t *)data;
    obj_memcpy(&pair->entry, value, sizeof(obj_collection_catalog_entry_t *));
}


/* ********** database catalog entry methods  ********** */

/* create database catalog entry */
static obj_db_catalog_entry_t *obj_db_catalog_entry_create() {
    obj_db_catalog_entry_t *db_entry = obj_alloc(sizeof(obj_db_catalog_entry_t));
    obj_prealloc_map_init(&db_entry->collections, &collection_catalog_entry_map_methods, sizeof(obj_db_catalog_entry_pair_t));
    return db_entry;
}

/* destroy database catalog entry */
static void obj_db_catalog_entry_destroy(obj_db_catalog_entry_t *db_entry) {
    obj_prealloc_map_destroy_static(&db_entry->collections);
    obj_free(db_entry);
}

obj_db_catalog_entry_t *obj_db_catalog_entry_get(obj_engine_t *engine, char *db_name) {
    obj_prealloc_map_entry_t *entry = NULL;
    entry = obj_prealloc_map_find(&engine->map, &db_name);
    if (entry == NULL) {
        return NULL;
    }
    return *(obj_db_catalog_entry_t **)obj_prealloc_map_get_value(&engine->map, entry);
}

void obj_delete_db(obj_engine_t *engine, char *db_name) {
    obj_prealloc_map_delete(&engine->map, &db_name, false);
}

void obj_create_db(obj_engine_t *engine, char *db_name) {
    obj_db_catalog_entry_t *db_entry = obj_db_catalog_entry_create();
    obj_prealloc_map_add(&engine->map, &db_name, &db_entry);
}


/* ********** collection catalog entry methods ********** */

/* create collection catalog entry */
static obj_collection_catalog_entry_t *obj_collection_catalog_entry_create(obj_bson_t *prototype) {
    obj_collection_catalog_entry_t *collection_entry = obj_alloc(sizeof(obj_collection_catalog_entry_t));
    collection_entry->record_store = obj_record_store_create();
    collection_entry->checker.prototype = prototype;
    obj_array_init(&collection_entry->indexes, sizeof(obj_index_catalog_entry_t));
    obj_array_set_free(&collection_entry->indexes, obj_index_catalog_entry_destroy);
    return collection_entry;
}

/* destroy collection catalog entry */
static void obj_collection_catalog_entry_destroy(obj_collection_catalog_entry_t *collection_entry) {
    /* no iterators exist */
    obj_assert(obj_record_store_iterator_num(collection_entry->record_store) == 0);
    /* destroy record store */
    obj_record_store_destroy(collection_entry->record_store);
    obj_array_destroy_static(&collection_entry->indexes);
    obj_type_checker_destroy_static(&collection_entry->checker);
    obj_free(collection_entry);
}

obj_collection_catalog_entry_t *obj_collection_catalog_entry_get(obj_db_catalog_entry_t *db_entry, char *collection_name) {
    obj_prealloc_map_entry_t *entry = NULL;
    entry = obj_prealloc_map_find(&db_entry->collections, &collection_name);
    if (entry == NULL) {
        return NULL;
    }
    return *(obj_collection_catalog_entry_t **)obj_prealloc_map_get_value(&db_entry->collections, entry);
}

obj_bool_t obj_delete_collection(obj_db_catalog_entry_t *db_entry, char *collection_name) {
    return obj_prealloc_map_delete(&db_entry->collections, &collection_name, false);
}

obj_bool_t obj_create_collection(obj_db_catalog_entry_t *db_entry, char *collection_name, obj_bson_t *prototype) {
    obj_collection_catalog_entry_t *collection_entry = obj_collection_catalog_entry_create(prototype);
    return obj_prealloc_map_add(&db_entry->collections, &collection_name, &collection_entry);
}

/* insert objects */
void obj_insert_objects(obj_collection_catalog_entry_t *collection_entry, obj_bson_t *objects) {
    obj_bson_iter_t iter;
    obj_bson_iter_init(&iter, objects);
    char *key = NULL;
    obj_bson_type_t bson_type;
    int i;
    obj_array_t *indexes = &collection_entry->indexes;
    obj_bson_value_t *value = NULL;
    while (obj_bson_iter_next_internal(&iter, &key, &bson_type)) {
        value = obj_bson_iter_value(&iter);
        obj_uint8_t *data_copy = obj_alloc(value->value.v_object.len);
        obj_memcpy(data_copy, value->value.v_object.data, value->value.v_object.len);
        obj_bson_t *object = obj_bson_create_with_data(data_copy, value->value.v_object.len);
        obj_record_t *record = obj_alloc(sizeof(obj_record_t));
        record->bson = object;
        /* add to record store */
        obj_record_store_add_record(collection_entry->record_store, record);
        /* index */
        for (i = 0; i < indexes->size; i++) {
            obj_index_catalog_entry_t *index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(indexes, i);
            obj_bson_t *index_key = obj_get_index_key(object, index_entry->key_pattern);
            obj_skiplist_insert(index_entry->skiplist, index_key, record);
        }
    }
}



/* ********** index catalog entry methods ********** */

static void obj_index_catalog_entry_destroy(obj_index_catalog_entry_t *index_entry) {
    obj_bson_destroy(index_entry->key_pattern);
    obj_free(index_entry->name);
    obj_skiplist_destroy(index_entry->skiplist);
}

/* get indexes */
inline obj_array_t *obj_collection_catalog_entry_get_indexes(obj_collection_catalog_entry_t *collection_entry) {
    return &collection_entry->indexes;
}

/* create index */
obj_status_t obj_create_index(obj_collection_catalog_entry_t *collection_entry, obj_bson_t *key_pattern, char *index_name) {
    if (collection_entry->indexes.size >= OBJ_INDEX_NUM_MAX) {        
        return obj_status_create("too many indexes", OBJ_CODE_INDEX_NUM_EXCEED);
    }
    obj_index_catalog_entry_t index_entry;
    
    index_entry.key_pattern = key_pattern;
    index_entry.name = index_name;
    int i;
    obj_index_catalog_entry_t *curr = NULL;
    for (i = 0; i < collection_entry->indexes.size; i++) {
        curr = (obj_index_catalog_entry_t *)obj_array_get_index(&collection_entry->indexes, i);
        if (obj_strcmp(curr->name, index_name) == 0) {
            return obj_status_create("duplicate index", OBJ_CODE_INDEX_DUPLICATE);
        }
    }
    obj_index_key_order_t order;
    int nfields;
    if (!obj_index_key_pattern_is_valid(collection_entry->checker.prototype, key_pattern, &order, &nfields)) {
        return obj_status_create("invalid index key pattern", OBJ_CODE_INDEX_KEY_PATTERN_INVALID);
    }
    index_entry.nfields = nfields;
    index_entry.skiplist = obj_skiplist_create(order);
    obj_array_push_back(&collection_entry->indexes, &index_entry);
    return obj_status_create("", OBJ_CODE_OK);
}

/* delete index */
obj_bool_t obj_delete_index(obj_collection_catalog_entry_t *collection_entry, char *index_name) {
    int i;
    obj_index_catalog_entry_t *index_entry = NULL;
    for (i = 0; i < collection_entry->indexes.size; i++) {
        index_entry = (obj_index_catalog_entry_t *)obj_array_get_index(&collection_entry->indexes, i);
        if (obj_strcmp(index_entry->name, index_name) == 0) {
            /* found */
            obj_array_remove(&collection_entry->indexes, i);
            return true;
        }
    }
    return false;
}