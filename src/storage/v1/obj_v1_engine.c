#include "obj_core.h"

obj_v1_engine_t *g_v1_engine;


/* create storage engine v1 */
obj_v1_engine_t *obj_v1_engine_create() {
    obj_v1_engine_t *engine = obj_alloc(sizeof(obj_v1_engine_t));
    
    return engine;
}

/* list databases */
static void obj_v1_engine_list_databases(obj_engine_t *engine) {
    obj_v1_engine_t *v1_engine = (obj_v1_engine_t *)engine;

}


