#include "obj_core.h"

static obj_uint64_t obj_db_catalog_entry_map_hash_func(const void *key);
static int obj_db_catalog_entry_map_key_compare(const void *key1, const void *key2);
static void obj_db_catalog_entry_map_key_free(void *data);
static void obj_db_catalog_entry_map_value_free(void *data);
static void *obj_db_catalog_entry_map_key_get(void *data);
static void *obj_db_catalog_entry_map_value_get(void *data);
static void obj_db_catalog_entry_map_key_set(void *data, void *key);
static void obj_db_catalog_entry_map_value_set(void *data, void *value);

static obj_uint64_t obj_collection_catalog_entry_map_hash_func(const void *key);
static int obj_collection_catalog_entry_map_key_compare(const void *key1, const void *key2);
static void obj_collection_catalog_entry_map_key_free(void *data);
static void obj_collection_catalog_entry_map_value_free(void *data);
static void *obj_collection_catalog_entry_map_key_get(void *data);
static void *obj_collection_catalog_entry_map_value_get(void *data);
static void obj_collection_catalog_entry_map_key_set(void *data, void *key);
static void obj_collection_catalog_entry_map_value_set(void *data, void *value);

static obj_collection_catalog_entry_t *obj_v1_db_catalog_entry_get_collection(obj_db_catalog_entry_t *db_entry, obj_stringdata_t *ns);
static obj_collection_catalog_entry_t *obj_v1_db_catalog_entry_create_collection_if_not_exists(obj_db_catalog_entry_t *db_entry, obj_stringdata_t *ns);
static obj_record_store_t *obj_v1_db_catalog_entry_get_record_store(obj_db_catalog_entry_t *db_entry, obj_stringdata_t *ns);
static obj_status_t obj_v1_db_catalog_entry_drop_collection(obj_db_catalog_entry_t *db_entry, obj_stringdata_t *ns);
static void obj_v1_db_catalog_entry_get_collections(obj_db_catalog_entry_t *db_entry, obj_array_t *array);



/* ********** database catalog entry map methods ********** */
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
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    obj_v1_db_catalog_entry_destroy(pair->entry);
}

static void *obj_db_catalog_entry_map_key_get(void *data) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    return &pair->db;
}

static void *obj_db_catalog_entry_map_value_get(void *data) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    return &pair->entry;
}

/* must copy string before set key */
static void obj_db_catalog_entry_map_key_set(void *data, void *key) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    obj_memcpy(&pair->db, key, sizeof(obj_stringdata_t));
}

static void obj_db_catalog_entry_map_value_set(void *data, void *value) {
    obj_db_catalog_pair_t *pair = (obj_db_catalog_pair_t *)data;
    obj_memcpy(&pair->entry, value, sizeof(obj_v1_db_catalog_entry_t *));
}



/* ********** collection catalog entry map methods ********** */
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
    obj_v1_collection_catalog_entry_destroy(pair->entry);
}

static void *obj_collection_catalog_entry_map_key_get(void *data) {
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

/* ********** database catalog entry methods ********** */
static obj_db_catalog_entry_methods_t db_catalog_entry_methods = {
    obj_v1_db_catalog_entry_get_collection,
    obj_v1_db_catalog_entry_create_collection_if_not_exists,
    obj_v1_db_catalog_entry_get_record_store,
    obj_v1_db_catalog_entry_drop_collection,
    obj_v1_db_catalog_entry_get_collections
};


/* create database catalog entry */
obj_v1_db_catalog_entry_t *obj_v1_db_catalog_entry_create() {
    obj_v1_db_catalog_entry_t *db_entry = obj_alloc(sizeof(obj_v1_db_catalog_entry_t));
    if (db_entry == NULL) {
        return NULL;
    }
    db_entry->base.methods = &db_catalog_entry_methods;
    if (!obj_prealloc_map_init(&db_entry->collections, &collection_catalog_entry_map_methods, sizeof(obj_collection_catalog_pair_t))) {
        obj_free(db_entry);
        return NULL;
    }
    return db_entry;
}

/* destroy database catalog entry */
void obj_v1_db_catalog_entry_destroy(obj_v1_db_catalog_entry_t *db_entry) {
    obj_assert(db_entry);
    obj_prealloc_map_destroy_static(&db_entry->collections);
}

/* get collection catalog entry */
static obj_collection_catalog_entry_t *obj_v1_db_catalog_entry_get_collection(obj_db_catalog_entry_t *db_entry, obj_stringdata_t *ns) {
    obj_v1_db_catalog_entry_t *v1_db_entry = (obj_v1_db_catalog_entry_t *)db_entry;
    obj_prealloc_map_entry_t *entry = NULL;
    obj_v1_collection_catalog_entry_t *v1_collection_entry = NULL;
    entry = obj_prealloc_map_find(&v1_db_entry->collections, ns);
    if (entry == NULL) {
        return NULL;
    }
    v1_collection_entry = *(obj_v1_collection_catalog_entry_t **)obj_prealloc_map_get_value(&v1_db_entry->collections, entry);
    return (obj_collection_catalog_entry_t *)v1_collection_entry;
}

/* create collection if not exists */
static obj_collection_catalog_entry_t *obj_v1_db_catalog_entry_create_collection_if_not_exists(obj_db_catalog_entry_t *db_entry, obj_stringdata_t *ns) {
    obj_v1_db_catalog_entry_t *v1_db_entry = (obj_v1_db_catalog_entry_t *)db_entry;
    obj_prealloc_map_entry_t *entry = NULL;
    obj_v1_collection_catalog_entry_t *v1_collection_entry = NULL;
    entry = obj_prealloc_map_find(&v1_db_entry->collections, ns);
    if (entry != NULL) {
        v1_collection_entry = *(obj_v1_collection_catalog_entry_t **)obj_prealloc_map_get_value(&v1_db_entry->collections, entry);
        return (obj_collection_catalog_entry_t *)v1_collection_entry;
    }
    v1_collection_entry = obj_v1_collection_catalog_entry_create(v1_db_entry);
    if (v1_collection_entry == NULL) {
        return NULL;
    }
    obj_stringdata_t ns_copy = obj_stringdata_copy_stringdata(ns);
    if (ns_copy.data == NULL) {
        obj_v1_collection_catalog_entry_destroy(v1_collection_entry);
        return NULL;
    }
    if (obj_prealloc_map_add(&v1_db_entry->collections, &ns_copy, &v1_collection_entry) != OBJ_PREALLOC_MAP_CODE_OK) {
        obj_stringdata_destroy(&ns_copy);
        obj_v1_collection_catalog_entry_destroy(v1_collection_entry);
        return NULL;
    }
    return (obj_collection_catalog_entry_t *)v1_collection_entry;
}

/* get record store */
static obj_record_store_t *obj_v1_db_catalog_entry_get_record_store(obj_db_catalog_entry_t *db_entry, obj_stringdata_t *ns) {
    obj_v1_db_catalog_entry_t *v1_db_entry = (obj_v1_db_catalog_entry_t *)db_entry;
    obj_prealloc_map_entry_t *entry = NULL;
    obj_v1_collection_catalog_entry_t *v1_collection_entry = NULL;
    entry = obj_prealloc_map_find(&v1_db_entry->collections, ns);
    if (entry == NULL) {
        return NULL;
    }
    v1_collection_entry = *(obj_v1_collection_catalog_entry_t **)obj_prealloc_map_get_value(&v1_db_entry->collections, entry);
    return (obj_record_store_t *)v1_collection_entry->record_store;
}

/* drop collection */
static obj_status_t obj_v1_db_catalog_entry_drop_collection(obj_db_catalog_entry_t *db_entry, obj_stringdata_t *ns) {
    obj_v1_db_catalog_entry_t *v1_db_entry = (obj_v1_db_catalog_entry_t *)db_entry;
    /* remove from map, clean resources */
    obj_prealloc_map_delete(&v1_db_entry->collections, ns, false);
    return obj_status_create("", OBJ_CODE_OK);
}

/* get collections */
static void obj_v1_db_catalog_entry_get_collections(obj_db_catalog_entry_t *db_entry, obj_array_t *array) {
    obj_v1_db_catalog_entry_t *v1_db_entry = (obj_v1_db_catalog_entry_t *)db_entry;
    int i;
    obj_prealloc_map_entry_t *entry = NULL;
    obj_stringdata_t *ns = NULL;
    for (i = 0; i < v1_db_entry->collections.bucket_size; i++) {
        entry = v1_db_entry->collections.bucket[i];
        while (entry != NULL) {
            ns = (obj_stringdata_t *)obj_prealloc_map_get_key(&v1_db_entry->collections, entry);
            if (!obj_array_push_back(array, ns)) {
                obj_array_empty(array);
                return;
            }
            entry = entry->next;
        }
    }
}

/* ********** collection catalog entry methods ********** */

static obj_collection_catalog_entry_methods_t collection_catalog_entry_methods = {

};

/* create collection catalog entry */
obj_v1_collection_catalog_entry_t *obj_v1_collection_catalog_entry_create(obj_v1_db_catalog_entry_t *db_entry) {
    obj_v1_collection_catalog_entry_t *collection_entry = obj_alloc(sizeof(obj_v1_collection_catalog_entry_t));
    if (collection_entry == NULL) {
        return NULL;
    }
    collection_entry->base.methods = &collection_catalog_entry_methods;
    collection_entry->db_entry = db_entry;
    collection_entry->record_store = obj_v1_record_store_create();
    if (collection_entry->record_store == NULL) {
        obj_free(collection_entry);
        return NULL;
    }
    return collection_entry;
}

/* destroy collection catalog entry */
void obj_v1_collection_catalog_entry_destroy(obj_v1_collection_catalog_entry_t *collection_entry) {
    obj_assert(collection_entry);
    /* destroy record store */
    obj_v1_record_store_destroy(collection_entry->record_store);
}

