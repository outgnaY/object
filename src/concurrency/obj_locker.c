#include "obj_core.h"


static obj_prealloc_map_methods_t obj_resource_id_request_map_methods = {
    obj_resource_id_request_map_hash_func,         
    obj_resource_id_request_map_key_compare,
    NULL,
    NULL,
    obj_resource_id_request_map_key_get,
    obj_resource_id_request_map_value_get,
    obj_resource_id_request_map_key_set,
    obj_resource_id_request_map_value_set,
    NULL,
    NULL
};

static obj_uint64_t obj_resource_id_request_map_hash_func(const void *key) {
    return obj_prealloc_map_hash_function(key, sizeof(obj_lock_resource_id_t));
}

/* compare resource id */
static int obj_resource_id_request_map_key_compare(const void *key1, const void *key2) {
    obj_lock_resource_id_t *id1 = (obj_lock_resource_id_t *)key1;
    obj_lock_resource_id_t *id2 = (obj_lock_resource_id_t *)key2;
    return (*id1) - (*id2);
}

static void *obj_resource_id_request_map_key_get(void *data) {
    obj_resource_id_request_pair_t *pair = (obj_resource_id_request_pair_t *)data;
    return &pair->resource_id;
}

static void *obj_resource_id_request_map_value_get(void *data) {
    obj_resource_id_request_pair_t *pair = (obj_resource_id_request_pair_t *)data;
    return &pair->request;
}

static void obj_resource_id_request_map_key_set(void *data, void *key) {
    obj_resource_id_request_pair_t *pair = (obj_resource_id_request_pair_t *)data;
    obj_memcpy(&pair->resource_id, key, sizeof(obj_lock_resource_id_t));
}

static void obj_resource_id_request_map_value_set(void *data, void *value) {
    obj_resource_id_request_pair_t *pair = (obj_resource_id_request_pair_t *)data;
    obj_memcpy(&pair->request, value, sizeof(obj_lock_request_t));
}


obj_lock_mode_t obj_locker_get_lock_mode(obj_locker_t *locker, obj_lock_resource_id_t resource_id) {
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&locker->request_map, &resource_id);
    obj_lock_request_t *request;
    if (entry != NULL) {
        request = (obj_lock_request_t *)obj_prealloc_map_get_value(&locker->request_map, entry);
        return request->mode;
    }
    return OBJ_LOCK_MODE_NONE;
}

/* X */
inline obj_bool_t obj_locker_isW(obj_locker_t *locker) {
    return obj_locker_get_lock_mode(locker, g_resource_id_global) == OBJ_LOCK_MODE_X;
}

/* S */
inline obj_bool_t obj_locker_isR(obj_locker_t *locker) {
    return obj_locker_get_lock_mode(locker, g_resource_id_global) == OBJ_LOCK_MODE_S;
}

/* check if the locker is locked */
inline obj_bool_t obj_locker_is_locked(obj_locker_t *locker) {
    return obj_locker_get_lock_mode(locker, g_resource_id_global) != OBJ_LOCK_MODE_NONE;
}

/* check if the given lock mode is covered by current lock mode */
inline obj_bool_t obj_locker_is_lock_held_for_mode(obj_locker_t *locker, obj_lock_resource_id_t resource_id, obj_lock_mode_t mode) {
    return obj_lock_is_mode_covered(mode, obj_locker_get_lock_mode(locker, resource_id));
}

/* IX/X */
inline obj_bool_t obj_locker_is_write_locked(obj_locker_t *locker) {
    return obj_locker_is_lock_held_for_mode(locker, g_resource_id_global, OBJ_LOCK_MODE_IX);
}

/* IS/S */
inline obj_bool_t obj_locker_is_read_locked(obj_locker_t *locker) {
    return obj_locker_is_lock_held_for_mode(locker, g_resource_id_global, OBJ_LOCK_MODE_IS);
}


obj_bool_t obj_locker_is_db_locked_for_mode(obj_locker_t *locker, char *db_name, int name_len, obj_lock_mode_t mode) {
    if (obj_locker_isW(locker)) {
        return true;
    }
    if (obj_locker_isR(locker) && obj_lock_is_shared_lock_mode(mode)) {
        return true;
    }
    obj_lock_resource_id_t resource_id = obj_lock_resource_id(OBJ_LOCK_RESOURCE_DATABASE, db_name, name_len);
    return obj_locker_is_lock_held_for_mode(locker, resource_id, mode);
}
/*
obj_bool_t obj_locker_is_collection_locked_for_mode(obj_locker_t *locker, char *collection_name, int name_len, obj_lock_mode_t mode) {
    if (obj_locker_isW(locker)) {
        return true;
    }
    if (obj_locker_isR(locker) && obj_lock_is_shared_lock_mode(mode)) {
        return true;
    }
    
    obj_lock_resource_id_t resource_id_db = obj_lock_resource_id(OBJ_LOCK_RESOURCE_DATABASE, );
}
*/

/* create a locker */
obj_locker_t *obj_locker_create() {
    obj_locker_t *locker = obj_alloc(sizeof(obj_locker_t));
    if (locker == NULL) {
        return NULL;
    }
    if (!obj_locker_init(locker)) {
        obj_free(locker);
    }
    return locker;
}

/* init a locker */
obj_bool_t obj_locker_init(obj_locker_t *locker) {
    obj_assert(locker);
    obj_lock_grant_notify_init(&locker->notify);
    if (!obj_prealloc_map_init(&locker->request_map, &obj_resource_id_request_map_methods, sizeof(obj_resource_id_request_pair_t))) {
        return false;
    }
    return true;
}

/* destroy a locker */
void obj_locker_destroy(obj_locker_t *locker) {
    obj_lock_grant_notify_destroy(&locker->notify);
    /* request map must be empty */
    obj_assert(obj_prealloc_map_is_empty(&locker->request_map));
    obj_prealloc_map_destroy(&locker->request_map);
}

/* do lock */
obj_lock_result_t obj_locker_lock(obj_locker_t *locker, obj_lock_resource_id_t resource_id, obj_lock_mode_t mode, struct timespec deadline, obj_bool_t check_deadlock) {
    obj_lock_result_t result = obj_locker_lock_begin(locker, resource_id, mode);
    if (result == OBJ_LOCK_RESULT_OK) {
        return result;
    }
    obj_assert(result == OBJ_LOCK_RESULT_WAITING);
    return obj_locker_lock_complete(locker, resource_id, mode, deadline, check_deadlock);
}

/* fast path to lock */
obj_lock_result_t obj_locker_lock_begin(obj_locker_t *locker, obj_lock_resource_id_t resource_id, obj_lock_mode_t mode) {
    obj_lock_request_t *request;
    obj_bool_t is_new = true;
    obj_prealloc_map_entry_t *entry = NULL;
    entry = obj_prealloc_map_find(&locker->request_map, &resource_id);
    if (entry == NULL) {
        entry = obj_prealloc_map_add_key(&locker->request_map, &resource_id);
        if (entry == NULL) {
            /* out of memory */
            return OBJ_LOCK_RESULT_INTERNAL_ERROR;
        } else {
            request = (obj_lock_request_t *)obj_prealloc_map_get_value(&locker->request_map, entry);
            obj_lock_request_init(request, locker, &locker->notify);
        }
        
    } else {
        /* already have the request */
        request = (obj_lock_request_t *)obj_prealloc_map_get_value(&locker->request_map, entry);
        is_new = false;
    }
    obj_lock_resource_type_t resource_type = obj_lock_resource_id_get_type(resource_id);
    if (resource_type == OBJ_LOCK_RESOURCE_GLOBAL) {
        if (mode == OBJ_LOCK_MODE_S || mode == OBJ_LOCK_MODE_X) {
            request->enqueue_front = true;
            request->compatible_first = true;
        }
    }
    obj_lock_grant_notify_clear(&locker->notify);
    obj_lock_result_t result = is_new ? obj_lock_lock(g_lock_manager, resource_id, request, mode) : obj_lock_convert(g_lock_manager, resource_id, request, mode);
    /*
    if (result == OBJ_LOCK_RESULT_WAITING) {

    }
    */
    return result;
}

obj_lock_result_t obj_locker_lock_complete(obj_locker_t *locker, obj_lock_resource_id_t resource_id, obj_lock_mode_t mode, struct timespec deadline, obj_bool_t check_deadlock) {
    obj_lock_result_t result;
    while (true) {
        result = obj_lock_grant_notify_timed_wait(&locker->notify, deadline);
        if (result == OBJ_LOCK_RESULT_OK) {
            break;
        }
        if (check_deadlock) {

        }

    }
    
    return result;
}

obj_bool_t obj_locker_unlock(obj_locker_t *locker, obj_lock_resource_id_t resource_id) {
    obj_prealloc_map_entry_t *entry = NULL;
    entry = obj_prealloc_map_find(&locker->request_map, &resource_id);
    if (entry == NULL) {
        return false;
    }
    return obj_locker_unlock_internal(locker, entry);
}

obj_bool_t obj_locker_unlock_internal(obj_locker_t *locker, obj_prealloc_map_entry_t *entry) {
    obj_lock_resource_id_t resource_id;
    obj_lock_request_t *request;
    obj_prealloc_map_t *map = &locker->request_map;
    resource_id = *((obj_lock_resource_id_t *)obj_prealloc_map_get_key(map, entry));
    request = (obj_lock_request_t *)obj_prealloc_map_get_value(map, entry);
    if (obj_lock_unlock(g_lock_manager, request)) {
        /* remove entry */
        obj_prealloc_map_delete_entry(map, entry);
        return true;
    }
    return false;
}