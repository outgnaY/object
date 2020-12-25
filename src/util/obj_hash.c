#include "util/obj_hash.h"
#include "mem/obj_mem.h"

static unsigned long obj_hash_table_next_power(unsigned long size);
static void obj_hash_table_reset(obj_hash_table_t *table);
static obj_global_error_code_t obj_hash_table_generic_delete(obj_hash_table_t *table, const void *key, obj_bool_t nofree);

/* hash functions */
static obj_uint8_t obj_hash_table_hash_function_seed[16];
obj_uint64_t obj_siphash(const obj_uint8_t *in, const obj_size_t inlen, const obj_uint8_t *k);

obj_uint64_t obj_hash_function(const void *key, int len) {
    return obj_siphash(key, len, obj_hash_table_hash_function_seed);
}

static unsigned long obj_hash_table_next_power(unsigned long size) {
    unsigned long i = OBJ_HASH_TABLE_MIN_SIZE;
    if (size >= OBJ_INT32_MAX) {
        return OBJ_INT32_MAX + 1lu;
    }
    while (true) {
        if (i >= size) {
            return i;
        }
        i *= 2;
    }
}

/* reset the hash table, only called by obj_hash_table_clear */
static void obj_hash_table_reset(obj_hash_table_t *table) {
    table->table = NULL;
    table->used = 0;
    table->mutexes = NULL;
    table->n_mutexes = 0;
    table->n_buckets = 0;
    table->mask = 0;
}



/* create a hash table */
obj_hash_table_t *obj_hash_table_create(obj_hash_table_methods_t *methods, unsigned long buckets) {
    obj_hash_table_t *table = obj_alloc(sizeof(obj_hash_table_t));
    unsigned long n_mutexes;
    unsigned long i;
    if (table == NULL) {
        return NULL;
    }
    if (buckets < OBJ_HASH_TABLE_MIN_SIZE) {
        buckets = OBJ_HASH_TABLE_MIN_SIZE;
    }
    buckets = obj_hash_table_next_power(buckets);
    /* allocate buckets */
    if ((table->table = (obj_hash_table_entry_t **)obj_alloc(buckets * sizeof(obj_hash_table_entry_t *))) == NULL) {
        return NULL;
    }
    table->n_buckets = buckets;
    for (i = 0; i < buckets; i++) {
        table->table[i] = NULL;
    }
    if (buckets <= OBJ_HASH_TABLE_SEGMENTS) {
        n_mutexes = buckets;
    } else {
        n_mutexes = OBJ_HASH_TABLE_SEGMENTS;
    }
    table->n_mutexes = n_mutexes;
    if ((table->mutexes = (pthread_mutex_t *)obj_alloc(n_mutexes * sizeof(pthread_mutex_t))) == NULL) {
        obj_free(table->table);
        return NULL;
    }
    if ((table->used_mutex = (pthread_mutex_t *)obj_alloc(sizeof(pthread_mutex_t))) == NULL) {
        obj_free(table->table);
        obj_free(table->mutexes);
    }
    /* init mutexes */
    for (i = 0; i < n_mutexes; i++) {
        pthread_mutex_init(&table->mutexes[i], NULL);
    }
    table->methods = methods;
    table->mask = buckets - 1;
    return table;
}


/* clear a hash table */
void obj_hash_table_clear(obj_hash_table_t *table) {
    unsigned long i;
    obj_hash_table_entry_t *entry, *next_entry;
    pthread_mutex_t *lock;
    for (i = 0; i < table->n_buckets; i++) {
        lock = obj_hash_table_find_lock(table, i);
        pthread_mutex_lock(lock);
        if ((entry = table->table[i]) == NULL) {
            pthread_mutex_unlock(lock);
            continue;
        }
        while(entry) {
            next_entry = entry->next;
            obj_hash_table_free_key(table, entry);
            obj_hash_table_free_value(table, entry);
            obj_free(entry);
            entry = next_entry;
            obj_atomic_decr(table->used, 1);
        }
        table->table[i] = NULL;
        pthread_mutex_unlock(lock);
    }
    obj_free(table->table);
    obj_hash_table_reset(table);
}



/* add */
obj_global_error_code_t obj_hash_table_add(obj_hash_table_t *table, void *key, void *value) {
    obj_hash_table_entry_t *entry = NULL;
    unsigned long index;
    obj_uint64_t hash = obj_hash_table_hash_key(table, key);
    index = hash & table->mask;
    pthread_mutex_t *lock = obj_hash_table_find_lock(table, index);
    pthread_mutex_lock(lock);
    entry = table->table[index];
    while (entry) {
        if (key == entry->key || obj_hash_table_compare_keys(table, key, entry->key) == 0) {
            pthread_mutex_unlock(lock);
            return OBJ_CODE_HASH_TABLE_DUP_KEY;
        }
        entry = entry->next;
    }
    entry = obj_alloc(sizeof(obj_hash_table_entry_t));
    if (entry == NULL) {
        return OBJ_CODE_HASH_TABLE_NOMEM;
    }
    entry->next = table->table[index];
    // printf("next %p\n", entry->next);
    table->table[index] = entry;
    obj_atomic_incr(table->used, 1);
    obj_hash_table_set_key(table, entry, key);
    obj_hash_table_set_value(table, entry, value);
    pthread_mutex_unlock(lock);
    return OBJ_CODE_OK;
}

/* delete */
obj_global_error_code_t obj_hash_table_delete(obj_hash_table_t *table, const void *key) {
    return obj_hash_table_generic_delete(table, key, false);
}

static obj_global_error_code_t obj_hash_table_generic_delete(obj_hash_table_t *table, const void *key, obj_bool_t nofree) {
    obj_hash_table_entry_t *entry, *prev_entry;
    obj_uint64_t hash;
    unsigned long index;
    hash = obj_hash_table_hash_key(table, key);
    index = hash & table->mask;
    pthread_mutex_t *lock = obj_hash_table_find_lock(table, index);
    pthread_mutex_lock(lock);
    entry = table->table[index];
    prev_entry = NULL;
    while (entry) {
        if (key == entry->key || obj_hash_table_compare_keys(table, key, entry->key) == 0) {
            /* unlink from list */
            if (prev_entry) {
                prev_entry->next = entry->next;
            } else {
                table->table[index] = entry->next;
            }
            /* need free */
            if (!nofree) {
                obj_hash_table_free_key(table, entry);
                obj_hash_table_free_value(table, entry);
                obj_free(entry);
            }
            obj_atomic_decr(table->used, 1);
            pthread_mutex_unlock(lock);
            return OBJ_CODE_OK;
        }
        prev_entry = entry;
        entry = entry->next;
    }
    pthread_mutex_unlock(lock);
    return OBJ_CODE_HASH_TABLE_KEY_NOT_EXISTS;
}



/* find */
obj_hash_table_entry_t *obj_hash_table_find(obj_hash_table_t *table, const void *key) {
    obj_hash_table_entry_t *entry;
    obj_uint64_t hash;
    unsigned long index;
    if (obj_hash_table_used(table) == 0) {
        return NULL;
    }
    hash = obj_hash_table_hash_key(table, key);
    index = hash & table->mask;
    pthread_mutex_t *lock = obj_hash_table_find_lock(table, index);
    pthread_mutex_lock(lock);
    entry = table->table[index];
    while (entry) {
        if (key == entry->key || obj_hash_table_compare_keys(table, key, entry->key) == 0) {
            pthread_mutex_unlock(lock);
            return entry;
        }
        entry = entry->next;
    }
    pthread_mutex_unlock(lock);
    return NULL;
}

/* fetch value */
void *obj_hash_table_fetch_value(obj_hash_table_t *table, const void *key) {
    obj_hash_table_entry_t *entry;
    entry = obj_hash_table_find(table, key);
    return entry ? entry->value : NULL;
}

/* update */
obj_global_error_code_t obj_hash_table_update(obj_hash_table_t *table, void *key, void *value, obj_bool_t upsert) {
    obj_hash_table_entry_t *entry;
    obj_uint64_t hash;
    unsigned long index;
    hash = obj_hash_table_hash_key(table, key);
    index = hash & table->mask;
    pthread_mutex_t *lock = obj_hash_table_find_lock(table, index);
    pthread_mutex_lock(lock);
    entry = table->table[index];
    while (entry) {
        if (key == entry->key || obj_hash_table_compare_keys(table, key, entry->key) == 0) {
            obj_hash_table_free_value(table, entry);
            obj_hash_table_set_value(table, entry, value);
            pthread_mutex_unlock(lock);
            return OBJ_CODE_OK;
        }
    }
    /* not found, check if upsert */
    if (upsert) {
        entry = obj_alloc(sizeof(obj_hash_table_entry_t));
        if (entry == NULL) {
            return OBJ_CODE_HASH_TABLE_NOMEM;
        }
        entry->next = table->table[index];
        table->table[index] = entry;
        obj_atomic_incr(table->used, 1);
        obj_hash_table_set_key(table, entry, key);
        obj_hash_table_set_value(table, entry, value);
        pthread_mutex_unlock(lock);
        return OBJ_CODE_OK;
    }
    pthread_mutex_unlock(lock);
    return OBJ_CODE_HASH_TABLE_KEY_NOT_EXISTS;
}

/* stats */
void obj_hash_table_stats(obj_hash_table_t *table) {
    obj_hash_table_entry_t *entry;
    unsigned long i;
    pthread_mutex_t *lock;
    unsigned long cnt = 0;
    unsigned long sum = 0;
    for (i = 0; i < table->n_buckets; i++) {
        printf("bucket %lu ", i);
        lock = obj_hash_table_find_lock(table, i);
        pthread_mutex_lock(lock);
        if ((entry = table->table[i]) == NULL) {
            pthread_mutex_unlock(lock);
            continue;
        }
        while (entry) {
            cnt++;
            entry = entry->next;
        }
        sum += cnt;
        printf("cnt %lu\n", cnt);
        cnt = 0;
        pthread_mutex_unlock(lock);
    }
    printf("table->used = %lu, sum = %lu\n", table->used, sum);
}

obj_uint64_t obj_hash_table_default_hash_func(const void *key) {
    return obj_hash_function((unsigned char *)key, obj_strlen(key));
}

/* default string */
void *obj_hash_table_default_key_dup(const void *key) {
    int len = obj_strlen(key);
    char *dup = (char *)obj_alloc(len + 1);
    obj_memcpy(dup, key, len);
    dup[len] = '\0';
    return dup;
}

/* default string */
void *obj_hash_table_default_value_dup(const void *value) {
    int len = obj_strlen(value);
    char *dup = (char *)obj_alloc(len + 1);
    obj_memcpy(dup, value, len);
    dup[len] = '\0';
    return dup;
}

int obj_hash_table_default_key_compare(const void *key1, const void *key2) {
    obj_assert(key1 != NULL && key2 != NULL);
    return obj_strcmp(key1, key2);
}

void obj_hash_table_default_key_free(void *key) {
    obj_free(key);
}

void obj_hash_table_default_value_free(void *value) {
    obj_free(value);
}


