#ifndef OBJ_MEM_H
#define OBJ_MEM_H

/* typedefs */
typedef struct obj_mem_context_methods_s obj_mem_context_methods_t;
typedef struct obj_mem_context_s obj_mem_context_t;
typedef struct obj_mem_simple_context_s obj_mem_simple_context_t;
typedef struct obj_mem_pool_s obj_mem_pool_t;
typedef struct obj_mem_chunk_s obj_mem_chunk_t;

extern obj_mem_context_t *g_context;

#define OBJ_MEM_ALIGN(SIZE) OBJ_ALIGN(SIZE, OBJ_ALIGNMENT)

/* wrapper of glibc malloc and calloc */
void *obj_lib_alloc(obj_size_t size);
void *obj_lib_calloc(obj_size_t size);
#define obj_lib_free free

/* wrapper of alloc methods */
/*
#define obj_alloc(size) obj_mem_context_alloc(g_context, size)
#define obj_free(ptr) obj_mem_context_free(g_context, ptr)
#define obj_realloc(ptr, size) obj_mem_context_realloc(g_context, ptr, size)
*/

#define obj_alloc(size) malloc(size)
#define obj_free(ptr) free(ptr)
#define obj_realloc(ptr, size) realloc(ptr, size)




struct obj_mem_context_methods_s {
    void (*init) (obj_mem_context_t *context);
    void (*reset) (obj_mem_context_t *context);
    void (*delete) (obj_mem_context_t *context);
    void (*stats)(obj_mem_context_t *context); 
    void *(*alloc) (obj_mem_context_t *context, obj_size_t size);
    void (*free) (obj_mem_context_t *context, void *ptr);
    void *(*realloc) (obj_mem_context_t *context, void *ptr, obj_size_t size);
};

/* memory context */
struct obj_mem_context_s {
    obj_mem_context_methods_t *methods;
};

/* init global mem context */
void obj_global_mem_context_init();

/* reset global mem context */
void obj_global_mem_context_reset();

/* delete global mem context */
void obj_global_mem_context_delete();

/* display statistic info of global mem context */
void obj_global_mem_context_stats();

/* create context */
obj_mem_context_t *obj_mem_context_create(obj_size_t size, obj_mem_context_methods_t *methods);

/* init */
void obj_mem_context_init(obj_mem_context_t *context);

/* reset */
void obj_mem_context_reset(obj_mem_context_t *context);

/* delete */
void obj_mem_context_delete(obj_mem_context_t *context);

/* stats */
void obj_mem_context_stats(obj_mem_context_t *context);

/* alloc */
void *obj_mem_context_alloc(obj_mem_context_t *context, obj_size_t size);

/* free */
void obj_mem_context_free(obj_mem_context_t *context, void *ptr);

/* realloc */
void *obj_mem_context_realloc(obj_mem_context_t *context, void *ptr, obj_size_t size);



#endif  /* OBJ_MEM_H */