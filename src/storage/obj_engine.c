#include "obj_core.h"

static obj_engine_t *g_engine;

/* call once */
void obj_global_v1_engine_init() {
    g_v1_engine = obj_v1_engine_create();
}

/* TODO implement different storage engines */
void obj_global_engine_init() {
    obj_assert(g_v1_engine != NULL);
    g_engine = (obj_engine_t *)g_v1_engine;
}

