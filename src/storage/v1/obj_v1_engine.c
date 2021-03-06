#include "obj_core.h"

obj_v1_engine_t *g_v1_engine;



void obj_global_v1_engine_init() {
    g_v1_engine = obj_v1_engine_create();
    if (g_v1_engine == NULL) {
        fprintf(stderr, "can't init global v1 storage engine\n");
        exit(1);
    }
}

void obj_global_v1_engine_destroy() {
    obj_v1_engine_destroy(g_v1_engine);
}

/* create storage engine v1 */
static obj_v1_engine_t *obj_v1_engine_create() {
    obj_v1_engine_t *engine = obj_alloc(sizeof(obj_v1_engine_t));
    if (engine == NULL) {
        return NULL;
    }
    return engine;
}

/* destroy storage engine v1 */
static void obj_v1_engine_destroy(obj_v1_engine_t *engine) {
    obj_assert(engine);
    obj_prealloc_map_destroy(&engine->map);
    /* pthread_mutex_init(&engine->map_mutex, NULL); */
}





