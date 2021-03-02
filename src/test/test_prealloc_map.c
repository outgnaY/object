#include "obj_core.h"

static obj_uint64_t obj_resource_id_request_map_hash_func(const void *key);
static int obj_resource_id_request_map_key_compare(const void *key1, const void *key2);
static void *obj_resource_id_request_map_key_get(void *data);
static void *obj_resource_id_request_map_value_get(void *data);
static void obj_resource_id_request_map_key_set(void *data, void *key);
static void obj_resource_id_request_map_value_set(void *data, void *value);
static void obj_resource_id_request_map_key_dump(void *key);

static obj_prealloc_map_methods_t obj_resource_id_request_map_methods = {
    obj_resource_id_request_map_hash_func,         
    obj_resource_id_request_map_key_compare,
    NULL,
    NULL,
    obj_resource_id_request_map_key_get,
    obj_resource_id_request_map_value_get,
    obj_resource_id_request_map_key_set,
    obj_resource_id_request_map_value_set,
    obj_resource_id_request_map_key_dump,
    NULL
};

static obj_uint64_t obj_resource_id_request_map_hash_func(const void *key) {
    return obj_prealloc_map_hash_function(key, sizeof(obj_lock_resource_id_t));
}

/* compare resource id */
static int obj_resource_id_request_map_key_compare(const void *key1, const void *key2) {
    obj_lock_resource_id_t *id1 = (obj_lock_resource_id_t *)key1;
    obj_lock_resource_id_t *id2 = (obj_lock_resource_id_t *)key2;
    return (*id1) - (*id2);
}

static void *obj_resource_id_request_map_key_get(void *data) {
    obj_resource_id_request_pair_t *pair = (obj_resource_id_request_pair_t *)data;
    return &pair->resource_id;
}

static void *obj_resource_id_request_map_value_get(void *data) {
    obj_resource_id_request_pair_t *pair = (obj_resource_id_request_pair_t *)data;
    return &pair->request;
}

static void obj_resource_id_request_map_key_set(void *data, void *key) {
    obj_resource_id_request_pair_t *pair = (obj_resource_id_request_pair_t *)data;
    obj_memcpy(&pair->resource_id, key, sizeof(obj_lock_resource_id_t));
}

static void obj_resource_id_request_map_value_set(void *data, void *value) {
    obj_resource_id_request_pair_t *pair = (obj_resource_id_request_pair_t *)data;
    obj_memcpy(&pair->request, value, sizeof(obj_lock_request_t));
}

static void obj_resource_id_request_map_key_dump(void *key) {
    obj_lock_resource_id_t resource_id;
    resource_id = *((obj_lock_resource_id_t *)key);
    printf("%lu", resource_id);
}

int main() {
    obj_global_mem_context_init();
    obj_prealloc_map_t map;
    int i;
    obj_prealloc_map_entry_t *entry = NULL;
    obj_prealloc_map_init(&map, &obj_resource_id_request_map_methods, sizeof(obj_resource_id_request_pair_t));
    /* add */
    obj_lock_resource_id_t id;
    for (i = 0; i < 32; i++) {
        id = i;
        obj_prealloc_map_add_key(&map, &id);
    }
    obj_prealloc_map_dump(&map);
    for (i = 0; i < 32; i++) {
        id = i;
        entry = obj_prealloc_map_find(&map, &id);
        obj_assert(entry);
    }
    id = 46;
    entry = obj_prealloc_map_find(&map, &id);
    obj_assert(entry == NULL);
    id = 31;
    entry = obj_prealloc_map_find(&map, &id);
    /* delete entry */
    obj_prealloc_map_delete_entry(&map, entry);
    entry = obj_prealloc_map_find(&map, &id);
    obj_assert(entry == NULL);
    obj_prealloc_map_dump(&map);
    /* delete */
    for (i = 0; i < 31; i++) {
        id = i;
        obj_assert(!obj_prealloc_map_delete(&map, &id, true));
    }
    obj_assert(obj_prealloc_map_is_empty(&map));
    /*
    entry = obj_prealloc_map_find(&map, &id);
    obj_assert(entry);
    obj_assert(*((obj_lock_resource_id_t *)obj_prealloc_map_get_key(&map, entry)) == 20);
    */
    obj_prealloc_map_dump(&map);
    obj_prealloc_map_destroy_static(&map);

    return 0;
}