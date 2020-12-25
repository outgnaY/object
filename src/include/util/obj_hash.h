#ifndef OBJ_HASH_H
#define OBJ_HASH_H

#include "obj_common.h"

/* hash table */
typedef struct obj_hash_table_methods_s obj_hash_table_methods_t;
typedef struct obj_hash_table_s obj_hash_table_t;
typedef struct obj_hash_table_entry_s obj_hash_table_entry_t;

/* methods */
struct obj_hash_table_methods_s {
    obj_uint64_t (*hash_func)(const void *key);
    void *(*key_dup)(const void *key);
    void *(*value_dup)(const void *value);
    int (*key_compare)(const void *key1, const void *key2);
    void (*key_free)(void *key);
    void (*value_free)(void *value);
};

/* hash table */
struct obj_hash_table_s {
    obj_hash_table_methods_t *methods;
    unsigned long n_buckets;
    obj_hash_table_entry_t **table;
    unsigned long n_mutexes;
    pthread_mutex_t *mutexes;
    pthread_mutex_t *used_mutex; 
    unsigned long used;
    unsigned long mask;
};

struct obj_hash_table_entry_s {
    void *key;
    void *value;
    obj_hash_table_entry_t *next;
};

#define OBJ_HASH_TABLE_MIN_SIZE 16

#define OBJ_HASH_TABLE_SEGMENTS 16

#define OBJ_HASH_TABLE_SEGMENTS_SHIFT 4

#define obj_hash_table_used(table) ((table)->used)

#define obj_hash_table_hash_key(table, key) ((table)->methods->hash_func(key))

#define obj_hash_table_compare_keys(table, key1, key2) \
    (((table)->methods->key_compare) ? \
    (table)->methods->key_compare(key1, key2) : \
    (key1) == (key2))

#define obj_hash_table_set_key(table, entry, key) do { \
    if ((table)->methods->key_dup) \
        (entry)->key = (table)->methods->key_dup(key); \
    else \
        (entry)->key = key; \
} while(0)

#define obj_hash_table_set_value(table, entry, value) do {\
    if ((table)->methods->value_dup) \
        (entry)->value = (table)->methods->value_dup(value); \
    else \
        (entry)->value = value; \
} while(0)

#define obj_hash_table_free_key(table, entry) \
    if ((table)->methods->key_free) \
        (table)->methods->key_free((entry)->key)

#define obj_hash_table_free_value(table, entry) \
    if ((table)->methods->value_free) \
        (table)->methods->value_free((entry)->value)

#define obj_hash_table_find_lock(table, index) (&table->mutexes[(index << OBJ_HASH_TABLE_SEGMENTS_SHIFT) / table->n_buckets])

/* functions */
obj_hash_table_t *obj_hash_table_create(obj_hash_table_methods_t *methods, unsigned long buckets);
void obj_hash_table_destroy(obj_hash_table_t *table);
obj_global_error_code_t obj_hash_table_add(obj_hash_table_t *table, void *key, void *value);
obj_global_error_code_t obj_hash_table_delete(obj_hash_table_t *table, const void *key);
obj_hash_table_entry_t *obj_hash_table_find(obj_hash_table_t *table, const void *key);
void *obj_hash_table_fetch_value(obj_hash_table_t *table, const void *key);
obj_global_error_code_t obj_hash_table_update(obj_hash_table_t *table, void *key, void *value, obj_bool_t upsert);
void obj_hash_table_stats(obj_hash_table_t *table);

/* default methods */
obj_uint64_t obj_hash_table_default_hash_func(const void *key);
void *obj_hash_table_default_key_dup(const void *key);
void *obj_hash_table_default_value_dup(const void *value);
int obj_hash_table_default_key_compare(const void *key1, const void *key2);
void obj_hash_table_default_key_free(void *key);
void obj_hash_table_default_value_free(void *value);

#endif  /* OBJ_HASH_H */