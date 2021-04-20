#include "obj_core.h"

obj_engine_t *g_engine;

static obj_engine_t *obj_engine_create();
static void obj_engine_init(obj_engine_t *engine);
static void obj_engine_destroy(obj_engine_t *engine);

/* init global storage engine */
void obj_global_engine_init() {
    g_engine = obj_engine_create();
    if (g_engine == NULL) {
        fprintf(stderr, "failed to create global storage engine");
        exit(1);
    }
}

/* destroy global storage engine */
void obj_global_engine_destroy() {
    obj_engine_destroy(g_engine);
    g_engine = NULL;
}

static obj_engine_t *obj_engine_create() {
    obj_engine_t *engine = obj_alloc(sizeof(obj_engine_t));
    obj_engine_init(engine);
    return engine;
}

static void obj_engine_init(obj_engine_t *engine) {
    /* init database catalog entry map */
    obj_prealloc_map_init(&engine->map, &db_catalog_entry_map_methods, sizeof(obj_db_catalog_entry_pair_t));

}

static void obj_engine_destroy(obj_engine_t *engine) {
    obj_prealloc_map_destroy_static(&engine->map);
    obj_free(engine);
}

