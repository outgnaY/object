#ifndef OBJ_LOCKER_H
#define OBJ_LOCKER_H

#include "obj_core.h"

typedef struct obj_locker_s obj_locker_t;
typedef struct obj_resource_id_request_pair_s obj_resource_id_request_pair_t;

struct obj_locker_s {
    obj_lock_grant_notify_t notify;
    /* lock request map */
    obj_prealloc_map_t request_map;
    obj_bool_t max_lock_timeout_is_set;
    struct timespec max_lock_timeout;
    obj_duration_msecond deadlock_timeout;
};

struct obj_resource_id_request_pair_s {
    obj_lock_resource_id_t resource_id;
    obj_lock_request_t request;
};

/* forward declarations */
extern obj_lock_manager_t *g_lock_manager;
extern obj_lock_resource_id_t g_resource_id_global;

obj_lock_mode_t obj_locker_get_lock_mode(obj_locker_t *locker, obj_lock_resource_id_t resource_id);
obj_bool_t obj_locker_isW(obj_locker_t *locker);
obj_bool_t obj_locker_isR(obj_locker_t *locker);
obj_bool_t obj_locker_is_locked(obj_locker_t *locker);
obj_bool_t obj_locker_is_lock_held_for_mode(obj_locker_t *locker, obj_lock_resource_id_t resource_id, obj_lock_mode_t mode);
obj_bool_t obj_locker_is_write_locked(obj_locker_t *locker);
obj_bool_t obj_locker_is_read_locked(obj_locker_t *locker);
obj_bool_t obj_locker_is_db_locked_for_mode(obj_locker_t *locker, char *db_name, int name_len, obj_lock_mode_t mode);
obj_locker_t *obj_locker_create();
obj_bool_t obj_locker_init(obj_locker_t *locker);
void obj_locker_destroy_static(obj_locker_t *locker);
void obj_locker_destroy(obj_locker_t *locker);
obj_lock_result_t obj_locker_lock(obj_locker_t *locker, obj_lock_resource_id_t resource_id, obj_lock_mode_t mode, obj_abs_time_msecond deadline, obj_bool_t check_deadlock);
obj_lock_result_t obj_locker_lock_begin(obj_locker_t *locker, obj_lock_resource_id_t resource_id, obj_lock_mode_t mode);
obj_lock_result_t obj_locker_lock_complete(obj_locker_t *locker, obj_lock_resource_id_t resource_id, obj_lock_mode_t mode, obj_abs_time_msecond deadline, obj_bool_t check_deadlock);
obj_bool_t obj_locker_unlock(obj_locker_t *locker, obj_lock_resource_id_t resource_id);
void obj_locker_clean_requests(obj_locker_t *locker);


#endif  /* OBJ_LOCKER_H */