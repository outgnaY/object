#ifndef OBJ_ENGINE_H
#define OBJ_ENGINE_H

#include "obj_core.h"

typedef struct obj_engine_s obj_engine_t;
typedef struct obj_engine_methods_s obj_engine_methods_t;

struct obj_engine_s {
    obj_engine_methods_t *methods;
};

struct obj_engine_methods_s {
    obj_status_t (*close_db)(obj_engine_t *engine, obj_stringdata_t *db);
    obj_status_t (*drop_db)(obj_engine_t *engine, obj_stringdata_t *db);
    obj_db_catalog_entry_t *(*get_db_catalog_entry)(obj_engine_t *engine, obj_stringdata_t *db);
    obj_db_catalog_entry_t *(*get_or_create_db_catalog_entry)(obj_engine_t *engine, obj_stringdata_t *db, obj_bool_t *create);
};

/* global storage engine */
extern obj_engine_t *g_engine;

void obj_global_engine_init();
void obj_global_engine_destroy();

#endif  /* OBJ_ENGINE_H */