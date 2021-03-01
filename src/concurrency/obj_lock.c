#include "obj_core.h"

/* global lock manager */
obj_lock_manager_t *g_lock_manager;
/* resource id of global */
obj_lock_resource_id_t g_resource_id_global;
static obj_prealloc_map_methods_t obj_resource_id_lock_head_map_methods = {
    obj_resource_id_lock_head_map_hash_func,
    obj_resource_id_lock_head_map_key_compare,
    NULL,
    obj_lock_lock_head_destroy,
    obj_resource_id_lock_head_map_key_get,
    obj_resource_id_lock_head_map_value_get,
    obj_resource_id_lock_head_map_key_set,
    obj_resource_id_lock_head_map_value_set,
    NULL,
    NULL
};

static obj_uint64_t obj_resource_id_lock_head_map_hash_func(const void *key) {
    return obj_prealloc_map_hash_function(key, sizeof(obj_lock_resource_id_t));
}

/* compare resource id */
static int obj_resource_id_lock_head_map_key_compare(const void *key1, const void *key2) {
    obj_lock_resource_id_t *id1 = (obj_lock_resource_id_t *)key1;
    obj_lock_resource_id_t *id2 = (obj_lock_resource_id_t *)key2;
    return (*id1) - (*id2);
}

static void *obj_resource_id_lock_head_map_key_get(void *data) {
    obj_resource_id_lock_head_pair_t *pair = (obj_resource_id_lock_head_pair_t *)data;
    return &pair->resource_id;
}

static void *obj_resource_id_lock_head_map_value_get(void *data) {
    obj_resource_id_lock_head_pair_t *pair = (obj_resource_id_lock_head_pair_t *)data;
    return &pair->lock_head;
}

static void obj_resource_id_lock_head_map_key_set(void *data, void *key) {
    obj_resource_id_lock_head_pair_t *pair = (obj_resource_id_lock_head_pair_t *)data;
    obj_memcpy(&pair->resource_id, key, sizeof(obj_lock_resource_id_t));
}

static void obj_resource_id_lock_head_map_value_set(void *data, void *value) {
    obj_resource_id_lock_head_pair_t *pair = (obj_resource_id_lock_head_pair_t *)data;
    obj_memcpy(&pair->lock_head, value, sizeof(obj_lock_head_t *));
}


static int obj_lock_conflict_table[] = {
    /* OBJ_LOCK_MODE_NONE */
    0,
    /* OBJ_LOCK_MODE_IS */
    (1 << OBJ_LOCK_MODE_X),
    /* OBJ_LOCK_MODE_IX */
    (1 << OBJ_LOCK_MODE_S) | (1 << OBJ_LOCK_MODE_X),
    /* OBJ_LOCK_MODE_S */
    (1 << OBJ_LOCK_MODE_IX) | (1 << OBJ_LOCK_MODE_X),
    /* OBJ_LOCK_MODE_X */
    (1 << OBJ_LOCK_MODE_S) | (1 << OBJ_LOCK_MODE_X) | (1 << OBJ_LOCK_MODE_IS) | (1 << OBJ_LOCK_MODE_IX)
};

/* intent lock modes */
static int obj_lock_intent_modes = (1 << OBJ_LOCK_MODE_IS) | (1 << OBJ_LOCK_MODE_IX);

/* seed for hash */
static obj_uint8_t obj_lock_string_hash_seed[16];

static inline obj_bool_t obj_lock_conflict(obj_lock_mode_t new_mode, int existing_mode) {
    return (obj_lock_conflict_table[new_mode] & existing_mode) != 0;
}

static inline int obj_lock_mode_mask(obj_lock_mode_t mode) {
    return 1 << mode;
}

inline obj_bool_t obj_lock_is_shared_lock_mode(obj_lock_mode_t mode) {
    return (mode == OBJ_LOCK_MODE_IS || mode == OBJ_LOCK_MODE_S);
}

/* if mode is covered by cover_mode */
inline obj_bool_t obj_lock_is_mode_covered(obj_lock_mode_t mode, obj_lock_mode_t cover_mode) {
    return (obj_lock_conflict_table[cover_mode] | obj_lock_conflict_table[mode]) == obj_lock_conflict_table[cover_mode];
}

/* hash string */
static inline obj_uint64_t obj_lock_hash_string(const void *key, int len) {
    return obj_siphash(key, len, obj_lock_string_hash_seed);
}

/* init global lock manager */
void obj_global_lock_manager_init() {
    g_lock_manager = obj_lock_manager_create();
    if (g_lock_manager == NULL) {
        fprintf(stderr, "can't init global lock manager\n");
        exit(1);
    }
    /* set global resource id */
    g_resource_id_global = obj_lock_resource_id_from_hashid(OBJ_LOCK_RESOURCE_GLOBAL, OBJ_LOCK_RESOURCE_ID_SINGLETON_GLOBAL);
}

/* create lock manager */
static obj_lock_manager_t *obj_lock_manager_create() {
    obj_lock_manager_t *lock_manager = obj_alloc(sizeof(obj_lock_manager_t));
    if (lock_manager == NULL) {
        goto clean;
    }
    lock_manager->num_buckets = OBJ_LOCK_BUCKET_NUM;
    lock_manager->buckets = obj_alloc(OBJ_LOCK_BUCKET_NUM * sizeof(obj_lock_bucket_t));
    if (lock_manager->buckets == NULL) {
        goto clean;
    }
    int i;
    for (i = 0; i < OBJ_LOCK_BUCKET_NUM; i++) {
        if (!obj_lock_bucket_init(&lock_manager->buckets[i])) {
            goto clean;
        }
    }
    return lock_manager;
clean:
    if (lock_manager) {
        if (lock_manager->buckets) {
            for (i = 0; i < OBJ_LOCK_BUCKET_NUM; i++) {
                obj_lock_bucket_destroy_static(&lock_manager->buckets[i]);
            }
            obj_free(lock_manager->buckets);
        }
        obj_free(lock_manager);
    }
    return NULL;
}

/* destroy a lock manager */
static void obj_lock_manager_destroy(obj_lock_manager_t *lock_manager) {
    int i;
    obj_lock_cleanup_unused_locks(lock_manager);
    for (i = 0; i < OBJ_LOCK_BUCKET_NUM; i++) {
        obj_assert(obj_prealloc_map_is_empty(&(lock_manager->buckets[i].map)));
        obj_lock_bucket_destroy_static(&lock_manager->buckets[i]);
    }
    obj_free(lock_manager->buckets);
    obj_free(lock_manager);
}

/* init a lock bucket */
static obj_bool_t obj_lock_bucket_init(obj_lock_bucket_t *bucket) {
    obj_assert(bucket);
    pthread_mutex_init(&bucket->mutex, NULL);
    if (!obj_prealloc_map_init(&bucket->map, &obj_resource_id_lock_head_map_methods, sizeof(obj_resource_id_lock_head_pair_t *))) {
        return false;
    }
    return true;
}

/* don't free bucket itself */
static void obj_lock_bucket_destroy_static(obj_lock_bucket_t *bucket) {
    pthread_mutex_destroy(&bucket->mutex);
    obj_prealloc_map_destroy_static(&bucket->map);
}

static void obj_lock_lock_head_destroy(obj_lock_head_t *lock_head) {
    obj_free(lock_head);
}

/* put a lock request in the lockhead's conflict queue or granted queue */
obj_lock_result_t obj_lock_new_request(obj_lock_request_t *request, obj_lock_head_t *lock_head) {
    request->lock_head = lock_head;
    /* to avoid starving */
    if (obj_lock_conflict(request->mode, lock_head->granted_mode) || (!lock_head->compatible_first_count && obj_lock_conflict(request->mode, lock_head->conflict_mode))) {
        request->status = OBJ_LOCK_REQUEST_STATUS_WAITING;
        if (request->enqueue_front) {
            OBJ_EMBEDDED_LIST_ADD_FIRST(list, lock_head->conflict_list, request);
        } else {
            OBJ_EMBEDDED_LIST_ADD_LAST(list, lock_head->conflict_list, request);
        }
        obj_lock_incr_conflict_mode_count(lock_head, request->mode);
        return OBJ_LOCK_RESULT_WAITING;
    }
    request->status = OBJ_LOCK_REQUEST_STATUS_GRANTED;
    OBJ_EMBEDDED_LIST_ADD_LAST(list, lock_head->granted_list, request);
    obj_lock_incr_granted_mode_count(lock_head, request->mode);
    if (request->compatible_first) {
        lock_head->compatible_first_count++;
    }
    /* success */
    return OBJ_LOCK_RESULT_OK;
}

void obj_lock_request_init(obj_lock_request_t *request, obj_locker_t *locker, obj_lock_grant_notify_t *notify) {
    request->enqueue_front = false;
    request->compatible_first = false;
    request->mode = OBJ_LOCK_MODE_NONE;
    request->recursive_count = 1;
    request->status = OBJ_LOCK_REQUEST_STATUS_NEW;
    request->convert_mode = OBJ_LOCK_MODE_NONE;
    request->lock_head = NULL;
    request->notify = notify;
    request->locker = locker;
}

static inline void obj_lock_incr_granted_mode_count(obj_lock_head_t *lock_head, obj_lock_mode_t mode) {
    obj_assert(lock_head->granted_count[mode] >= 0);
    if (++lock_head->granted_count[mode] == 1) {
        lock_head->granted_mode |= obj_lock_mode_mask(mode);
    }
}

static inline void obj_lock_decr_granted_mode_count(obj_lock_head_t *lock_head, obj_lock_mode_t mode) {
    obj_assert(lock_head->granted_count[mode] >= 1);
    if (--lock_head->conflict_count[mode] == 0) {
        lock_head->granted_mode &= ~obj_lock_mode_mask(mode);
    }
}

static inline void obj_lock_incr_conflict_mode_count(obj_lock_head_t *lock_head, obj_lock_mode_t mode) {
    obj_assert(lock_head->conflict_count[mode] >= 0);
    if (++lock_head->conflict_count[mode] == 1) {
        lock_head->conflict_mode |= obj_lock_mode_mask(mode);
    }
}

static inline void obj_lock_decr_conflict_mode_count(obj_lock_head_t *lock_head, obj_lock_mode_t mode) {
    obj_assert(lock_head->conflict_count[mode] >= 1);
    if (--lock_head->conflict_count[mode] == 0) {
        lock_head->conflict_mode &= ~obj_lock_mode_mask(mode);
    }
}

inline obj_lock_resource_id_t obj_lock_resource_id(obj_lock_resource_type_t type, char *str, int len) {
    obj_uint64_t hash = obj_lock_hash_string(str, len);
    return obj_lock_resource_id_fullhash(type, hash);
}

inline obj_lock_resource_id_t obj_lock_resource_id_from_hashid(obj_lock_resource_type_t type, obj_uint64_t hash) {
    return obj_lock_resource_id_fullhash(type, hash);
}

/* hash with resource type */
static inline obj_lock_resource_id_t obj_lock_resource_id_fullhash(obj_lock_resource_type_t type, obj_uint64_t hash) {
    return (type << (64 - OBJ_LOCK_RESOURCE_TYPE_BITS)) + (hash & ((obj_uint64_t)OBJ_UINT64_MAX >> OBJ_LOCK_RESOURCE_TYPE_BITS));
}

inline obj_lock_resource_type_t obj_lock_resource_id_get_type(obj_lock_resource_id_t resource_id) {
    return resource_id >> (64 - OBJ_LOCK_RESOURCE_TYPE_BITS);
}

/* lock */
obj_lock_result_t obj_lock_lock(obj_lock_manager_t *lock_manager, obj_lock_resource_id_t resource_id, obj_lock_request_t *request, obj_lock_mode_t mode) {
    obj_assert(request->status == OBJ_LOCK_REQUEST_STATUS_NEW);
    obj_assert(request->recursive_count == 1);
    request->mode = mode;
    obj_lock_bucket_t *bucket = obj_lock_manager_get_bucket(lock_manager, resource_id);
    obj_assert(bucket);
    /* protect the bucket */
    pthread_mutex_lock(&bucket->mutex);
    obj_lock_head_t *lock_head = obj_lock_find_or_insert(bucket, resource_id);
    obj_lock_result_t result = obj_lock_new_request(request, lock_head);
    pthread_mutex_unlock(&bucket->mutex);
    return result;
}

/* convert */
obj_lock_result_t obj_lock_convert(obj_lock_manager_t *lock_manager, obj_lock_resource_id_t resource_id, obj_lock_request_t *request, obj_lock_mode_t new_mode) {
    /* already hold the lock in some mode */
    obj_assert(request->status == OBJ_LOCK_REQUEST_STATUS_GRANTED);
    obj_assert(request->recursive_count > 0);
    request->recursive_count++;
    /* acquire same lock, don't need lock */
    if ((obj_lock_conflict_table[request->mode] | obj_lock_conflict_table[new_mode]) == obj_lock_conflict_table[request->mode]) {
        return OBJ_LOCK_RESULT_OK;
    }
    /* more strict */
    obj_assert((obj_lock_conflict_table[request->mode] | obj_lock_conflict_table[new_mode]) == obj_lock_conflict_table[request->mode]);
    obj_lock_bucket_t *bucket = obj_lock_manager_get_bucket(lock_manager, resource_id);
    pthread_mutex_lock(&bucket->mutex);


    pthread_mutex_unlock(&bucket->mutex);
}

void obj_lock_downgrade(obj_lock_manager_t *lock_manager, obj_lock_request_t *request, obj_lock_mode_t new_mode) {
    obj_assert(request->lock_head);
    obj_assert(request->status == OBJ_LOCK_REQUEST_STATUS_GRANTED);
    obj_assert(request->recursive_count > 0);
    /* assert request is more strict */
    obj_assert((obj_lock_conflict_table[request->mode] | obj_lock_conflict_table[new_mode]) == obj_lock_conflict_table[request->mode]);
    obj_lock_head_t *lock_head = request->lock_head;
    obj_lock_bucket_t *bucket = obj_lock_manager_get_bucket(lock_manager, lock_head->resource_id);
    pthread_mutex_lock(&bucket->mutex);
    obj_lock_incr_granted_mode_count(lock_head, new_mode);
    obj_lock_decr_granted_mode_count(lock_head, request->mode);
    request->mode = new_mode;
    obj_lock_on_lock_mode_changed(lock_manager, lock_head, true);
    pthread_mutex_unlock(&bucket->mutex);
}

/* unlock */
obj_bool_t obj_lock_unlock(obj_lock_manager_t *lock_manager, obj_lock_request_t *request) {
    obj_assert(request->recursive_count > 0);
    request->recursive_count--;
    if ((request->status == OBJ_LOCK_REQUEST_STATUS_GRANTED) && request->recursive_count > 0) {
        return false;
    }
    obj_lock_head_t *lock_head = request->lock_head;
    obj_assert(lock_head);
    obj_lock_bucket_t *bucket = obj_lock_manager_get_bucket(lock_manager, lock_head->resource_id);
    obj_assert(bucket);
    pthread_mutex_lock(&bucket->mutex);
    if (request->status == OBJ_LOCK_REQUEST_STATUS_GRANTED) {
        /* release a currently held lock, most common path */
        /* remove from granted list */
        OBJ_EMBEDDED_LIST_REMOVE(list, lock_head->granted_list, request);
        obj_lock_decr_granted_mode_count(lock_head, request->mode);
        if (request->compatible_first) {
            obj_assert(lock_head->compatible_first_count > 0);
            lock_head->compatible_first_count--;
            obj_assert(lock_head->compatible_first_count == 0 || !OBJ_EMBEDDED_LIST_IS_EMPTY(lock_head->granted_list));
        }
        /* if granted list is empty now, check conflict queue */
        obj_lock_on_lock_mode_changed(lock_manager, lock_head, lock_head->granted_count[request->mode] == 0);
    } else if (request->status == OBJ_LOCK_REQUEST_STATUS_WAITING) {
        obj_assert(request->recursive_count == 0);
        OBJ_EMBEDDED_LIST_REMOVE(list, lock_head->conflict_list, request);
        obj_lock_decr_conflict_mode_count(lock_head, request->mode);
        /* check conflict queue */
        obj_lock_on_lock_mode_changed(lock_manager, lock_head, true);
    } else if (request->status == OBJ_LOCK_REQUEST_STATUS_CONVERTING) {
        /* cancel a pending convert request */
        obj_assert(request->recursive_count > 0);
        obj_assert(lock_head->conversion_count > 0);
        /* cancel the conversion reqeust */
        request->status = OBJ_LOCK_REQUEST_STATUS_GRANTED;
        lock_head->conversion_count--;
        obj_lock_decr_granted_mode_count(lock_head, request->convert_mode);
        request->convert_mode = OBJ_LOCK_MODE_NONE;
        obj_lock_on_lock_mode_changed(lock_manager, lock_head, lock_head->granted_count[request->convert_mode] == 0);
    } else {
        /* can't reach here */
        obj_assert(0);
    }
    pthread_mutex_unlock(&bucket->mutex);
    return request->recursive_count > 0;
}

obj_lock_head_t *obj_lock_find_or_insert(obj_lock_bucket_t *bucket, obj_lock_resource_id_t resource_id) {
    obj_lock_head_t *lock_head;

    return lock_head;
}

/* get bucket according to resource id */
obj_lock_bucket_t *obj_lock_manager_get_bucket(obj_lock_manager_t *lock_manager, obj_lock_resource_id_t resource_id) {
    return &lock_manager->buckets[resource_id % OBJ_LOCK_BUCKET_NUM];
}

/* for debug */
void obj_lock_manager_dump(obj_lock_manager_t *lock_manager) {

}

void obj_lock_cleanup_unused_locks(obj_lock_manager_t *lock_manager) {
    int i;
    obj_lock_bucket_t *bucket;
    for (i = 0; i < lock_manager->num_buckets; i++) {
        bucket = &lock_manager->buckets[i];
        pthread_mutex_lock(&bucket->mutex);
        obj_lock_cleanup_unused_locks_in_bucket(bucket);
        pthread_mutex_unlock(&bucket->mutex);
    }
}

void obj_lock_cleanup_unused_locks_in_bucket(obj_lock_bucket_t *bucket) {
    int i;
    obj_prealloc_map_entry_t *entry = NULL;
    obj_prealloc_map_entry_t *prev_entry = NULL;
    obj_prealloc_map_t *map = &bucket->map;
    obj_lock_head_t *lock_head;
    for (i = 0; i < map->bucket_size; i++) {
        entry = map->bucket[i];
        prev_entry = NULL;
        while (entry != NULL) {
            lock_head = *((obj_lock_head_t **)obj_prealloc_map_get_value(map, entry));
            if (lock_head->granted_mode == 0) {
                obj_assert(lock_head->granted_mode == 0);
                obj_assert(OBJ_EMBEDDED_LIST_IS_EMPTY(lock_head->granted_list));
                obj_assert(lock_head->conflict_mode == 0);
                obj_assert(OBJ_EMBEDDED_LIST_IS_EMPTY(lock_head->conflict_list));
                obj_assert(lock_head->conversion_count == 0);
                obj_assert(lock_head->compatible_first_count == 0);
                /* delete entry */
                if (prev_entry != NULL) {
                    prev_entry->next = entry->next;
                } else {
                    map->bucket[i] = entry->next;
                }
                obj_lock_lock_head_destroy(lock_head);
                obj_free(entry);
            }
            prev_entry = entry;
            entry = entry->next;
        }
    }
}

/* call back function */
void obj_lock_on_lock_mode_changed(obj_lock_manager_t *lock_manager, obj_lock_head_t *lock_head, obj_bool_t check_conflict_queue) {
    /* unblock any converting requests. because conversions are still counted as granted and are on the granted queue */
    obj_lock_request_t *curr = OBJ_EMBEDDED_LIST_GET_FIRST(lock_head->granted_list);
    int i;
    while (curr != NULL && (lock_head->conversion_count > 0)) {
        if (curr->status == OBJ_LOCK_REQUEST_STATUS_CONVERTING) {
            obj_assert(curr->convert_mode != 0);
            int granted_modes_without_current_request = 0;
            for (i = 1; i < OBJ_LOCK_MODE_COUNT; i++) {
                int curr_request_holds = (curr->mode == i ? 1 : 0);
                int curr_request_waits = (curr->convert_mode == i ? 1 : 0);
                /* can't both hold and wait on the same lock mode */
                obj_assert(curr_request_holds + curr_request_waits <= 1);
                if (lock_head->granted_count[i] > (curr_request_holds + curr_request_waits)) {
                    granted_modes_without_current_request |= obj_lock_mode_mask(i);
                }
            }
            /* convert now if possible */
            if (!obj_lock_conflict(curr->convert_mode, granted_modes_without_current_request)) {
                lock_head->conversion_count--;
                obj_lock_decr_granted_mode_count(lock_head, curr->mode);
                curr->status = OBJ_LOCK_REQUEST_STATUS_GRANTED;
                curr->mode = curr->convert_mode;
                curr->convert_mode = OBJ_LOCK_MODE_NONE;
                obj_lock_grant_notify_notify(lock_head->resource_id, OBJ_LOCK_RESULT_OK);
            }
        }
        curr = OBJ_EMBEDDED_LIST_GET_NEXT(list, curr);
    }
    /* grant any conflicting requests, which might now be unblocked */
    curr = OBJ_EMBEDDED_LIST_GET_FIRST(lock_head->conflict_list);
    obj_lock_request_t *curr_next = NULL;
    /* set on enabling compatiblefirst mode */
    obj_bool_t newly_compatible_first = false;
    while (curr != NULL && check_conflict_queue) {
        obj_assert(curr->status == OBJ_LOCK_REQUEST_STATUS_WAITING);
        curr_next = OBJ_EMBEDDED_LIST_GET_NEXT(list, curr);
        /* check conflict */
        if (obj_lock_conflict(curr->mode, lock_head->granted_mode)) {
            if (!OBJ_EMBEDDED_LIST_GET_PREV(list, curr) && !newly_compatible_first) {
                break;
            }
            continue;
        }
        curr->status = OBJ_LOCK_REQUEST_STATUS_GRANTED;
        /* remove from the conflict list */
        OBJ_EMBEDDED_LIST_REMOVE(list, lock_head->conflict_list, curr);
        obj_lock_decr_conflict_mode_count(lock_head, curr->mode);
        /* add to the granted list */
        OBJ_EMBEDDED_LIST_ADD_LAST(list, lock_head->granted_list, curr);
        obj_lock_incr_granted_mode_count(lock_head, curr->mode);
        
        if (curr->compatible_first) {
            newly_compatible_first |= (lock_head->compatible_first_count++ == 0);
        }
        /* notify waiter */
        obj_lock_grant_notify_notify(curr->notify, OBJ_LOCK_RESULT_OK);
        /* nothing is compatible with a OBJ_LOCK_MODE_X */
        if (curr->mode == OBJ_LOCK_MODE_X) {
            break;
        }
        curr = curr_next;
    }
}

void obj_lock_grant_notify_init(obj_lock_grant_notify_t *notify) {
    pthread_mutex_init(&notify->mutex, NULL);
    pthread_cond_init(&notify->cond, NULL);
    notify->result = OBJ_LOCK_RESULT_COUNT;
}

void obj_lock_grant_notify_clear(obj_lock_grant_notify_t *notify) {
    /* set invalid */
    notify->result = OBJ_LOCK_RESULT_COUNT;
}

void obj_lock_grant_notify_destroy(obj_lock_grant_notify_t *notify) {
    pthread_mutex_destroy(&notify->mutex);
    pthread_cond_destroy(&notify->cond);
}

/*
obj_lock_result_t obj_lock_grant_notify_wait(obj_lock_grant_notify_t *notify) {
    pthread_mutex_lock(&notify->mutex);
    while (notify->result != OBJ_LOCK_RESULT_COUNT) {
        pthread_cond_wait(&notify->cond, &notify->mutex);
    }
    pthread_mutex_unlock(&notify->mutex);
}
*/

/* wait milliseconds */
obj_lock_result_t obj_lock_grant_notify_timed_wait(obj_lock_grant_notify_t *notify, obj_duration_msecond wait_time) {
    int wait_res;
    struct timeval tv;
    struct timespec deadline;
    gettimeofday(&tv, NULL);
    long nsec = tv.tv_usec * 1000 + (wait_time % 1000) * 1000000;
    deadline.tv_sec = tv.tv_sec + nsec / 1000000000 + wait_time / 1000;
    deadline.tv_nsec = nsec % 1000000000;
    pthread_mutex_lock(&notify->mutex);
    while (notify->result == OBJ_LOCK_RESULT_COUNT) {
        wait_res = pthread_cond_timedwait(&notify->cond, &notify->mutex, &deadline);
        if (wait_res == 0) {
            continue;
        } else  if (wait_res == ETIMEDOUT) {
            /* timeout */
            pthread_mutex_unlock(&notify->mutex);
            return OBJ_LOCK_RESULT_TIMEOUT;
        } else {
            /* unexpected error */
            fprintf(stderr, "unexpected error occurred during wait \n");
            pthread_mutex_unlock(&notify->mutex);
            return OBJ_LOCK_RESULT_INTERNAL_ERROR;
        }
    }
    pthread_mutex_unlock(&notify->mutex);
    return OBJ_LOCK_RESULT_OK;
}

/* notify all waiters */
void obj_lock_grant_notify_notify(obj_lock_grant_notify_t *notify, obj_lock_result_t result) {
    pthread_mutex_lock(&notify->mutex);
    notify->result = result;
    pthread_cond_broadcast(&notify->cond);
    pthread_mutex_unlock(&notify->mutex);
}

