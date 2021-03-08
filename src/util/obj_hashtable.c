#include "obj_core.h"

/* a simple, unsafe hashtable implemention */

static int obj_hashtable_index(obj_uint64_t hash, int bucket_size);
static void obj_hashtable_resize(obj_hashtable_t *table);

/* hash functions */
static obj_uint8_t obj_hashtable_hash_function_seed[16];

obj_uint64_t obj_hashtable_hash_function(const void *key, int len) {
    return obj_siphash(key, len, obj_hashtable_hash_function_seed);
}

obj_hashtable_t *obj_hashtable_create(obj_hashtable_methods_t *methods) {
    obj_hashtable_t *table = obj_alloc(sizeof(obj_hashtable_t));
    if (table == NULL) {
        return NULL;
    }
    table->bucket_size = OBJ_HASHTABLE_BUCKET_INIT_SIZE;
    table->size = 0;
    table->methods = methods;
    table->bucket = (obj_hashtable_entry_t **)obj_alloc(table->bucket_size * sizeof(obj_hashtable_entry_t *));
    if (table->bucket == NULL) {
        obj_free(table);
        return NULL;
    }
    /* clear */
    obj_memset(table->bucket, 0, sizeof(obj_hashtable_entry_t *) * table->bucket_size);
    return table;
}

void obj_hashtable_destroy_static(obj_hashtable_t *table) {
    obj_assert(table);
    obj_assert(table->bucket);
    int i;
    obj_hashtable_entry_t *entry = NULL, *next_entry = NULL;
    for (i = 0; i < table->bucket_size; i++) {
        if ((entry = table->bucket[i]) == NULL) {
            continue;
        }
        while (entry) {
            next_entry = entry->next;
            obj_hashtable_free_key(table, entry);
            obj_hashtable_free_value(table, entry);
            obj_free(entry);
            entry = next_entry;
        }
    }
    obj_free(table->bucket);
}

void obj_hashtable_destroy(obj_hashtable_t *table) {
    obj_hashtable_destroy_static(table);
    obj_free(table);
}

static inline int obj_hashtable_index(obj_uint64_t hash, int bucket_size) {
    return hash & (bucket_size - 1);
}

static void obj_hashtable_resize(obj_hashtable_t *table) {
    int new_bucket_size = (table->bucket_size * 2 <= OBJ_HASHTABLE_BUCKET_MAX_SIZE ? table->bucket_size * 2 : OBJ_HASHTABLE_BUCKET_MAX_SIZE);
    if (new_bucket_size <= table->bucket_size) {
        return;
    }
    obj_hashtable_entry_t **new_bucket = (obj_hashtable_entry_t **)obj_alloc(new_bucket_size * sizeof(obj_hashtable_entry_t *));
    if (new_bucket == NULL) {
        return;
    }
    obj_memset(new_bucket, 0, new_bucket_size * sizeof(obj_hashtable_entry_t *));
    int i;
    obj_hashtable_entry_t *entry = NULL, *next_entry = NULL;
    obj_uint64_t hash;
    int index;
    for (i = 0; i < table->bucket_size; i++) {
        if ((entry = table->bucket[i]) == NULL) {
            continue;
        }
        while (entry) {
            next_entry = entry->next;
            /* rehash */
            hash = obj_hashtable_hash_key(table, entry->key);
            index = obj_hashtable_index(hash, new_bucket_size);
            entry->next = new_bucket[index];
            new_bucket[index] = entry;
            entry = next_entry;
        }
    }
    obj_free(table->bucket);
    table->bucket = new_bucket;
    table->bucket_size = new_bucket_size;
}

obj_global_error_code_t obj_hashtable_add(obj_hashtable_t *table, void *key, void *value) {
    obj_hashtable_entry_t *entry = NULL;
    int index;
    obj_uint64_t hash = obj_hashtable_hash_key(table, key);
    /* check for resize */
    if (table->size > table->bucket_size * OBJ_HASHTABLE_LOAD_FACTOR) {
        obj_hashtable_resize(table);
    }
    index = obj_hashtable_index(hash, table->bucket_size);
    entry = table->bucket[index];
    while (entry) {
        if (key == entry->key || obj_hashtable_compare_keys(table, key, entry->key) == 0) {
            return OBJ_HASHTABLE_CODE_DUP_KEY;
        }
        entry = entry->next;
    }
    
    entry = obj_alloc(sizeof(obj_hashtable_entry_t));
    if (entry == NULL) {
        return OBJ_HASHTABLE_CODE_NOMEM;
    }
    entry->next = table->bucket[index];
    table->bucket[index] = entry;
    obj_hashtable_set_key(table, entry, key);
    obj_hashtable_set_value(table, entry, value);
    ++table->size;
    return OBJ_CODE_OK;
}

obj_global_error_code_t obj_hashtable_delete(obj_hashtable_t *table, void *key, obj_bool_t nofree) {
    obj_hashtable_entry_t *entry, *prev_entry;
    obj_uint64_t hash;
    int index;
    hash = obj_hashtable_hash_key(table, key);
    index = obj_hashtable_index(hash, table->bucket_size);
    entry = table->bucket[index];
    prev_entry = NULL;
    while (entry) {
        if (key == entry->key || obj_hashtable_compare_keys(table, key, entry->key) == 0) {
            /* unlink from list */
            if (prev_entry) {
                prev_entry->next = entry->next;
            } else {
                table->bucket[index] = entry->next;
            }
            if (!nofree) {
                obj_hashtable_free_key(table, entry);
                obj_hashtable_free_value(table, entry);
                obj_free(entry);
            }
            table->size--;
            return OBJ_CODE_OK;
        }
        prev_entry = entry;
        entry = entry->next;
    }
    return OBJ_HASHTABLE_CODE_KEY_NOT_EXISTS;
}

obj_hashtable_entry_t *obj_hashtable_find(obj_hashtable_t *table, void *key) {
    obj_hashtable_entry_t *entry;
    obj_uint64_t hash;
    int index;
    hash = obj_hashtable_hash_key(table, key);
    index = obj_hashtable_index(hash, table->bucket_size);
    entry = table->bucket[index];
    while (entry) {
        if (key == entry->key || obj_hashtable_compare_keys(table, key, entry->key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

/* for check */
obj_bool_t obj_hashtable_is_empty(obj_hashtable_t *table) {
    int i;
    obj_hashtable_entry_t *entry;
    for (i = 0; i < table->bucket_size; i++) {
        entry = table->bucket[i];
        if (entry) {
            return false;
        }
    }
    return true;
}

void obj_hashtable_default_key_free(void *key) {
    obj_free(key);
}

void obj_hashtable_default_value_free(void *value) {
    obj_free(value);
}