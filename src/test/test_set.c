#include "obj_core.h"

static obj_uint64_t obj_int_set_hash_func(void *key);
static int obj_int_set_key_compare(void *key1, void *key2);
static void *obj_int_set_key_get(void *data);
static void obj_int_set_key_set(void *data, void *key);

static obj_prealloc_map_methods_t methods = {
    obj_int_set_hash_func,
    obj_int_set_key_compare,
    NULL,
    NULL,
    obj_int_set_key_get,
    NULL,
    obj_int_set_key_set,
    NULL,
    NULL,
    NULL
};


static obj_uint64_t obj_int_set_hash_func(void *key) {
    return obj_set_hash_function(key, sizeof(int));
}

static int obj_int_set_key_compare(void *key1, void *key2) {
    return *(int *)key1 - *(int *)key2;
}

static void *obj_int_set_key_get(void *data) {
    return (int *)data;
}

static void obj_int_set_key_set(void *data, void *key) {
    obj_memcpy(data, key, sizeof(int));
} 

int main() {
    obj_global_mem_context_init();
    obj_set_t set;
    obj_set_init(&set, &methods, sizeof(int));
    int i;
    for (i = 0; i < 60; i++) {
        printf("i = %d\n", i);
        obj_set_add(&set, &i);
    }
    int target = 5;
    obj_assert(obj_set_find(&set, &target));
    target = 60;
    obj_assert(!obj_set_find(&set, &target));
    
    return 0;
}