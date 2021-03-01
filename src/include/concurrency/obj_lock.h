#ifndef OBJ_LOCK_H
#define OBJ_LOCK_H

#include "obj_core.h"

/* forward declaration */
typedef struct obj_locker_s obj_locker_t;

/* lock resource */
typedef enum obj_lock_resource_type obj_lock_resource_type_t;
typedef enum obj_lock_resource_id_singleton_hashid obj_lock_resource_id_singleton_hashid_t;
typedef obj_uint64_t obj_lock_resource_id_t;

/* lock implemention related */
typedef enum obj_lock_result obj_lock_result_t;
typedef enum obj_lock_mode obj_lock_mode_t;
typedef struct obj_resource_id_lock_head_pair_s obj_resource_id_lock_head_pair_t;
typedef struct obj_lock_manager_s obj_lock_manager_t;
typedef struct obj_lock_bucket_s obj_lock_bucket_t;
typedef enum obj_lock_request_status obj_lock_request_status_t;
typedef struct obj_lock_grant_notify_s obj_lock_grant_notify_t;
typedef struct obj_lock_request_s obj_lock_request_t;
typedef struct obj_lock_head_s obj_lock_head_t;
typedef struct obj_lock_locker_s obj_lock_locker_t;

/* resources to lock */
enum obj_lock_resource_type {
    OBJ_LOCK_RESOURCE_INVALID = 0,
    OBJ_LOCK_RESOURCE_GLOBAL,
    OBJ_LOCK_RESOURCE_DATABASE,
    OBJ_LOCK_RESOURCE_COLLECTION,
    OBJ_LOCK_RESOURCE_TYPE_COUNT
};

enum obj_lock_resource_id_singleton_hashid {
    OBJ_LOCK_RESOURCE_ID_SINGLETON_INVALID = 0,
    OBJ_LOCK_RESOURCE_ID_SINGLETON_GLOBAL
};

enum obj_lock_result {
    OBJ_LOCK_RESULT_OK,
    OBJ_LOCK_RESULT_WAITING,
    OBJ_LOCK_RESULT_TIMEOUT,
    OBJ_LOCK_RESULT_DEADLOCK,
    OBJ_LOCK_RESULT_INTERNAL_ERROR,
    OBJ_LOCK_RESULT_INVALID
};

/**
 * lock modes.
 * compatibility matrix
 *                      granted mode
 *  ------------------.---------------------------------------------------------------------------------------------
 *  request mode      | OBJ_LOCK_MODE_NONE | OBJ_LOCK_MODE_IS | OBJ_LOCK_MODE_IX | OBJ_LOCK_MODE_S | OBJ_LOCK_MODE_X 
 *  OBJ_LOCK_MODE_IS  |         +                    +                  +                  +                 -
 *  OBJ_LOCK_MODE_IX  |         +                    +                  +                  -                 -
 *  OBJ_LOCK_MODE_S   |         +                    +                  -                  +                 -
 *  OBJ_LOCK_MODE_X   |         +                    -                  -                  -                 -
 */
enum obj_lock_mode {
    OBJ_LOCK_MODE_NONE = 0,
    OBJ_LOCK_MODE_IS = 1,
    OBJ_LOCK_MODE_IX = 2,
    OBJ_LOCK_MODE_S = 3,
    OBJ_LOCK_MODE_X = 4,
    OBJ_LOCK_MODE_COUNT
};

struct obj_resource_id_lock_head_pair_s {
    obj_lock_resource_id_t resource_id;
    obj_lock_head_t *lock_head;
};

struct obj_lock_manager_s {
    int num_buckets;
    obj_lock_bucket_t *buckets;
};

struct obj_lock_bucket_s {
    pthread_mutex_t mutex;
    obj_prealloc_map_t map;
    /* obj_hashtable_t *table; */
};

enum obj_lock_request_status {
    OBJ_LOCK_REQUEST_STATUS_NEW,
    OBJ_LOCK_REQUEST_STATUS_GRANTED,
    OBJ_LOCK_REQUEST_STATUS_WAITING,
    OBJ_LOCK_REQUEST_STATUS_CONVERTING,
    OBJ_LOCK_REQUEST_STATUS_COUNT
};

struct obj_lock_grant_notify_s {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    obj_lock_result_t result;
};

struct obj_lock_request_s {
    obj_lock_mode_t mode;
    obj_lock_head_t *lock_head;
    obj_lock_request_status_t status;
    obj_bool_t enqueue_front;
    obj_bool_t compatible_first;
    int recursive_count;
    obj_lock_mode_t convert_mode;
    OBJ_EMBEDDED_LIST_NODE_T(obj_lock_request_t) list;
    obj_lock_grant_notify_t *notify;
    obj_locker_t *locker;
};

struct obj_lock_head_s {
    /* requests that have already been granted */
    OBJ_EMBEDDED_LIST_BASE_NODE_T(obj_lock_request_t) granted_list;
    int granted_count[OBJ_LOCK_MODE_COUNT];
    int granted_mode;
    /* requests that have not been granted due to conflict with granted requests */
    OBJ_EMBEDDED_LIST_BASE_NODE_T(obj_lock_request_t) conflict_list;
    int conflict_count[OBJ_LOCK_MODE_COUNT];
    int conflict_mode;
    /* conversion */
    int conversion_count;
    int compatible_first_count;
    /* resource */
    obj_lock_resource_id_t resource_id;
};

#define OBJ_LOCK_BUCKET_NUM 128
#define OBJ_LOCK_RESOURCE_TYPE_BITS 3

/* globals */
extern obj_lock_resource_id_t g_resource_id_global;

/* forward declaration */
obj_uint64_t obj_siphash(const obj_uint8_t *in, const obj_size_t inlen, const obj_uint8_t *k);

obj_bool_t obj_lock_is_shared_lock_mode(obj_lock_mode_t mode);
obj_bool_t obj_lock_is_mode_covered(obj_lock_mode_t mode, obj_lock_mode_t cover_mode);
void obj_global_lock_manager_init();
void obj_global_lock_manager_destroy();
obj_lock_result_t obj_lock_new_request(obj_lock_request_t *request, obj_lock_head_t *lock_head);
void obj_lock_request_init(obj_lock_request_t *request, obj_locker_t *locker, obj_lock_grant_notify_t *notify);
obj_lock_resource_id_t obj_lock_resource_id(obj_lock_resource_type_t type, char *str, int len);
obj_lock_resource_id_t obj_lock_resource_id_from_hashid(obj_lock_resource_type_t type, obj_uint64_t hash);
obj_lock_resource_type_t obj_lock_resource_id_get_type(obj_lock_resource_id_t resource_id);
obj_lock_result_t obj_lock_lock(obj_lock_manager_t *lock_manager, obj_lock_resource_id_t resource_id, obj_lock_request_t *request, obj_lock_mode_t mode);
obj_lock_result_t obj_lock_convert(obj_lock_manager_t *lock_manager, obj_lock_resource_id_t resource_id, obj_lock_request_t *request, obj_lock_mode_t new_mode);
void obj_lock_downgrade(obj_lock_manager_t *lock_manager, obj_lock_request_t *request, obj_lock_mode_t new_mode);
obj_bool_t obj_lock_unlock(obj_lock_manager_t *lock_manager, obj_lock_request_t *request);
obj_lock_head_t *obj_lock_find_or_insert(obj_lock_bucket_t *bucket, obj_lock_resource_id_t resource_id);
obj_lock_bucket_t *obj_lock_manager_get_bucket(obj_lock_manager_t *lock_manager, obj_lock_resource_id_t resource_id);
void obj_lock_manager_dump(obj_lock_manager_t *lock_manager);
void obj_lock_cleanup_unused_locks(obj_lock_manager_t *lock_manager);
void obj_lock_cleanup_unused_locks_in_bucket(obj_lock_bucket_t *bucket);
void obj_lock_on_lock_mode_changed(obj_lock_manager_t *lock_manager, obj_lock_head_t *lock_head, obj_bool_t check_conflict_queue);
void obj_lock_grant_notify_init(obj_lock_grant_notify_t *notify);
void obj_lock_grant_notify_clear(obj_lock_grant_notify_t *notify);
void obj_lock_grant_notify_destroy(obj_lock_grant_notify_t *notify);
obj_lock_result_t obj_lock_grant_notify_timed_wait(obj_lock_grant_notify_t *notify, obj_duration_msecond wait_time);
void obj_lock_grant_notify_notify(obj_lock_grant_notify_t *notify, obj_lock_result_t result);

#endif  /* OBJ_LOCK_H */