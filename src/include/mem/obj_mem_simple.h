#ifndef OBJ_MEM_SIMPLE_H
#define OBJ_MEM_SIMPLE_H


#define OBJ_ALLOC_MINBITS 3
#define OBJ_MEM_SIMPLE_CONTEXT_FREELISTS 11
#define OBJ_MEM_CHUNK_LIMIT (1 << (OBJ_MEM_SIMPLE_CONTEXT_FREELISTS - 1 + OBJ_ALLOC_MINBITS))
#define OBJ_MIN_MEM_POOL_SIZE 1024
/* headers */
#define OBJ_MEM_POOL_HEADER_SIZE OBJ_MEM_ALIGN(sizeof(obj_mem_pool_t))
#define OBJ_MEM_CHUNK_HEADER_SIZE OBJ_MEM_ALIGN(sizeof(obj_mem_chunk_t))

#define OBJ_PTR_GET_MEM_CHUNK(ptr) ((obj_mem_chunk_t *)(((char *)(ptr)) - OBJ_MEM_CHUNK_HEADER_SIZE)) 
#define OBJ_MEM_CHUNK_GET_PTR(chunk) ((void *)(((char *)(chunk)) + OBJ_MEM_CHUNK_HEADER_SIZE))

#define OBJ_SINGLE_CHUNK_GET_POOL(chunk) ((obj_mem_pool_t *)(((char *)(chunk)) - OBJ_MEM_POOL_HEADER_SIZE))


/* concrete memory alloc context */
struct obj_mem_simple_context_s {
    obj_mem_context_t header;                                       /* base context */
    obj_mem_pool_t *pools;                                          /* memory pools in this alloc context */
    obj_mem_chunk_t *freelist[OBJ_MEM_SIMPLE_CONTEXT_FREELISTS];    /* free lists */
    obj_size_t init_pool_size;                                      /* initial pool size */
    obj_size_t max_pool_size;                                       /* max pool size */
    obj_size_t next_pool_size;                                      /* next pool size to allocate */
    obj_size_t alloc_chunk_limit;                                   /* chunk size limit */
    obj_bool_t is_reset;     
    pthread_mutex_t lock;                                     
};

/* memory pool */
struct obj_mem_pool_s {
    obj_mem_simple_context_t *context;  /* alloc context owns this pool */
    obj_mem_pool_t *prev;               /* prev allocated block */
    obj_mem_pool_t *next;               /* next allocated block */
    char *free_ptr;                     /* start of free space in this pool */
    char *end_ptr;                      /* end of free space in this pool */
};

/* memory chunk */
struct obj_mem_chunk_s {
    void *ptr;
    obj_size_t size;                    /* size of usable space in the chunk */
    
};


/* create a new allocate context */
obj_mem_context_t *obj_mem_simple_context_create(obj_size_t init_pool_size, obj_size_t max_pool_size);

/* init a allocate context */
void obj_mem_simple_context_init(obj_mem_context_t *context);

/* reset a allocate context */
void obj_mem_simple_context_reset(obj_mem_context_t *context);

/* delete a allocate context */
void obj_mem_simple_context_delete(obj_mem_context_t *context);

/* display statistic info of a allocate context */
void obj_mem_simple_context_stats(obj_mem_context_t *context);

/* allocate memory of given size */
void *obj_mem_simple_context_alloc(obj_mem_context_t *context, obj_size_t size);

/* free memory allocated */
void obj_mem_simple_context_free(obj_mem_context_t *context, void *ptr);

/* reallocate memory */
void *obj_mem_simple_context_realloc(obj_mem_context_t *context, void *ptr, obj_size_t size);


#endif  /* OBJ_MEM_SIMPLE_H */