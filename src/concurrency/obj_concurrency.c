#include "obj_core.h"

void obj_global_lock_init(obj_global_lock_t *global_lock, obj_locker_t *locker, obj_lock_mode_t mode) {
    obj_assert(global_lock);
    obj_assert(locker);
    global_lock->locker = locker;
    global_lock->mode = mode;
    global_lock->result = OBJ_LOCK_RESULT_INVALID;
}

obj_global_lock_t *obj_global_lock_create(obj_locker_t *locker, obj_lock_mode_t mode) {
    obj_global_lock_t *global_lock = obj_alloc(sizeof(obj_global_lock_t));
    if (global_lock == NULL) {
        return NULL;
    }
    obj_global_lock_init(global_lock, locker, mode);
    return global_lock;
}

obj_lock_result_t obj_global_lock_lock(obj_global_lock_t *global_lock) {
    return obj_locker_lock(global_lock->locker, g_resource_id_global, global_lock->mode, -1, false);
}

obj_bool_t obj_global_lock_unlock(obj_global_lock_t *global_lock) {
    return obj_locker_unlock(global_lock->locker, g_resource_id_global);
}

void obj_db_lock_init(obj_db_lock_t *db_lock, obj_locker_t *locker, obj_lock_mode_t mode, const char *db_name, int name_len) {
    obj_assert(db_lock);
    obj_assert(locker);
    db_lock->locker = locker;
    db_lock->mode = mode;
    db_lock->result = OBJ_LOCK_RESULT_INVALID;
    db_lock->rid = obj_lock_resource_id(OBJ_LOCK_RESOURCE_DATABASE, db_name, name_len);
}

obj_db_lock_t *obj_db_lock_create(obj_locker_t *locker, obj_lock_mode_t mode, const char *db_name, int name_len) {
    obj_db_lock_t *db_lock = obj_alloc(sizeof(obj_db_lock_t));
    if (db_lock == NULL) {
        return NULL;
    }
    obj_db_lock_init(db_lock, locker, mode, db_name, name_len);
    return db_lock;
}

obj_lock_result_t obj_db_lock_lock(obj_db_lock_t *db_lock) {
    return obj_locker_lock(db_lock->locker, db_lock->rid, db_lock->mode, -1, false);
}

obj_bool_t obj_db_lock_unlock(obj_db_lock_t *db_lock) {
    return obj_locker_unlock(db_lock->locker, db_lock->rid);
}

void obj_collection_lock_init(obj_collection_lock_t *collection_lock, obj_locker_t *locker, obj_lock_mode_t mode, const char *collection_name, int name_len) {
    obj_assert(collection_lock);
    obj_assert(locker);
    collection_lock->locker = locker;
    collection_lock->mode = mode;
    collection_lock->result = OBJ_LOCK_RESULT_INVALID;
    collection_lock->rid = obj_lock_resource_id(OBJ_LOCK_RESOURCE_COLLECTION, collection_name, name_len);
}

obj_collection_lock_t *obj_collection_lock_create(obj_locker_t *locker, obj_lock_mode_t mode, const char *collection_name, int name_len) {
    obj_collection_lock_t *collection_lock = obj_alloc(sizeof(obj_collection_lock_t));
    if (collection_lock == NULL) {
        return NULL;
    }
    obj_collection_lock_init(collection_lock, locker, mode, collection_name, name_len);
    return collection_lock;
}

obj_lock_result_t obj_collection_lock_lock(obj_collection_lock_t *collection_lock) {
    return obj_locker_lock(collection_lock->locker, collection_lock->rid, collection_lock->mode, -1, false);
}

obj_bool_t obj_collection_lock_unlock(obj_collection_lock_t *collection_lock) {
    return obj_locker_unlock(collection_lock->locker, collection_lock->rid);
}