#include "mem/obj_mem.h"
#include "mem/obj_mem_simple.h"
#include "util/obj_string.h"

obj_mem_context_t *g_context;

static obj_mem_context_methods_t obj_mem_simple_context_methods = {
    obj_mem_simple_context_init,
    obj_mem_simple_context_reset,
    obj_mem_simple_context_delete,
    obj_mem_simple_context_stats,
    obj_mem_simple_context_alloc,
    obj_mem_simple_context_free,
    obj_mem_simple_context_realloc,
};

static void *obj_mem_simple_context_alloc_nolock(obj_mem_context_t *context, obj_size_t size);
static void obj_mem_simple_context_free_nolock(obj_mem_context_t *context, void *ptr);


/* find freelist index of given size, caller must ensure that size <= OBJ_ALLOC_CHUNK_LIMIT */
static obj_inline int obj_mem_free_index(obj_size_t size) {
    int idx = 0;
    if (size > 0) {
        size = (size - 1) >> OBJ_ALLOC_MINBITS;
        while (size != 0) {
            idx++;
            size >>= 1;
        }
        obj_assert(idx < OBJ_MEM_SIMPLE_CONTEXT_FREELISTS);
    }
    return idx;
}


obj_mem_context_t *obj_mem_simple_context_create(obj_size_t init_pool_size, obj_size_t max_pool_size) {
    obj_mem_simple_context_t *simple_context;
    int i;
    simple_context = (obj_mem_simple_context_t *) obj_mem_context_create(sizeof(obj_mem_simple_context_t), &obj_mem_simple_context_methods);
    /* init fields of mem context */
    init_pool_size = OBJ_MEM_ALIGN(init_pool_size);
    if (init_pool_size < OBJ_MIN_MEM_POOL_SIZE) {
        init_pool_size = OBJ_MIN_MEM_POOL_SIZE;
    }
    max_pool_size = OBJ_MEM_ALIGN(max_pool_size);
    if (max_pool_size < init_pool_size) {
        max_pool_size = init_pool_size;
    }
    simple_context->init_pool_size = init_pool_size;
    simple_context->max_pool_size = max_pool_size;
    simple_context->next_pool_size = init_pool_size;

    simple_context->alloc_chunk_limit = OBJ_MEM_CHUNK_LIMIT;
    while (simple_context->alloc_chunk_limit > (obj_size_t)(max_pool_size - OBJ_MEM_POOL_HEADER_SIZE - OBJ_MEM_CHUNK_HEADER_SIZE)) {
        simple_context->alloc_chunk_limit >>= 1;
    }
    /* init freelists */
    for (i = 0; i < OBJ_MEM_SIMPLE_CONTEXT_FREELISTS; i++) {
        simple_context->freelist[i] = NULL;
    }
    simple_context->is_reset = true;
    pthread_mutex_init(&simple_context->lock, NULL);
    return (obj_mem_context_t *)simple_context;
}

void obj_mem_simple_context_init(obj_mem_context_t *context) {
    /* do nothing */
}

void obj_mem_simple_context_reset(obj_mem_context_t *context) {
    obj_mem_simple_context_t *simple_context = (obj_mem_simple_context_t *)context;
    pthread_mutex_lock(&simple_context->lock);
    obj_mem_pool_t *pool = simple_context->pools;
    if (simple_context->is_reset) {
        return;
    }
    int i;
    for (i = 0; i < OBJ_MEM_SIMPLE_CONTEXT_FREELISTS; i++) {
        simple_context->freelist[i] = NULL;
    }
    simple_context->pools = NULL;
    obj_mem_pool_t *next;
    while (pool != NULL) {
        next = pool->next;
        obj_lib_free(pool);
        pool = next;
    }
    simple_context->next_pool_size = simple_context->init_pool_size;
    simple_context->is_reset = true;
    pthread_mutex_unlock(&simple_context->lock);
}

void obj_mem_simple_context_delete(obj_mem_context_t *context) {
    obj_mem_simple_context_t *simple_context = (obj_mem_simple_context_t *)context;
    pthread_mutex_lock(&simple_context->lock);
    obj_mem_pool_t *pool = simple_context->pools;
    int i;
    for (i = 0; i < OBJ_MEM_SIMPLE_CONTEXT_FREELISTS; i++) {
        simple_context->freelist[i] = NULL;
    }
    simple_context->pools = NULL;
    obj_mem_pool_t *next;
    /* free pools */
    while (pool != NULL) {
        next = pool->next;
        obj_lib_free(pool);
        pool = next;
    }
    pthread_mutex_unlock(&simple_context->lock);
}

void obj_mem_simple_context_stats(obj_mem_context_t *context) {
    obj_mem_simple_context_t *simple_context = (obj_mem_simple_context_t *)context;
    obj_mem_pool_t *pool;
    obj_mem_chunk_t *chunk;
    int pool_num = 0;
    int chunk_num = 0;
    unsigned long total_space = 0;
    unsigned long free_space = 0;
    int idx;
    pthread_mutex_lock(&simple_context->lock);
    for (pool = simple_context->pools; pool != NULL; pool = pool->next) {
        pool_num++;
        total_space += (pool->end_ptr - ((char *)pool));
        free_space += (pool->end_ptr - pool->free_ptr);
    }
    for (idx = 0; idx < OBJ_MEM_SIMPLE_CONTEXT_FREELISTS; idx++) {
        for (chunk = simple_context->freelist[idx]; chunk != NULL; chunk = (obj_mem_chunk_t *)chunk->ptr) {
            chunk_num++;
            free_space += chunk->size + OBJ_MEM_CHUNK_HEADER_SIZE;
        }
    }
    pthread_mutex_unlock(&simple_context->lock);
    fprintf(stderr, "%lu total in %d pools; %lu free (%d chunks); %lu used\n", 
    total_space, pool_num, free_space, chunk_num, total_space - free_space);
}

void *obj_mem_simple_context_alloc(obj_mem_context_t *context, obj_size_t size) {
    obj_mem_simple_context_t *simple_context = (obj_mem_simple_context_t *)context;
    void *ptr;
    pthread_mutex_lock(&simple_context->lock);
    ptr = obj_mem_simple_context_alloc_nolock(context, size);
    pthread_mutex_unlock(&simple_context->lock);
    return ptr;
}

static void *obj_mem_simple_context_alloc_nolock(obj_mem_context_t *context, obj_size_t size) {
    obj_mem_simple_context_t *simple_context = (obj_mem_simple_context_t *)context;
    obj_mem_pool_t *pool;
    obj_mem_chunk_t *chunk;
    int idx;
    obj_size_t chunk_size;
    obj_size_t pool_size;
    if (size > simple_context->alloc_chunk_limit) {
        /* alloc an entire block */
        chunk_size = OBJ_MEM_ALIGN(size);
        pool_size = chunk_size + OBJ_MEM_POOL_HEADER_SIZE + OBJ_MEM_CHUNK_HEADER_SIZE;
        pool = (obj_mem_pool_t *)obj_lib_alloc(pool_size);
        if (pool == NULL) {
            fprintf(stderr, "can't alloc size %lu\n", pool_size);
            return NULL;
        }
        pool->context = simple_context;
        /* set used */
        pool->free_ptr = pool->end_ptr = ((char *)pool) + pool_size;
        chunk = (obj_mem_chunk_t *)(((char *)pool) + OBJ_MEM_POOL_HEADER_SIZE);
        chunk->ptr = simple_context;
        chunk->size = chunk_size;
        /* set pointers */
        if (simple_context->pools != NULL) {
            /* insert into next */
            pool->next = simple_context->pools->next;
            if (simple_context->pools->next != NULL) {
                simple_context->pools->next->prev = pool;
            }
            pool->prev = simple_context->pools;
            simple_context->pools->next = pool;
            
        } else {
            pool->next = NULL;
            pool->prev = NULL;
            simple_context->pools = pool;
        }
        simple_context->is_reset = false;
        return OBJ_MEM_CHUNK_GET_PTR(chunk);
    }
    /* request is small, search in the freelist first */
    idx = obj_mem_free_index(size);
    chunk = simple_context->freelist[idx];
    /* find */
    if (chunk != NULL) {
        simple_context->freelist[idx] = (obj_mem_chunk_t *)chunk->ptr;
        chunk->ptr = (void *)simple_context;
        return OBJ_MEM_CHUNK_GET_PTR(chunk);
    }
    /* freelist is empty */
    chunk_size = (1 << OBJ_ALLOC_MINBITS) << idx;
    /* check if there is enough room */
    if ((pool = simple_context->pools) != NULL) {
        obj_size_t avail = pool->end_ptr - pool->free_ptr;
        /* if not enough room, cut into pieces and insert into freelist */
        if (avail < (chunk_size + OBJ_MEM_CHUNK_HEADER_SIZE)) {
            obj_size_t avail_chunk;
            int a_idx;
            while (avail >= ((1 << OBJ_ALLOC_MINBITS) + OBJ_MEM_CHUNK_HEADER_SIZE)) {
                avail_chunk = avail - OBJ_MEM_CHUNK_HEADER_SIZE;
                a_idx = obj_mem_free_index(avail_chunk);
                /* check if avail_chunk is power of 2 */
                if (avail_chunk != (1 << (a_idx + OBJ_ALLOC_MINBITS))) {
                    a_idx--;
                    avail_chunk = (1 << (a_idx + OBJ_ALLOC_MINBITS));
                }
                chunk = (obj_mem_chunk_t *)(pool->free_ptr);
                pool->free_ptr += (avail_chunk + OBJ_MEM_CHUNK_HEADER_SIZE);
                avail -= (avail_chunk + OBJ_MEM_CHUNK_HEADER_SIZE);
                chunk->size = avail_chunk;
                chunk->ptr = (void *)simple_context->freelist[a_idx];
                simple_context->freelist[a_idx] = chunk;
            }
            /* mark we need to create a new block */
            pool = NULL;
        }
    }
    /* we need to create a new block */
    if (pool == NULL) {
        obj_size_t required_size;
        pool_size = simple_context->next_pool_size;
        simple_context->next_pool_size <<= 1;
        if (simple_context->next_pool_size > simple_context->max_pool_size) {
            simple_context->next_pool_size = simple_context->max_pool_size;
        }
        required_size = chunk_size + OBJ_MEM_POOL_HEADER_SIZE + OBJ_MEM_CHUNK_HEADER_SIZE;
        while (pool_size < required_size) {
            pool_size <<= 1;
        }
        pool = (obj_mem_pool_t *)obj_lib_alloc(pool_size);
        /*
        while (pool == NULL && pool_size > 1024 * 1024) {
            pool_size >>= 1;
            if (pool_size < required_size) {
                break;
            }
            pool = (obj_mem_pool_t *)obj_lib_alloc(pool_size);
        }
        */
        if (pool == NULL) {
            return NULL;
        }
        pool->context = simple_context;
        pool->free_ptr = ((char *)pool) + OBJ_MEM_POOL_HEADER_SIZE;
        pool->end_ptr = ((char *)pool) + pool_size;
        /* set pointers */
        pool->next = simple_context->pools;
        if (pool->next != NULL) {
            pool->next->prev = pool;
        }
        pool->prev = NULL;
        simple_context->pools = pool; 
    }
    /* do the allocation */
    chunk = (obj_mem_chunk_t *)(pool->free_ptr);
    pool->free_ptr += (chunk_size + OBJ_MEM_CHUNK_HEADER_SIZE);
    chunk->ptr = (void *)simple_context;
    chunk->size = chunk_size;
    simple_context->is_reset = false;
    return OBJ_MEM_CHUNK_GET_PTR(chunk);
}

void obj_mem_simple_context_free(obj_mem_context_t *context, void *ptr) {
    obj_mem_simple_context_t *simple_context = (obj_mem_simple_context_t *)context;
    pthread_mutex_lock(&simple_context->lock);
    obj_mem_simple_context_free_nolock(context, ptr);
    pthread_mutex_unlock(&simple_context->lock);
}

static void obj_mem_simple_context_free_nolock(obj_mem_context_t *context, void *ptr) {
    obj_mem_simple_context_t *simple_context = (obj_mem_simple_context_t *)context;
    obj_mem_chunk_t *chunk = OBJ_PTR_GET_MEM_CHUNK(ptr);
    if (chunk->size > simple_context->alloc_chunk_limit) {
        /* big chunk */
        obj_mem_pool_t *pool = (obj_mem_pool_t *)OBJ_SINGLE_CHUNK_GET_POOL(chunk);
        /* remove from pool list */
        if (pool->prev != NULL) {
            pool->prev->next = pool->next;
        }else {
            simple_context->pools = pool->next;
        }
        if (pool->next != NULL) {
            pool->next->prev = pool->prev;
        }
        free(pool);

    } else {
        /* small chunk, normal case */
        int idx = obj_mem_free_index(chunk->size);
        /* insert into free list */
        chunk->ptr = (void *)simple_context->freelist[idx];
        simple_context->freelist[idx] = chunk;
    }
}

void *obj_mem_simple_context_realloc(obj_mem_context_t *context, void *ptr, obj_size_t size) {
    obj_mem_simple_context_t *simple_context = (obj_mem_simple_context_t *)context;
    pthread_mutex_lock(&simple_context->lock);
    obj_mem_chunk_t *chunk = OBJ_PTR_GET_MEM_CHUNK(ptr);
    obj_size_t old_size = chunk->size;
    if (old_size >= size) {
        return ptr;
    }
    if (old_size > simple_context->alloc_chunk_limit) {
        obj_mem_pool_t *pool = (obj_mem_pool_t *)OBJ_SINGLE_CHUNK_GET_POOL(chunk);
        obj_mem_pool_t *prev_pool = pool->prev;
        obj_mem_pool_t *next_pool = pool->next;
        obj_size_t chunk_size;
        obj_size_t pool_size;
        chunk_size = OBJ_MEM_ALIGN(size);
        pool_size = chunk_size + OBJ_MEM_POOL_HEADER_SIZE + OBJ_MEM_CHUNK_HEADER_SIZE;
        /* realloc */
        pool = (obj_mem_pool_t *)realloc(pool, pool_size);
        pool->free_ptr = pool->end_ptr = ((char *)pool) + pool_size;
        chunk = (obj_mem_chunk_t *)(((char *)pool) + OBJ_MEM_POOL_HEADER_SIZE);
        /* set pointers */
        if (prev_pool == NULL) {
            simple_context->pools = pool;
        } else {
            prev_pool->next = pool;
        }

        if (next_pool != NULL) {
            next_pool->prev = pool;
        } 
        /*
        pool->prev = prev_pool;
        pool->next = next_pool;
        */
        chunk->size = chunk_size;
        pthread_mutex_unlock(&simple_context->lock);
        return OBJ_MEM_CHUNK_GET_PTR(chunk);

    } else {
        void *new_ptr = obj_mem_simple_context_alloc_nolock(context, size);
        obj_memcpy(new_ptr, ptr, old_size);
        /* free old chunk */
        obj_mem_simple_context_free_nolock(context, ptr);
        pthread_mutex_unlock(&simple_context->lock);
        return new_ptr;
    }
}