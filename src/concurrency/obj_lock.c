#include "obj_core.h"

/* global lock manager */
obj_lock_manager_t *g_lock_manager;

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
        obj_lock_bucket_init(&lock_manager->buckets[i]);
    }
    return lock_manager;
clean:
    obj_lock_manager_destroy(lock_manager);
    return NULL;
}

/* destroy a lock manager */
static void obj_lock_manager_destroy(obj_lock_manager_t *lock_manager) {
    /* TODO clean unused locks */
    int i;
    if (lock_manager) {
        if (lock_manager->buckets) {
            for (i = 0; i < OBJ_LOCK_BUCKET_NUM; i++) {
                obj_lock_bucket_destroy(&lock_manager->buckets[i]);
            }
            obj_free(lock_manager->buckets);
        }
        obj_free(lock_manager);
    }
}

/* init a lock bucket */
static void obj_lock_bucket_init(obj_lock_bucket_t *bucket) {
    obj_assert(bucket);
    pthread_mutex_init(&bucket->mutex, NULL);
}

/* don't free bucket itself */
static void obj_lock_bucket_destroy(obj_lock_bucket_t *bucket) {
    pthread_mutex_destroy(&bucket->mutex);
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

void obj_lock_request_init(obj_lock_request_t *request) {
    request->enqueue_front = false;
    request->compatible_first = false;
    request->mode = OBJ_LOCK_MODE_NONE;
    request->recursive_count = 1;
    request->status = OBJ_LOCK_REQUEST_STATUS_NEW;
    request->convert_mode = OBJ_LOCK_MODE_NONE;
    request->lock_head = NULL;
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

/* hash with resource type */
static inline obj_lock_resource_id_t obj_lock_resource_id_fullhash(obj_lock_resource_type_t type, obj_uint64_t hash) {
    return (type << (64 - OBJ_LOCK_RESOURCE_TYPE_BITS)) + (hash & ((obj_uint64_t)OBJ_UINT64_MAX >> OBJ_LOCK_RESOURCE_TYPE_BITS));
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
obj_lock_result_t obj_lock_convert(obj_lock_manager_t *lock_manager, obj_lock_resource_id_t resource_id, obj_lock_request_t *request, obj_lock_mode_t mode) {

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
        
        
    }
    pthread_mutex_unlock(&bucket->mutex);
}

obj_lock_head_t *obj_lock_find_or_insert(obj_lock_bucket_t *bucket, obj_lock_resource_id_t resource_id) {
    obj_lock_head_t *lock_head;

    return lock_head;
}

obj_lock_bucket_t *obj_lock_manager_get_bucket(obj_lock_manager_t *lock_manager, obj_lock_resource_id_t resource_id) {
    return &lock_manager->buckets[resource_id % OBJ_LOCK_BUCKET_NUM];
}

