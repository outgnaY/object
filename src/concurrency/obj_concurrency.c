#include "obj_core.h"

void obj_global_lock_init(obj_global_lock_t *global_lock, obj_locker_t *locker, obj_lock_mode_t mode) {
    obj_assert(global_lock);
    obj_assert(locker);
    global_lock->locker = locker;
    global_lock->mode = mode;
    global_lock->result = OBJ_LOCK_RESULT_INVALID;
}

void obj_global_lock_init_with_deadline(obj_global_lock_t *global_lock, obj_locker_t *locker, obj_lock_mode_t mode, obj_abs_time_msecond deadline) {
    obj_global_lock_init(global_lock, locker, mode);
    global_lock->deadline = deadline;
}

obj_global_lock_t *obj_global_lock_create(obj_locker_t *locker, obj_lock_mode_t mode) {
    obj_global_lock_t *global_lock = obj_alloc(sizeof(obj_global_lock_t));
    obj_global_lock_init(global_lock, locker, mode);
    return global_lock;
}

obj_global_lock_t *obj_global_lock_create_with_deadline(obj_locker_t *locker, obj_lock_mode_t mode, obj_abs_time_msecond deadline) {
    obj_global_lock_t *global_lock = obj_global_lock_create(locker, mode);
    global_lock->deadline = deadline;
    return global_lock;
}

obj_lock_result_t obj_global_lock_lock(obj_global_lock_t *global_lock) {
    return obj_locker_lock(global_lock->locker, g_resource_id_global, global_lock->mode, global_lock->deadline, false);
}

obj_bool_t obj_global_lock_unlock(obj_global_lock_t *global_lock) {
    return obj_locker_unlock(global_lock->locker, g_resource_id_global);
}

void obj_db_lock_init(obj_db_lock_t *db_lock, obj_locker_t *locker, obj_lock_mode_t mode, char *db) {
    obj_db_lock_init_with_len(db_lock, locker, mode, db, obj_strlen(db));
}

void obj_db_lock_init_with_len(obj_db_lock_t *db_lock, obj_locker_t *locker, obj_lock_mode_t mode, char *db, int len) {
    obj_assert(db_lock);
    obj_assert(locker);
    db_lock->locker = locker;
    db_lock->mode = mode;
    db_lock->result = OBJ_LOCK_RESULT_INVALID;
    db_lock->rid = obj_lock_resource_id(OBJ_LOCK_RESOURCE_DATABASE, db, len);
}

void obj_db_lock_init_with_deadline(obj_db_lock_t *db_lock, obj_locker_t *locker, obj_lock_mode_t mode, char *db, obj_abs_time_msecond deadline) {
    obj_db_lock_init(db_lock, locker, mode, db);
    db_lock->deadline = deadline;
}

obj_db_lock_t *obj_db_lock_create(obj_locker_t *locker, obj_lock_mode_t mode, char *db) {
    obj_db_lock_t *db_lock = obj_alloc(sizeof(obj_db_lock_t));
    obj_db_lock_init(db_lock, locker, mode, db);
    return db_lock;
}

obj_db_lock_t *obj_db_lock_create_with_deadline(obj_locker_t *locker, obj_lock_mode_t mode, char *db, obj_abs_time_msecond deadline) {
    obj_db_lock_t *db_lock = obj_db_lock_create(locker, mode, db);
    db_lock->deadline = deadline;
    return db_lock;
}

obj_lock_result_t obj_db_lock_lock(obj_db_lock_t *db_lock) {
    return obj_locker_lock(db_lock->locker, db_lock->rid, db_lock->mode, db_lock->deadline, false);
}

obj_bool_t obj_db_lock_unlock(obj_db_lock_t *db_lock) {
    return obj_locker_unlock(db_lock->locker, db_lock->rid);
}

void obj_collection_lock_init(obj_collection_lock_t *collection_lock, obj_locker_t *locker, obj_lock_mode_t mode, char *collection) {
    obj_collection_lock_init_with_len(collection_lock, locker, mode, collection, obj_strlen(collection));
}

void obj_collection_lock_init_with_len(obj_collection_lock_t *collection_lock, obj_locker_t *locker, obj_lock_mode_t mode, char *collection, int len) {
    obj_assert(collection_lock);
    obj_assert(locker);
    collection_lock->locker = locker;
    collection_lock->mode = mode;
    collection_lock->result = OBJ_LOCK_RESULT_INVALID;
    collection_lock->rid = obj_lock_resource_id(OBJ_LOCK_RESOURCE_COLLECTION, collection, len);
}

void obj_collection_lock_init_with_deadline(obj_collection_lock_t *collection_lock, obj_locker_t *locker, obj_lock_mode_t mode, char *collection, obj_abs_time_msecond deadline) {
    obj_collection_lock_init(collection_lock, locker, mode, collection);
    collection_lock->deadline = deadline;
}

obj_collection_lock_t *obj_collection_lock_create(obj_locker_t *locker, obj_lock_mode_t mode, char *collection) {
    obj_collection_lock_t *collection_lock = obj_alloc(sizeof(obj_collection_lock_t));
    obj_collection_lock_init(collection_lock, locker, mode, collection);
    return collection_lock;
}

obj_collection_lock_t *obj_collection_lock_with_deadline(obj_locker_t *locker, obj_lock_mode_t mode, char *collection, obj_abs_time_msecond deadline) {
    obj_collection_lock_t *collection_lock = obj_collection_lock_create(locker, mode, collection);
    collection_lock->deadline = deadline;
    return collection_lock;
}

obj_lock_result_t obj_collection_lock_lock(obj_collection_lock_t *collection_lock) {
    return obj_locker_lock(collection_lock->locker, collection_lock->rid, collection_lock->mode, collection_lock->deadline, false);
}

obj_bool_t obj_collection_lock_unlock(obj_collection_lock_t *collection_lock) {
    return obj_locker_unlock(collection_lock->locker, collection_lock->rid);
}