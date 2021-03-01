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



#endif  /* OBJ_LOCKER_H */