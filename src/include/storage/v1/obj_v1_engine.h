#ifndef OBJ_V1_ENGINE_H
#define OBJ_V1_ENGINE_H

#include "obj_core.h"

typedef struct obj_v1_engine_s obj_v1_engine_t;

/* storage engine v1 */
struct obj_v1_engine_s {
    obj_engine_t base;
    /* database catalog entries */
    obj_prealloc_map_t map;
    /* 
     * protect map from concurrent update. 
     * e.x.: thread1 calls open_db_create_if_not_exists, thread2 also calls open_db_create_if_not_exists.
     * thread1 and thread2 both hold database X lock. this can't prevent thread1 and thread2 modify map concurrently
     */
    pthread_mutex_t mutex;
};

/* global v1 storage engine */
extern obj_v1_engine_t *g_v1_engine;

void obj_global_v1_engine_init();
void obj_global_v1_engine_destroy();

#endif  /* OBJ_V1_ENGINE_H */