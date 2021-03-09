#include "obj_core.h"

obj_engine_t *g_engine;

/* init global storage engine, v1 by default */
void obj_global_engine_init() {
    obj_global_v1_engine_init();
    g_engine = (obj_engine_t *)g_v1_engine;
}

/* destroy global storage engine */
void obj_global_engine_destroy() {
    obj_global_v1_engine_destroy();
    g_engine = NULL;
}