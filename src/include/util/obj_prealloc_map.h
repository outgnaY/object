#ifndef OBJ_PREALLOC_MAP_H
#define OBJ_PREALLOC_MAP_H

#include "obj_core.h"

typedef enum obj_prealloc_map_error_code obj_prealloc_map_error_code_t;
typedef struct obj_prealloc_map_methods_s obj_prealloc_map_methods_t;
typedef struct obj_prealloc_map_entry_s obj_prealloc_map_entry_t;
typedef struct obj_prealloc_map_s obj_prealloc_map_t;

enum obj_prealloc_map_error_code {
    OBJ_PREALLOC_MAP_CODE_OK = 0,
    OBJ_PREALLOC_MAP_CODE_DUP_KEY,
    OBJ_PREALLOC_MAP_CODE_NOMEM,
    OBJ_PREALLOC_MAP_CODE_KEY_NOT_EXISTS
};

/* methods */
struct obj_prealloc_map_methods_s {
    obj_uint64_t (*hash_func)(const void *key);
    /*
    void *(*key_dup)(const void *key);
    void *(*value_dup)(const void *value);
    */
    int (*key_compare)(const void *key1, const void *key2);
    void (*key_free)(void *data);
    void (*value_free)(void *data);
    void *(*key_get)(void *data);
    void *(*value_get)(void *data);
    void (*key_set)(void *data, void *key);
    void (*value_set)(void *data, void *value);
    void (*key_dump)(void *key);
    void (*value_dump)(void *value);
};

struct obj_prealloc_map_entry_s {
    obj_prealloc_map_entry_t *next;
    /* obj_prealloc_map_entry_t *prev; */
    obj_uint8_t data[];
};

struct obj_prealloc_map_s {
    obj_prealloc_map_entry_t **bucket;
    int bucket_size;
    int size;
    int element_size;
    obj_prealloc_map_methods_t *methods;
};
/* forward declaration */
obj_uint64_t obj_siphash(const obj_uint8_t *in, const obj_size_t inlen, const obj_uint8_t *k);

obj_uint64_t obj_prealloc_map_hash_function(const void *key, int len);
obj_prealloc_map_t *obj_prealloc_map_create(obj_prealloc_map_methods_t *methods, int element_size);
obj_bool_t obj_prealloc_map_init(obj_prealloc_map_t *map, obj_prealloc_map_methods_t *methods, int element_size);
void obj_prealloc_map_destroy_static(obj_prealloc_map_t *map);
void obj_prealloc_map_destroy(obj_prealloc_map_t *map);
obj_prealloc_map_entry_t *obj_prealloc_map_add_key(obj_prealloc_map_t *map, void *key);
obj_prealloc_map_error_code_t obj_prealloc_map_add(obj_prealloc_map_t *map, void *key, void *value);
void obj_prealloc_map_delete_entry(obj_prealloc_map_t *map, obj_prealloc_map_entry_t *entry);
obj_prealloc_map_error_code_t obj_prealloc_map_delete(obj_prealloc_map_t *map, void *key, obj_bool_t nofree);
void obj_prealloc_map_delete_all(obj_prealloc_map_t *map);
obj_prealloc_map_entry_t *obj_prealloc_map_find(obj_prealloc_map_t *map, void *key);
obj_bool_t obj_prealloc_map_is_empty(obj_prealloc_map_t *map);
void obj_prealloc_map_default_key_free(void *key);
void obj_prealloc_map_default_value_free(void *value);
void obj_prealloc_map_default_key_dump(void *key);
void obj_prealloc_map_default_value_dump(void *value);
void obj_prealloc_map_dump(obj_prealloc_map_t *map);

#define OBJ_PREALLOC_MAP_BUCKET_INIT_SIZE 16
#define OBJ_PREALLOC_MAP_BUCKET_MAX_SIZE 65536
#define OBJ_PREALLOC_MAP_LOAD_FACTOR 0.75
#define obj_prealloc_map_hash_key(map, key) ((map)->methods->hash_func(key))
#define obj_prealloc_map_compare_keys(map, key1, key2) \
    (((map)->methods->key_compare) ? \
    (map)->methods->key_compare(key1, key2) : \
    (key1) == (key2))

#define obj_prealloc_map_set_key(map, entry, key) ((map)->methods->key_set(entry->data, key))
#define obj_prealloc_map_set_value(map, entry, value) ((map)->methods->value_set(entry->data, value))
#define obj_prealloc_map_get_key(map, entry) ((map)->methods->key_get(entry->data))
#define obj_prealloc_map_get_value(map, entry) ((map)->methods->value_get(entry->data))
#define obj_prealloc_map_free_key(map, entry) \
    if ((map)->methods->key_free) \
        (map)->methods->key_free((entry)->data)
#define obj_prealloc_map_free_value(map, entry) \
    if ((map)->methods->value_free) \
        (map)->methods->value_free((entry)->data)
#define obj_prealloc_map_key_dump(map, key) do { \
    if ((map)->methods->key_dump) \
        (map)->methods->key_dump(key); \
    else obj_prealloc_map_default_key_dump(key); \
} while(0) \

#define obj_prealloc_map_value_dump(map, value) do { \
    if ((map)->methods->value_dump) \
        (map)->methods->value_dump(value); \
    else obj_prealloc_map_default_value_dump(value); \
} while(0) \


#endif  /* OBJ_PREALLOC_MAP_H */