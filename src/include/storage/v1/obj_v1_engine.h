#ifndef OBJ_V1_ENGINE_H
#define OBJ_V1_ENGINE_H

#include "obj_core.h"

typedef struct obj_v1_engine_s obj_v1_engine_t;

/* storage engine v1 */
struct obj_v1_engine_s {
    obj_engine_t base;

};

obj_v1_engine_t *obj_v1_engine_create();

#endif  /* OBJ_V1_ENGINE_H */