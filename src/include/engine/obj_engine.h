#ifndef OBJ_ENGINE_H
#define OBJ_ENGINE_H

#include "obj_core.h"

typedef struct obj_engine_s obj_engine_t;


/* storage engine */
struct obj_engine_s {
    obj_prealloc_map_t map;
    /*
    pthread_mutex_t mutex;
    */
};



#endif  /* OBJ_ENGINE_H */