#ifndef OBJ_CONCURRENCY_H
#define OBJ_CONCURRENCY_H

#include "obj_core.h"

typedef struct obj_global_lock_s obj_global_lock_t;
typedef struct obj_db_lock_s obj_db_lock_t;
typedef struct obj_collection_lock_s obj_collection_lock_t;

/* global lock */
struct obj_global_lock_s {

};

/* db lock */
struct obj_db_lock_s {
    obj_lock_resource_id_t rid;
    obj_lock_result_t result;
    obj_lock_locker_t *locker;
    obj_lock_mode_t mode;
    
};

/* collection lock */
struct obj_collection_lock_s {
    obj_lock_resource_id_t rid;
    obj_lock_result_t result;
    obj_lock_locker_t *locker;
};


#endif  /* OBJ_CONCURRENCY_H */