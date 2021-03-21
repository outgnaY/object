#ifndef OBJ_SET_H
#define OBJ_SET_H

#include "obj_core.h"

/* implement of set */

typedef enum obj_set_error_code obj_set_error_code_t;
typedef struct obj_set_methods_s obj_set_methods_t;
typedef struct obj_set_s obj_set_t;


struct obj_set_s {
    obj_prealloc_map_t map;
};


obj_uint64_t obj_set_hash_function(const void *key, int len);
obj_set_t *obj_set_create(obj_prealloc_map_methods_t *methods, int element_size);
obj_bool_t obj_set_init(obj_set_t *set, obj_prealloc_map_methods_t *methods, int element_size);
void obj_set_destroy_static(obj_set_t *set);
void obj_set_destroy(obj_set_t *set);
obj_bool_t obj_set_find(obj_set_t *set, void *key);
void obj_set_add(obj_set_t *set, void *key);


#endif  /* OBJ_SET_H */