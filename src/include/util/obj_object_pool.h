#ifndef OBJ_OBJECT_POOL_H
#define OBJ_OBJECT_POOL_H

#include "obj_core.h"

typedef enum obj_object_pool_chunk_type obj_object_pool_chunk_type_t;
typedef struct obj_object_pool_chunk_header_s obj_object_pool_chunk_header_t;
typedef struct obj_object_pool_regular_free_chunk_s obj_object_pool_regular_free_chunk_t;
typedef struct obj_object_pool_dynamic_free_chunk_s obj_object_pool_dynamic_free_chunk_t;
typedef struct obj_object_pool_block_s obj_object_pool_block_t;
typedef struct obj_object_pool_block_group_s obj_object_pool_block_group_t;
typedef struct obj_object_pool_local_pool_s obj_object_pool_local_pool_t;
typedef struct obj_object_pool_s obj_object_pool_t;

#define OBJ_OBJECT_POOL_BLOCK_MAX_SIZE 64 * 1024
#define OBJ_OBJECT_POOL_BLOCK_MAX_ITEM 256
#define OBJ_OBJECT_POOL_FREE_CHUNK_MAX_ITEM 256

#define OBJ_OBJECT_POOL_MAX_BLOCK_NGROUP 65536
#define OBJ_OBJECT_POOL_GROUP_NBLOCK_NBIT 16
#define OBJ_OBJECT_POOL_GROUP_NBLOCK (1 << OBJ_OBJECT_POOL_GROUP_NBLOCK_NBIT)
#define OBJ_OBJECT_POOL_INITIAL_FREE_LIST_SIZE 1024

/* static __thread obj_object_pool_local_pool_t *_local_pool = NULL; */

/* static volatile obj_object_pool_t *_singleton = NULL; */

enum obj_object_pool_chunk_type {
    OBJ_OBJECT_POOL_CHUNK_REGULAR = 0,
    OBJ_OBJECT_POOL_CHUNK_DYNAMIC
};

struct obj_object_pool_chunk_header_s {
    obj_object_pool_chunk_type_t type;
    int nfree;
};

struct obj_object_pool_regular_free_chunk_s {
    obj_object_pool_chunk_header_t header;
    /* bu default */
    void *ptrs[OBJ_OBJECT_POOL_FREE_CHUNK_MAX_ITEM];
};

struct obj_object_pool_dynamic_free_chunk_s {
    obj_object_pool_chunk_header_t header;
    void *ptrs[];
};

struct obj_object_pool_block_s {
    int nitem;
    int size;
    char items[];
};

struct obj_object_pool_block_group_s {
    volatile int nblocks;
    volatile obj_object_pool_block_t *blocks[OBJ_OBJECT_POOL_GROUP_NBLOCK];
};

/* pool per thread, keep thread-local */
struct obj_object_pool_local_pool_s {
    /* reference to the global pool */
    obj_object_pool_t *pool;
    obj_object_pool_block_t *cur_block;
    int cur_block_index;
    obj_object_pool_regular_free_chunk_t cur_free;
};

struct obj_object_pool_s {
    int elem_size;
    /* num of free chunks */
    pthread_mutex_t free_chunks_mutex;      
    /* num of block group */                    
    pthread_mutex_t block_group_mutex;   
    pthread_mutex_t change_thread_mutex;                       
    volatile obj_object_pool_block_group_t *block_groups[OBJ_OBJECT_POOL_MAX_BLOCK_NGROUP];
    volatile int ngroup;
    /* global free chunk list */
    obj_array_t free_chunks;
    pthread_key_t key;
    volatile int nlocal;
};

void obj_global_object_pool_list_init();
void obj_global_object_pool_list_destroy();
obj_object_pool_t *obj_object_pool_create(int elem_size);
void *obj_object_pool_get_object(obj_object_pool_t *pool);
int obj_object_pool_return_object(obj_object_pool_t *pool, void *ptr);


#endif  /* OBJ_OBJECT_POOL_H */