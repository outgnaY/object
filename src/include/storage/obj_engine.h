#ifndef OBJ_ENGINE_H
#define OBJ_ENGINE_H

/* forward declaration */
typedef struct obj_v1_engine_s obj_v1_engine_t;

typedef struct obj_engine_methods_s obj_engine_methods_t;
typedef struct obj_engine_s obj_engine_t;

struct obj_engine_methods_s {
    void (list_databases)(obj_engine_t *engine);
};

struct obj_engine_s {
    obj_engine_methods_t *methods;
};

extern obj_v1_engine_t *g_v1_engine;

#endif  /* OBJ_ENGINE_H */