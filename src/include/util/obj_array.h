#ifndef OBJ_ARRAY_H
#define OBJ_ARRAY_H

typedef enum obj_array_flag obj_array_flag_t;
typedef struct obj_array_s obj_array_t;

/*
enum obj_array_flag {
    OBJ_ARRAY_FLAG_STATIC = 1           
};
*/

struct obj_array_s {
    /* obj_array_flag_t flag; */
    void *data;
    int size;
    int element_size;
    int capacity;
    /* free element */
    void (*free)(void *ptr);
};


#define OBJ_ARRAY_INIT_SIZE_DEFAULT 8
#define OBJ_ARRAY_EXPAND_FACTOR_DEFAULT 2
#define OBJ_ARRAY_SIZE_MAX 0x7fffffff

#define obj_array_set_flag(arr, flag) (((arr)->flag) = flag)
#define obj_array_set_free(arr, fn) (((arr)->free) = fn)
#define obj_array_is_empty(arr) (((arr)->size) == 0)
#define obj_array_length(arr) ((arr)->size)
#define obj_array_get_index_value(arr, i, T) *((T *)obj_array_get_index(arr, i))
#define obj_array_back(arr, T) obj_array_get_index_value(arr, (arr)->size - 1, T)

obj_array_t *obj_array_create(int element_size);
obj_array_t *obj_array_create_size(int element_size, int size);
void obj_array_init(obj_array_t *array, int element_size);
void obj_array_init_size(obj_array_t *array, int element_size, int size);
void obj_array_destroy_static(obj_array_t *array);
void obj_array_destroy(obj_array_t *array);
void obj_array_empty(obj_array_t *array);
void obj_array_reserve(obj_array_t *array, int capacity);
void obj_array_resize(obj_array_t *array, int size);
void *obj_array_get_index(obj_array_t *array, int index);
void obj_array_set_index(obj_array_t *array, int index, void *element_ptr);
void obj_array_push_back(obj_array_t *array, void *element_ptr);
void obj_array_pop_back(obj_array_t *array);
void obj_array_insert(obj_array_t *array, int index, void *element_ptr);
void obj_array_remove(obj_array_t *array, int index);
void obj_array_dump(obj_array_t *array, void (*cb)(obj_array_t *array));
void obj_array_sort(obj_array_t *array, int (*compare)(const void *a, const void *b));

#endif  /* OBJ_ARRAY_H */