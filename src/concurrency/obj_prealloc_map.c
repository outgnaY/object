#include "obj_core.h"

/* prealloc map implemention, not thread-safe */

static int obj_prealloc_map_index(obj_uint64_t hash, int bucket_size);
static void obj_prealloc_map_resize(obj_prealloc_map_t *map);


/* hash functions */
static obj_uint8_t obj_prealloc_map_hash_function_seed[16];

obj_uint64_t obj_prealloc_map_hash_function(const void *key, int len) {
    return obj_siphash(key, len, obj_prealloc_map_hash_function_seed);
}

obj_prealloc_map_t *obj_prealloc_map_create(obj_prealloc_map_methods_t *methods, int element_size) {
    obj_prealloc_map_t *map = obj_alloc(sizeof(obj_prealloc_map_t));
    if (map == NULL) {
        return NULL;
    }
    if (!obj_prealloc_map_init(map, methods, element_size)) {
        obj_free(map);
        return NULL;
    }
    return map;
}

obj_bool_t obj_prealloc_map_init(obj_prealloc_map_t *map, obj_prealloc_map_methods_t *methods, int element_size) {
    obj_assert(map != NULL);
    map->bucket_size = OBJ_PREALLOC_MAP_BUCKET_INIT_SIZE;
    map->size = 0;
    map->methods = methods;
    map->element_size = element_size;
    map->bucket = (obj_prealloc_map_entry_t **)obj_alloc(map->bucket_size * sizeof(obj_prealloc_map_entry_t *));
    if (map->bucket == NULL) {
        return false;
    }
    obj_memset(map->bucket, 0, sizeof(obj_prealloc_map_entry_t *) * map->bucket_size);
    return true;
}

void obj_prealloc_map_destroy_static(obj_prealloc_map_t *map) {
    obj_assert(map);
    if (map->bucket == NULL) {
        return;
    }
    int i;
    obj_prealloc_map_entry_t *entry = NULL, *next_entry = NULL;
    for (i = 0; i < map->bucket_size; i++) {
        if ((entry = map->bucket[i]) == NULL) {
            continue;
        }
        while (entry) {
            next_entry = entry->next;
            obj_prealloc_map_free_key(map, entry);
            obj_prealloc_map_free_value(map, entry);
            obj_free(entry);
            entry = next_entry;
        }
    }
    obj_free(map->bucket);
}

void obj_prealloc_map_destroy(obj_prealloc_map_t *map) {
    obj_prealloc_map_destroy_static(map);
    obj_free(map);
}

static inline int obj_prealloc_map_index(obj_uint64_t hash, int bucket_size) {
    return hash & (bucket_size - 1);
}

static void obj_prealloc_map_resize(obj_prealloc_map_t *map) {
    int new_bucket_size = (map->bucket_size * 2 <= OBJ_PREALLOC_MAP_BUCKET_MAX_SIZE ? map->bucket_size * 2 : OBJ_PREALLOC_MAP_BUCKET_MAX_SIZE);
    if (new_bucket_size <= map->bucket_size) {
        return;
    }
    obj_prealloc_map_entry_t **new_bucket = (obj_prealloc_map_entry_t **)obj_alloc(new_bucket_size * sizeof(obj_prealloc_map_entry_t *));
    if (new_bucket == NULL) {
        return;
    }
    obj_memset(new_bucket, 0, new_bucket_size * sizeof(obj_prealloc_map_entry_t *));
    int i;
    obj_prealloc_map_entry_t *entry = NULL, *next_entry = NULL;
    obj_uint64_t hash;
    int index;
    void *cur_key = NULL;
    for (i = 0; i < map->bucket_size; i++) {
        if ((entry = map->bucket[i]) == NULL) {
            continue;
        }
        while (entry) {
            cur_key = obj_prealloc_map_get_key(map, entry);
            next_entry = entry->next;
            /* rehash */
            hash = obj_prealloc_map_hash_key(map, cur_key);
            index = obj_prealloc_map_index(hash, new_bucket_size);
            entry->next = new_bucket[index];
            /*
            if (new_bucket[index] != NULL) {
                new_bucket[index]->prev = entry;
            }
            entry->prev = NULL;
            */
            new_bucket[index] = entry;
            entry = next_entry;
        }
    }
    obj_free(map->bucket);
    map->bucket = new_bucket;
    map->bucket_size = new_bucket_size;
}

obj_prealloc_map_entry_t *obj_prealloc_map_add_key(obj_prealloc_map_t *map, void *key) {
    obj_prealloc_map_entry_t *entry = NULL;
    int index;
    obj_uint64_t hash;
    if (map->size > map->bucket_size * OBJ_PREALLOC_MAP_LOAD_FACTOR) {
        obj_prealloc_map_resize(map);
    }
    hash = obj_prealloc_map_hash_key(map, key);
    index = obj_prealloc_map_index(hash, map->bucket_size);
    entry = map->bucket[index];
    void *cur_key = NULL;
    while (entry) {
        cur_key = obj_prealloc_map_get_key(map, entry);
        if (key == cur_key || obj_prealloc_map_compare_keys(map, key, cur_key) == 0) {
            return NULL;
        }
        entry = entry->next;
    }
    entry = obj_alloc(map->element_size + sizeof(obj_prealloc_map_entry_t));
    if (entry == NULL) {
        return NULL;
    }
    entry->next = map->bucket[index];
    /*
    if (map->bucket[index] != NULL) {
        map->bucket[index]->prev = entry;
    }
    entry->prev = NULL;
    */
    map->bucket[index] = entry;
    /* set key */
    obj_prealloc_map_set_key(map, entry, key);
    ++map->size;
    return entry;
}

obj_prealloc_map_error_code_t obj_prealloc_map_add(obj_prealloc_map_t *map, void *key, void *value) {
    obj_prealloc_map_entry_t *entry = NULL;
    int index;
    obj_uint64_t hash;
    if (map->size > map->bucket_size * OBJ_PREALLOC_MAP_LOAD_FACTOR) {
        obj_prealloc_map_resize(map);
    }
    hash = obj_prealloc_map_hash_key(map, key);
    index = obj_prealloc_map_index(hash, map->bucket_size);
    entry = map->bucket[index];
    void *cur_key = NULL;
    while (entry) {
        cur_key = obj_prealloc_map_get_key(map, entry);
        if (key == cur_key || obj_prealloc_map_compare_keys(map, key, cur_key) == 0) {
            return OBJ_PREALLOC_MAP_CODE_DUP_KEY;
        }
        entry = entry->next;
    }
    entry = obj_alloc(map->element_size + sizeof(obj_prealloc_map_entry_t));
    if (entry == NULL) {
        return OBJ_PREALLOC_MAP_CODE_NOMEM;
    }
    entry->next = map->bucket[index];
    /*
    if (map->bucket[index] != NULL) {
        map->bucket[index]->prev = entry;
    }
    entry->prev = NULL;
    */
    map->bucket[index] = entry;
    /* set key and value */
    obj_prealloc_map_set_key(map, entry, key);
    obj_prealloc_map_set_value(map, entry, value);
    ++map->size;
    return OBJ_PREALLOC_MAP_CODE_OK;
}

void obj_prealloc_map_delete_entry(obj_prealloc_map_t *map, obj_prealloc_map_entry_t *entry) {
    printf("delete entry\n");
    obj_assert(entry);
    int index;
    void *key = obj_prealloc_map_get_key(map, entry);
    obj_uint64_t hash;
    hash = obj_prealloc_map_hash_key(map, key);
    index = obj_prealloc_map_index(hash, map->bucket_size);
    if (map->bucket[index] == entry) {
        map->bucket[index] = entry->next;
        goto free;
    }
    obj_prealloc_map_entry_t *prev_entry = map->bucket[index];
    while (prev_entry->next != entry) {
        prev_entry = prev_entry->next;
    }
    prev_entry->next = entry->next;
free:
    obj_prealloc_map_free_key(map, entry);
    obj_prealloc_map_free_value(map, entry);
    obj_free(entry);
    map->size--;
    return;
}

obj_prealloc_map_error_code_t obj_prealloc_map_delete(obj_prealloc_map_t *map, void *key, obj_bool_t nofree) {
    printf("delete\n");
    obj_prealloc_map_entry_t *entry, *prev_entry;
    obj_uint64_t hash;
    int index;
    hash = obj_prealloc_map_hash_key(map, key);
    index = obj_prealloc_map_index(hash, map->bucket_size);
    entry = map->bucket[index];
    prev_entry = NULL;
    void *cur_key = NULL;
    while (entry) {
        cur_key = obj_prealloc_map_get_key(map, entry);
        if (key == cur_key || obj_prealloc_map_compare_keys(map, key, cur_key) == 0) {
            /* unlink from list */
            if (prev_entry) {
                prev_entry->next = entry->next;
                /*
                if (entry->next != NULL) {
                    entry->next->prev = prev_entry;
                }
                */
            } else {
                map->bucket[index] = entry->next;
                /*
                entry->next->prev = NULL;
                */
            }
            if (!nofree) {
                obj_prealloc_map_free_key(map, entry);
                obj_prealloc_map_free_value(map, entry);
                obj_free(entry);
            }
            map->size--;
            return OBJ_PREALLOC_MAP_CODE_OK;
        }
        prev_entry = entry;
        entry = entry->next;
    }
    return OBJ_PREALLOC_MAP_CODE_KEY_NOT_EXISTS;
}

obj_prealloc_map_entry_t *obj_prealloc_map_find(obj_prealloc_map_t *map, void *key) {
    obj_prealloc_map_entry_t *entry;
    obj_uint64_t hash;
    int index;
    hash = obj_prealloc_map_hash_key(map, key);
    index = obj_prealloc_map_index(hash, map->bucket_size);
    entry = map->bucket[index];
    void *cur_key = NULL;
    while (entry) {
        cur_key = obj_prealloc_map_get_key(map, entry);
        if (key == cur_key || obj_prealloc_map_compare_keys(map, key, cur_key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

/* for check */
obj_bool_t obj_prealloc_map_is_empty(obj_prealloc_map_t *map) {
    int i;
    obj_prealloc_map_entry_t *entry;
    for (i = 0; i < map->bucket_size; i++) {
        entry = map->bucket[i];
        if (entry) {
            return false;
        }
    }
    return true;
}

void obj_prealloc_map_default_key_free(void *key) {
    obj_free(key);
}

void obj_prealloc_map_default_value_free(void *value) {
    obj_free(value);
}

void obj_prealloc_map_default_key_dump(void *key) {
    printf("%p", key);
}

void obj_prealloc_map_default_value_dump(void *value) {
    printf("%p", value);
}

void obj_prealloc_map_dump(obj_prealloc_map_t *map) {
    int i;
    obj_prealloc_map_entry_t *entry;
    void *key, *value;
    printf("********** map **********\n");
    printf("map: %p, size: %d\n", map, map->size);
    for (i = 0; i < map->bucket_size; i++) {
        printf("bucket [%d]:\n", i);
        entry = map->bucket[i];
        if (entry == NULL) {
            printf("empty");
        }
        while (entry) {
            key = obj_prealloc_map_get_key(map, entry);
            value = obj_prealloc_map_get_value(map, entry);
            printf("[");
            obj_prealloc_map_key_dump(map, key);
            printf("->");
            obj_prealloc_map_value_dump(map, value);
            printf("]");
            printf("  ");
            entry = entry->next;
        }
        printf("\n");
    }
}
