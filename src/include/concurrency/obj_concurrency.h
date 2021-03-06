#ifndef OBJ_CONCURRENCY_H
#define OBJ_CONCURRENCY_H

#include "obj_core.h"

typedef struct obj_global_lock_s obj_global_lock_t;
typedef struct obj_db_lock_s obj_db_lock_t;
typedef struct obj_collection_lock_s obj_collection_lock_t;

/* global lock */
struct obj_global_lock_s {
    obj_lock_result_t result;
    obj_locker_t *locker;
    obj_lock_mode_t mode;
    obj_abs_time_msecond deadline;
};

/* db lock */
struct obj_db_lock_s {
    obj_lock_resource_id_t rid;
    obj_lock_result_t result;
    obj_locker_t *locker;
    obj_lock_mode_t mode;
    obj_abs_time_msecond deadline;
};

/* collection lock */
struct obj_collection_lock_s {
    obj_lock_resource_id_t rid;
    obj_lock_result_t result;
    obj_locker_t *locker;
    obj_lock_mode_t mode;
    obj_abs_time_msecond deadline;
};

void obj_global_lock_init(obj_global_lock_t *global_lock, obj_locker_t *locker, obj_lock_mode_t mode);
void obj_global_lock_init_with_deadline(obj_global_lock_t *global_lock, obj_locker_t *locker, obj_lock_mode_t mode, obj_abs_time_msecond deadline);
obj_global_lock_t *obj_global_lock_create(obj_locker_t *locker, obj_lock_mode_t mode);
obj_global_lock_t *obj_global_lock_create_with_deadline(obj_locker_t *locker, obj_lock_mode_t mode, obj_abs_time_msecond deadline);
obj_lock_result_t obj_global_lock_lock(obj_global_lock_t *global_lock);
obj_bool_t obj_global_lock_unlock(obj_global_lock_t *global_lock);
void obj_db_lock_init(obj_db_lock_t *db_lock, obj_locker_t *locker, obj_lock_mode_t mode, obj_stringdata_t *db);
void obj_db_lock_init_with_deadline(obj_db_lock_t *db_lock, obj_locker_t *locker, obj_lock_mode_t mode, obj_stringdata_t *db, obj_abs_time_msecond deadline);
obj_db_lock_t *obj_db_lock_create(obj_locker_t *locker, obj_lock_mode_t mode, obj_stringdata_t *db);
obj_db_lock_t *obj_db_lock_create_with_deadline(obj_locker_t *locker, obj_lock_mode_t mode, obj_stringdata_t *db, obj_abs_time_msecond deadline);
obj_lock_result_t obj_db_lock_lock(obj_db_lock_t *db_lock);
obj_bool_t obj_db_lock_unlock(obj_db_lock_t *db_lock);
void obj_collection_lock_init(obj_collection_lock_t *collection_lock, obj_locker_t *locker, obj_lock_mode_t mode, obj_stringdata_t *collection);
void obj_collection_lock_init_with_deadline(obj_collection_lock_t *collection_lock, obj_locker_t *locker, obj_lock_mode_t mode, obj_stringdata_t *collection, obj_abs_time_msecond deadline);
obj_collection_lock_t *obj_collection_lock_create(obj_locker_t *locker, obj_lock_mode_t mode, obj_stringdata_t *collection);
obj_collection_lock_t *obj_collection_lock_with_deadline(obj_locker_t *locker, obj_lock_mode_t mode, obj_stringdata_t *collection, obj_abs_time_msecond deadline);
obj_lock_result_t obj_collection_lock_lock(obj_collection_lock_t *collection_lock);
obj_bool_t obj_collection_lock_unlock(obj_collection_lock_t *collection_lock);

#endif  /* OBJ_CONCURRENCY_H */