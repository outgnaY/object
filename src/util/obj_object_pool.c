#include "obj_core.h"

/* object pool list */
obj_list_t *g_object_pool_list;

static void obj_global_object_pool_list_free(void *ptr);
static obj_bool_t obj_object_pool_register(obj_object_pool_t *pool);
static void obj_object_pool_destroy(obj_object_pool_t *pool);
static obj_object_pool_block_t *obj_object_pool_add_block(obj_object_pool_t *pool, int *index);
static obj_object_pool_block_t *obj_object_pool_create_block(int elem_size);
static obj_bool_t obj_object_pool_add_block_group(obj_object_pool_t *pool, int old_ngroup);
static obj_object_pool_block_group_t *obj_object_pool_create_block_group();
static obj_bool_t obj_object_pool_pop_free_chunk(obj_object_pool_t *pool, obj_object_pool_regular_free_chunk_t *chunk);
static obj_bool_t obj_object_pool_push_free_chunk(obj_object_pool_t *pool, obj_object_pool_regular_free_chunk_t chunk);
static obj_object_pool_local_pool_t *obj_object_pool_create_local_pool(obj_object_pool_t *pool);
static void obj_object_pool_destroy_local_pool(void *arg);
static obj_object_pool_local_pool_t *obj_object_pool_get_or_new_local_pool(obj_object_pool_t *pool);
static void *obj_object_pool_get_object_local(obj_object_pool_local_pool_t *local_pool);
static int obj_object_pool_return_object_local(obj_object_pool_local_pool_t *local_pool, void *ptr);


/* module init */
void obj_global_object_pool_list_init() {
    g_object_pool_list = obj_list_create();
    obj_list_set_free(g_object_pool_list, obj_global_object_pool_list_free);
    if (g_object_pool_list == NULL) {
        fprintf(stderr, "failed to create global object pool list\n");
        exit(1);
    }
}

static void obj_global_object_pool_list_free(void *ptr) {
    obj_object_pool_t *pool = (obj_object_pool_t *)ptr;
    obj_object_pool_destroy(pool);
}

void obj_global_object_pool_list_destroy() {
    obj_list_node_t *node = NULL;
    obj_object_pool_t *pool = NULL;
    obj_list_destroy(g_object_pool_list);
}

/* register to global list */
static obj_bool_t obj_object_pool_register(obj_object_pool_t *pool) {
    return obj_list_add_node_tail(g_object_pool_list, pool);
}

/* create a new pool */
obj_object_pool_t *obj_object_pool_create(int elem_size) {
    /* align 8 */
    elem_size = OBJ_ALIGN(elem_size, 8);
    obj_object_pool_t *pool = (obj_object_pool_t *)obj_alloc(sizeof(obj_object_pool_t));
    if (pool == NULL) {
        return NULL;
    }
    pthread_mutex_init(&pool->block_group_mutex, NULL);
    pthread_mutex_init(&pool->free_chunks_mutex, NULL);
    pthread_mutex_init(&pool->change_thread_mutex, NULL);
    pool->elem_size = elem_size;
    pool->ngroup = 0;
    obj_memset(pool->block_groups, OBJ_OBJECT_POOL_MAX_BLOCK_NGROUP * sizeof(obj_object_pool_block_group_t *), 0);
    /* init freechunks array */
    obj_array_init_size(&pool->free_chunks, elem_size, OBJ_OBJECT_POOL_INITIAL_FREE_LIST_SIZE);
    /* init key */
    pthread_key_create(&pool->key, obj_object_pool_destroy_local_pool);
    pool->nlocal = 0;
    /* register to global list */
    obj_object_pool_register(pool);
    return pool;
}

/* destroy the pool, called last, after clean thread-locals */
static void obj_object_pool_destroy(obj_object_pool_t *pool) {
    obj_assert(pool);
    pthread_mutex_destroy(&pool->block_group_mutex);
    pthread_mutex_destroy(&pool->free_chunks_mutex);
    pthread_mutex_destroy(&pool->change_thread_mutex);
    obj_array_destroy(&pool->free_chunks);
    /* clear object pool */
    pthread_mutex_lock(&pool->change_thread_mutex);
    int nlocal;
    obj_atomic_get(pool->nlocal, nlocal);
    if (nlocal != 0) {
        fprintf(stderr, "nlocal = %d\n", nlocal);
        /* do nothing if there're active threads */
        pthread_mutex_unlock(&pool->change_thread_mutex);
        return;
    }
    /* this is the last thread */
    obj_object_pool_regular_free_chunk_t dummy;
    /* clean global free chunk list */
    while (obj_object_pool_pop_free_chunk(pool, &dummy)) {}
    /* free all memory */
    int ngroup, i, j, nblock;
    obj_object_pool_block_group_t *block_group;
    obj_object_pool_block_t *block;
    ngroup = pool->ngroup;
    for (i = 0; i < ngroup; i++) {
        block_group = pool->block_groups[i];
        pool->block_groups[i] = NULL;
        if (block_group == NULL) {
            /* reach rightmost blockgroup */
            break;
        }
        nblock = block_group->nblocks;
        for (j = 0; j < nblock; j++) {
            block = block_group->blocks[j];
            if (block == NULL) {
                /* reach rightmost block */
                break;
            }
            obj_free(block);
        }
        obj_free(block_group);
    }
    obj_memset(pool->block_groups, 0, sizeof(obj_object_pool_block_group_t *) * OBJ_OBJECT_POOL_MAX_BLOCK_NGROUP);
    pthread_mutex_unlock(&pool->change_thread_mutex);
}

/* create a block and append it to right-most blockgroup */
static obj_object_pool_block_t *obj_object_pool_add_block(obj_object_pool_t *pool, int *index) {
    obj_object_pool_block_t *new_block = obj_object_pool_create_block(pool->elem_size);
    if (new_block == NULL) {
        return NULL;
    }
    int ngroup;
    int block_index;
    volatile obj_object_pool_block_group_t *block_group = NULL;
    do {
        obj_atomic_get(pool->ngroup, ngroup);
        if (ngroup >= 1) {
            /* !!! */
            obj_atomic_get(pool->block_groups[ngroup - 1], block_group);
            obj_atomic_get_incr(block_group->nblocks, block_index, 1);
            if (block_index < OBJ_OBJECT_POOL_GROUP_NBLOCK) {
                obj_atomic_set(block_group->blocks[block_index], new_block);
                *index = (ngroup - 1) * OBJ_OBJECT_POOL_GROUP_NBLOCK + block_index;
                return new_block;
            }
            obj_atomic_decr(block_group->nblocks, 1);
        }
    } while (obj_object_pool_add_block_group(pool, ngroup));
    /* failed to add block_group */
    obj_free(new_block);
    return NULL;
}

static obj_object_pool_block_t *obj_object_pool_create_block(int elem_size) {
    /*
    int n1 = OBJ_OBJECT_POOL_BLOCK_MAX_SIZE / elem_size;
    int n2 = (n1 < 1 ? 1 : n1);
    int nblock = (n2 > OBJ_OBJECT_POOL_BLOCK_MAX_ITEM ? OBJ_OBJECT_POOL_BLOCK_MAX_ITEM : n2);
    */
    int nblock = OBJ_OBJECT_POOL_BLOCK_MAX_ITEM;
    obj_object_pool_block_t *block = (obj_object_pool_block_t *)obj_alloc(sizeof(obj_object_pool_block_t) + nblock * elem_size);
    if (block == NULL) {
        return NULL;
    }
    block->size = nblock * elem_size;
    block->nitem = 0;
    return block;
}

static obj_bool_t obj_object_pool_add_block_group(obj_object_pool_t *pool, int old_ngroup) {
    obj_object_pool_block_group_t *block_group = NULL;
    pthread_mutex_lock(&pool->block_group_mutex);
    int ngroup;
    obj_atomic_get(pool->ngroup, ngroup);
    if (ngroup != old_ngroup) {
        /* other thread got lock and added group before this thread */
        pthread_mutex_unlock(&pool->block_group_mutex);
        return true;
    }
    if (ngroup < OBJ_OBJECT_POOL_MAX_BLOCK_NGROUP) {
        block_group = obj_object_pool_create_block_group();
        if (block_group != NULL) {
            obj_atomic_set(pool->block_groups[ngroup], block_group);
            obj_atomic_set(pool->ngroup, ngroup + 1);
        }
    }
    pthread_mutex_unlock(&pool->block_group_mutex);
    return block_group != NULL;
}

static obj_object_pool_block_group_t *obj_object_pool_create_block_group() {
    obj_object_pool_block_group_t *block_group;
    block_group = (obj_object_pool_block_group_t *)obj_alloc(sizeof(obj_object_pool_block_group_t));
    if (block_group == NULL) {
        return NULL;
    }
    obj_memset(block_group, 0, sizeof(obj_object_pool_block_group_t));
    return block_group;
}

static obj_bool_t obj_object_pool_pop_free_chunk(obj_object_pool_t *pool, obj_object_pool_regular_free_chunk_t *chunk) {
    /* receiver must be a regular chunk */
    if (obj_array_is_empty(&pool->free_chunks)) {
        return false;
    }
    pthread_mutex_lock(&pool->free_chunks_mutex);
    if (obj_array_is_empty(&pool->free_chunks)) {
        pthread_mutex_unlock(&pool->free_chunks_mutex);
        return false;
    }
    obj_object_pool_dynamic_free_chunk_t *ptr = (obj_object_pool_dynamic_free_chunk_t *)obj_array_back(&pool->free_chunks, uintptr_t);
    obj_array_pop_back(&pool->free_chunks);
    pthread_mutex_unlock(&pool->free_chunks_mutex);
    chunk->header.nfree = ptr->header.nfree;
    obj_memcpy(((obj_object_pool_regular_free_chunk_t *)chunk)->ptrs, ptr->ptrs, sizeof(*ptr->ptrs) * ptr->header.nfree);
    obj_free(ptr);
    return true;
}

static obj_bool_t obj_object_pool_push_free_chunk(obj_object_pool_t *pool, obj_object_pool_regular_free_chunk_t chunk) {
    obj_object_pool_dynamic_free_chunk_t *chunk_copy = (obj_object_pool_dynamic_free_chunk_t *)obj_alloc(offsetof(obj_object_pool_dynamic_free_chunk_t, ptrs) + sizeof(*chunk.ptrs) * chunk.header.nfree);
    if (chunk_copy == NULL) {
        return false;
    }
    chunk_copy->header.nfree = chunk.header.nfree;
    obj_memcpy(((obj_object_pool_dynamic_free_chunk_t *)chunk_copy)->ptrs, chunk.ptrs, sizeof(*chunk.ptrs) * chunk.header.nfree);
    pthread_mutex_lock(&pool->free_chunks_mutex);
    /* add to free chunk list */
    obj_array_push_back(&pool->free_chunks, chunk_copy);
    pthread_mutex_unlock(&pool->free_chunks_mutex);
    return true;
}


static obj_object_pool_local_pool_t *obj_object_pool_create_local_pool(obj_object_pool_t *pool) {
    obj_assert(pool);
    obj_object_pool_local_pool_t *local_pool = (obj_object_pool_local_pool_t *)obj_alloc(sizeof(obj_object_pool_local_pool_t));
    if (local_pool == NULL) {
        return NULL;
    }
    local_pool->pool = pool;
    local_pool->cur_block = NULL;
    local_pool->cur_block_index = 0;
    local_pool->cur_free.header.nfree = 0;
    local_pool->cur_free.header.type = OBJ_OBJECT_POOL_CHUNK_REGULAR;
    return local_pool;
}

static void obj_object_pool_destroy_local_pool(void *arg) {
    obj_assert(arg);
    obj_object_pool_t *pool = ((obj_object_pool_local_pool_t *)arg)->pool;
    /* remove thread-local */
    obj_atomic_decr(pool->nlocal, 1);
    pthread_setspecific(pool->key, NULL);
    /* free */
    obj_free((obj_object_pool_local_pool_t *)arg);
}

static obj_object_pool_local_pool_t *obj_object_pool_get_or_new_local_pool(obj_object_pool_t *pool) {
    /* check thread-local */
    obj_object_pool_local_pool_t *local_pool = (obj_object_pool_local_pool_t *)pthread_getspecific(pool->key);
    if (local_pool != NULL) {
        return local_pool;
    }
    local_pool = obj_object_pool_create_local_pool(pool);
    if (local_pool == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&pool->change_thread_mutex);
    /* set thread-local */
    pthread_setspecific(pool->key, local_pool);
    obj_atomic_incr(pool->nlocal, 1);
    pthread_mutex_unlock(&pool->change_thread_mutex);
    return local_pool;
}

/* get object from pool */
inline void *obj_object_pool_get_object(obj_object_pool_t *pool) {
    obj_object_pool_local_pool_t *local_pool = obj_object_pool_get_or_new_local_pool(pool);
    if (local_pool != NULL) {
        return obj_object_pool_get_object_local(local_pool);
    }
    return NULL;
}

/* return the object to pool */
inline int obj_object_pool_return_object(obj_object_pool_t *pool, void *ptr) {
    obj_object_pool_local_pool_t *local_pool = obj_object_pool_get_or_new_local_pool(pool);
    if (local_pool != NULL) {
        return obj_object_pool_return_object_local(local_pool, ptr);
    }
    return -1;
}


static void *obj_object_pool_get_object_local(obj_object_pool_local_pool_t *local_pool) {
    /* try to fetch from local free chunk */
    if (local_pool->cur_free.header.nfree) {
        return local_pool->cur_free.ptrs[--local_pool->cur_free.header.nfree];
    }
    /* try to fetch from global */
    if (obj_object_pool_pop_free_chunk(local_pool->pool, &local_pool->cur_free)) {
        return local_pool->cur_free.ptrs[--local_pool->cur_free.header.nfree];
    }
    void *ret;
    /* fetch memory from local block */
    if (local_pool->cur_block && local_pool->cur_block->nitem < OBJ_OBJECT_POOL_BLOCK_MAX_ITEM) {
        ret = local_pool->cur_block->items + local_pool->cur_block->nitem * local_pool->pool->elem_size;
        /* TODO need init ? */
        ++local_pool->cur_block->nitem;
        return ret;
    }
    /* fetch a block from global */
    local_pool->cur_block = obj_object_pool_add_block(local_pool->pool, &local_pool->cur_block_index);
    if (local_pool->cur_block != NULL) {
        ret = local_pool->cur_block->items + local_pool->cur_block->nitem * local_pool->pool->elem_size;
        /* TODO need init ? */
        ++local_pool->cur_block->nitem;
        return ret;
    }
    /* out of memory */
    return NULL;
}

static int obj_object_pool_return_object_local(obj_object_pool_local_pool_t *local_pool, void *ptr) {
    /* try to return to local free list */
    if (local_pool->cur_free.header.nfree < OBJ_OBJECT_POOL_FREE_CHUNK_MAX_ITEM) {
        local_pool->cur_free.ptrs[local_pool->cur_free.header.nfree++] = ptr;
        return 0;
    }
    /* local full, return to global */
    if (obj_object_pool_push_free_chunk(local_pool->pool, local_pool->cur_free)) {
        local_pool->cur_free.header.nfree = 1;
        local_pool->cur_free.ptrs[0] = ptr;
        return 0;
    }
    return -1;
}

