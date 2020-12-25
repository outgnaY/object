#include "mem/obj_mem.h"
#include "mem/obj_mem_simple.h"
#include "util/obj_string.h"

void *obj_lib_alloc(obj_size_t size) {
    void *p;
    p = malloc(size);
    /* failed */
    if (p == NULL) {
        fprintf(stderr, "malloc %lu failed\n", size);
    }
    return p;
}

void *obj_lib_calloc(obj_size_t size) {
    void *p;
    p = obj_lib_alloc(size);
    if (p) {
        obj_memzero(p, size);
    }
    return p;
}

void obj_global_mem_context_init() {
    g_context = obj_mem_simple_context_create(1 * 1024, 32 * 1024);
}

void obj_global_mem_context_reset() {
    obj_mem_context_reset(g_context);
}

void obj_global_mem_context_delete() {
    obj_mem_context_delete(g_context);
}

void obj_global_mem_context_stats() {
    obj_mem_context_stats(g_context);
}

obj_mem_context_t *obj_mem_context_create(obj_size_t size, obj_mem_context_methods_t *methods) {
    obj_mem_context_t *context;
    context = (obj_mem_context_t *)obj_lib_alloc(size);
    obj_assert(context != NULL);
    context->methods = methods;
    return context;
}

void obj_mem_context_init(obj_mem_context_t *context) {
    (*context->methods->init)(context);
}

void obj_mem_context_reset(obj_mem_context_t *context) {
    (*context->methods->reset)(context);
}

void obj_mem_context_delete(obj_mem_context_t *context) {
    (*context->methods->delete)(context);
}

void obj_mem_context_stats(obj_mem_context_t *context) {
    (*context->methods->stats)(context);
}

void *obj_mem_context_alloc(obj_mem_context_t *context, obj_size_t size) {
    return (*context->methods->alloc)(context, size);
}

void obj_mem_context_free(obj_mem_context_t *context, void *ptr) {
    (*context->methods->free)(context, ptr);
}

void *obj_mem_context_realloc(obj_mem_context_t *context, void *ptr, obj_size_t size) {
    return (*context->methods->realloc)(context, ptr, size);
}

