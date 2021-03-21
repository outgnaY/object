#include "obj_core.h"

/* hash functions */
static obj_uint8_t obj_set_hash_function_seed[16];

obj_uint64_t obj_set_hash_function(const void *key, int len) {
    return obj_siphash((obj_uint8_t *)key, len, obj_set_hash_function_seed);
}

/* create set */
obj_set_t *obj_set_create(obj_prealloc_map_methods_t *methods, int element_size) {
    obj_set_t *set = obj_alloc(sizeof(obj_set_t));
    obj_set_init(set, methods, element_size);
    return set;
}

/* init set */
void obj_set_init(obj_set_t *set, obj_prealloc_map_methods_t *methods, int element_size) {
    obj_assert(set);
    obj_prealloc_map_init(&set->map, methods, element_size);
}

void obj_set_destroy_static(obj_set_t *set) {
    obj_assert(set);
    /* destroy map */
    obj_prealloc_map_destroy_static(&set->map);
}

void obj_set_destroy(obj_set_t *set) {
    obj_assert(set);
    obj_set_destroy_static(set);
    obj_free(set);
}

/* find if key in the set */
obj_bool_t obj_set_find(obj_set_t *set, void *key) {
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&set->map, key);
    return entry != NULL;
}

/* add to set */
void obj_set_add(obj_set_t *set, void *key) {
    obj_prealloc_map_add(&set->map, key, NULL);
}

void obj_set_delete_all(obj_set_t *set) {
    obj_prealloc_map_delete_all(&set->map);
}


