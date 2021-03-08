#ifndef OBJ_ENGINE_H
#define OBJ_ENGINE_H

#include "obj_core.h"

typedef struct obj_engine_s obj_engine_t;
typedef struct obj_engine_methods_s obj_engine_methods_t;

struct obj_engine_s {
    obj_engine_methods_t *methods;
};

struct obj_engine_methods_s {
    obj_status_t (*drop_database)(obj_stringdata_t *db);
    obj_db_catalog_entry_t *(*get_db_catalog_entry)(obj_stringdata_t *db);
    obj_db_catalog_entry_t *(*get_or_create_db_catalog_entry)(obj_stringdata_t *db, obj_bool_t *create);
};

/* global storage engine */
extern obj_engine_t *g_engine;

#endif  /* OBJ_ENGINE_H */