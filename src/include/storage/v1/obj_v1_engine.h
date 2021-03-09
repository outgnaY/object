#ifndef OBJ_V1_ENGINE_H
#define OBJ_V1_ENGINE_H

#include "obj_core.h"

typedef struct obj_db_catalog_pair_s obj_db_catalog_pair_t;
typedef struct obj_v1_engine_s obj_v1_engine_t;

struct obj_db_catalog_pair_s {
    obj_stringdata_t db;
    obj_v1_db_catalog_entry_t *entry;
};

/* storage engine v1 */
struct obj_v1_engine_s {
    obj_engine_t base;
    /* database catalog entries */
    obj_prealloc_map_t map;
    /* pthread_mutex_t map_mutex; */
};

obj_v1_engine_t *obj_v1_engine_create();

/* global v1 storage engine */
extern obj_v1_engine_t *g_v1_engine;

void obj_global_v1_engine_init();
void obj_global_v1_engine_destroy();

#endif  /* OBJ_V1_ENGINE_H */