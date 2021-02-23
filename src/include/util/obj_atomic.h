#ifndef OBJ_ATOMIC_H
#define OBJ_ATOMIC_H

#include <pthread.h>
#include "obj_config.h"
/* atomic operations */

/* #if defined(OBJ_HAVE_ATOMIC) */
/* implementation using gcc builtin functions */

#define obj_atomic_incr(var, count) __sync_add_and_fetch(&var, (count))
#define obj_atomic_get_incr(var, oldvalue_var, count) do { \
    oldvalue_var = __sync_fetch_and_add(&var, (count)); \
} while(0)
#define obj_atomic_decr(var, count) __sync_sub_and_fetch(&var, (count))
#define obj_atomic_get(var, dstvar) do { \
    dstvar = __sync_sub_and_fetch(&var, 0); \
} while(0)
#define obj_atomic_set(var, value) do { \
    while(!__sync_bool_compare_and_swap(&var, var, value)); \
} while(0)

/* #else */
/* implementation using pthread mutex */
/*
#define obj_atomic_incr(var, count) do { \
    pthread_mutex_lock(&var ## _mutex); \
    var += (count); \
    pthread_mutex_unlock(&var ## _mutex); \
} while(0)

#define obj_atomic_get_incr(var, oldvalue_var, count) do { \
    pthread_mutex_lock(&var ## _mutex); \
    oldvalue_var = var; \
    var += (count); \
    pthread_mutex_unlock(&var ## _mutex); \
} while(0)

#define obj_atomic_decr(var, count) do { \
    pthread_mutex_lock(&var ## _mutex); \
    var -= (count); \
    pthread_mutex_unlock(&var ## _mutex); \
} while(0)

#define obj_atomic_get(var, dstvar) do { \
    pthread_mutex_lock(&var ## _mutex); \
    dstvar = var; \
    pthread_mutex_unlock(&var ## _mutex); \
} while(0)

#define obj_atomic_set(var, value) do { \
    pthread_mutex_lock(&var ## _mutex); \
    var = value; \
    pthread_mutex_unlock(&var ## _mutex); \
 } while(0)

#endif
*/
#endif  /* OBJ_ATOMIC_H */