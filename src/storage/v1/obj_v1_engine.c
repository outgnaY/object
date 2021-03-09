#include "obj_core.h"

obj_v1_engine_t *g_v1_engine;

static obj_v1_engine_t *obj_v1_engine_create();
static void obj_v1_engine_destroy(obj_v1_engine_t *engine);
static obj_status_t obj_v1_engine_close_db(obj_engine_t *engine, obj_stringdata_t *db);
static obj_status_t obj_v1_engine_drop_db(obj_engine_t *engine, obj_stringdata_t *db);
static obj_db_catalog_entry_t *obj_v1_engine_get_db_catalog_entry(obj_engine_t *engine, obj_stringdata_t *db);
static obj_db_catalog_entry_t *obj_v1_engine_get_or_create_db_catalog_entry(obj_engine_t *engine, obj_stringdata_t *db, obj_bool_t *create);



static obj_engine_methods_t engine_methods = {
    obj_v1_engine_close_db,
    obj_v1_engine_drop_db,
    obj_v1_engine_get_db_catalog_entry,
    obj_v1_engine_get_or_create_db_catalog_entry
};


void obj_global_v1_engine_init() {
    g_v1_engine = obj_v1_engine_create();
    if (g_v1_engine == NULL) {
        fprintf(stderr, "can't init global v1 storage engine\n");
        exit(1);
    }
}

void obj_global_v1_engine_destroy() {
    obj_v1_engine_destroy(g_v1_engine);
    g_v1_engine = NULL;
}

/* create storage engine v1 */
static obj_v1_engine_t *obj_v1_engine_create() {
    obj_v1_engine_t *engine = obj_alloc(sizeof(obj_v1_engine_t));
    if (engine == NULL) {
        return NULL;
    }
    engine->base.methods = &engine_methods;
    if (!obj_prealloc_map_init(&engine->map, &db_catalog_entry_map_methods, sizeof(obj_db_catalog_pair_t))) {
        obj_free(engine);
        return NULL;
    }
    return engine;
}

/* destroy storage engine v1 */
static void obj_v1_engine_destroy(obj_v1_engine_t *engine) {
    obj_assert(engine);
    obj_prealloc_map_destroy(&engine->map);
    obj_free(engine);
}

/* close database */
static obj_status_t obj_v1_engine_close_db(obj_engine_t *engine, obj_stringdata_t *db) {
    obj_v1_engine_t *v1_engine = (obj_v1_engine_t *)engine;

}

/* drop database */
static obj_status_t obj_v1_engine_drop_db(obj_engine_t *engine, obj_stringdata_t *db) {
    obj_v1_engine_t *v1_engine = (obj_v1_engine_t *)engine;
    /* delete from map */
    obj_prealloc_map_delete(&v1_engine->map, db, false);
    return obj_status_create("", OBJ_CODE_OK);
}

/* get database catalog entry */
static obj_db_catalog_entry_t *obj_v1_engine_get_db_catalog_entry(obj_engine_t *engine, obj_stringdata_t *db) {
    obj_v1_engine_t *v1_engine = (obj_v1_engine_t *)engine;
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&v1_engine->map, db);
    if (entry == NULL) {
        return NULL;
    }
    obj_v1_db_catalog_entry_t *v1_db_catalog_entry = *(obj_v1_db_catalog_entry_t **)obj_prealloc_map_get_value(&v1_engine->map, entry);
    return (obj_db_catalog_entry_t *)v1_db_catalog_entry;
}

/* get or create database catalog entry */
static obj_db_catalog_entry_t *obj_v1_engine_get_or_create_db_catalog_entry(obj_engine_t *engine, obj_stringdata_t *db, obj_bool_t *create) {
    obj_v1_engine_t *v1_engine = (obj_v1_engine_t *)engine;
    obj_prealloc_map_entry_t *entry = obj_prealloc_map_find(&v1_engine->map, db);
    obj_v1_db_catalog_entry_t *v1_db_catalog_entry = NULL;
    if (entry != NULL) {
        v1_db_catalog_entry = *(obj_v1_db_catalog_entry_t **)obj_prealloc_map_get_value(&v1_engine->map, entry);
        return (obj_db_catalog_entry_t *)v1_db_catalog_entry;
    }
    v1_db_catalog_entry = obj_v1_db_catalog_entry_create();
    if (v1_db_catalog_entry == NULL) {
        return NULL;
    }
    obj_stringdata_t db_copy = obj_stringdata_copy_stringdata(db);
    if (db_copy.data == NULL) {
        obj_v1_db_catalog_entry_destroy(v1_db_catalog_entry);
        return NULL;
    }
    if (obj_prealloc_map_add(&v1_engine->map, &db_copy, &v1_db_catalog_entry) != OBJ_PREALLOC_MAP_CODE_OK) {
        obj_stringdata_destroy(&db_copy);
        obj_v1_db_catalog_entry_destroy(v1_db_catalog_entry);
        return NULL;
    }
    return (obj_db_catalog_entry_t *)v1_db_catalog_entry;
}
