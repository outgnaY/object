#ifndef OBJ_HASHTABLE_H
#define OBJ_HASHTABLE_H

#include "obj_core.h"

typedef struct obj_hashtable_methods_s obj_hashtable_methods_t;
typedef struct obj_hashtable_entry_s obj_hashtable_entry_t;
typedef struct obj_hashtable_s obj_hashtable_t;

/* methods */
struct obj_hashtable_methods_s {
    obj_uint64_t (*hash_func)(const void *key);
    void *(*key_dup)(const void *key);
    void *(*value_dup)(const void *value);
    int (*key_compare)(const void *key1, const void *key2);
    void (*key_free)(void *key);
    void (*value_free)(void *value);
};

/* hashtable entry */
struct obj_hashtable_entry_s {
    void *key;
    void *value;
    obj_hashtable_entry_t *next;
};

/* hashtable */
struct obj_hashtable_s {
    obj_hashtable_entry_t **bucket;
    int bucket_size;
    int size;
    obj_hashtable_methods_t *methods;
};

/* forward declaration */
obj_uint64_t obj_siphash(const obj_uint8_t *in, const obj_size_t inlen, const obj_uint8_t *k);

obj_uint64_t obj_hashtable_hash_function(const void *key, int len);
obj_hashtable_t *obj_hashtable_create(obj_hashtable_methods_t *methods);
void obj_hashtable_destroy_static(obj_hashtable_t *table);
void obj_hashtable_destroy(obj_hashtable_t *table);
obj_global_error_code_t obj_hashtable_add(obj_hashtable_t *table, void *key, void *value);
obj_global_error_code_t obj_hashtable_delete(obj_hashtable_t *table, void *key, obj_bool_t nofree);
obj_hashtable_entry_t *obj_hashtable_find(obj_hashtable_t *table, void *key);
obj_bool_t obj_hashtable_is_empty(obj_hashtable_t *table);
void obj_hashtable_default_key_free(void *key);
void obj_hashtable_default_value_free(void *value);


#define OBJ_HASHTABLE_BUCKET_INIT_SIZE 16
#define OBJ_HASHTABLE_BUCKET_MAX_SIZE 65536
#define OBJ_HASHTABLE_LOAD_FACTOR 0.75
#define obj_hashtable_hash_key(table, key) ((table)->methods->hash_func(key))
#define obj_hashtable_compare_keys(table, key1, key2) \
    (((table)->methods->key_compare) ? \
    (table)->methods->key_compare(key1, key2) : \
    (key1) == (key2))

#define obj_hashtable_set_key(table, entry, key) do { \
    if ((table)->methods->key_dup) \
        (entry)->key = (table)->methods->key_dup(key); \
    else \
        (entry)->key = key; \
} while(0)

#define obj_hashtable_set_value(table, entry, value) do {\
    if ((table)->methods->value_dup) \
        (entry)->value = (table)->methods->value_dup(value); \
    else \
        (entry)->value = value; \
} while(0)

#define obj_hashtable_free_key(table, entry) \
    if ((table)->methods->key_free) \
        (table)->methods->key_free((entry)->key)

#define obj_hashtable_free_value(table, entry) \
    if ((table)->methods->value_free) \
        (table)->methods->value_free((entry)->value)

#endif  /* OBJ_HASHTABLE_H */