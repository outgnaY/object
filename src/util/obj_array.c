#include "obj_core.h"

static void obj_array_ensure_capacity(obj_array_t *array, int capacity);

/* create */
obj_array_t *obj_array_create(int element_size) {
    return obj_array_create_size(element_size, OBJ_ARRAY_INIT_SIZE_DEFAULT);
}

/* create with initial size */
obj_array_t *obj_array_create_size(int element_size, int init_size) {
    obj_array_t *array = obj_alloc(sizeof(obj_array_t));
    array->data = obj_alloc(element_size * init_size);
    /* array->flag = 0; */
    array->element_size = element_size;
    array->free = NULL;
    array->size = 0;
    array->capacity = init_size;
    return array;
}

/* init */
void obj_array_init(obj_array_t *array, int element_size) {
    obj_assert(array);
    return obj_array_init_size(array, element_size, OBJ_ARRAY_INIT_SIZE_DEFAULT);
}

/* init with size */
void obj_array_init_size(obj_array_t *array, int element_size, int init_size) {
    array->data = obj_alloc(element_size * init_size);
    /* array->flag = OBJ_ARRAY_FLAG_STATIC; */
    array->element_size = element_size;
    array->free = NULL;
    array->size = 0;
    array->capacity = init_size;
}

void obj_array_destroy_static(obj_array_t *array) {
    obj_assert(array);
    obj_array_empty(array);
    obj_free(array->data);
}

/* destroy */
void obj_array_destroy(obj_array_t *array) {
    obj_array_destroy_static(array);
    obj_free(array);
}

/* empty */
void obj_array_empty(obj_array_t *array) {
    obj_assert(array);
    int size = array->size;
    if (array->free) {
        while (size--) {
            array->free(array->data + size * array->element_size);
        }
    }
    array->size = 0;
}

void obj_array_reserve(obj_array_t *array, int capacity) {
    obj_array_ensure_capacity(array, capacity);
}

void obj_array_resize(obj_array_t *array, int size) {
    if (array->capacity >= size) {
        array->size = size;
        return;
    }
    obj_array_ensure_capacity(array, size);
    array->size = size;
}


/* ensure capacity */
static void obj_array_ensure_capacity(obj_array_t *array, int capacity) {
    if (array->capacity >= capacity) {
        return;
    }
    int new_cap = capacity > array->capacity * 2 ? capacity : array->capacity * 2;
    /* check overflow */
    if (new_cap < 0 || new_cap > OBJ_ARRAY_SIZE_MAX) {
        return;
    }
    array->data = obj_realloc(array->data, new_cap * array->element_size);
    array->capacity = new_cap;
}

/* get element at index */
void *obj_array_get_index(obj_array_t *array, int index) {
    obj_assert(array && array->data && array->size > index);
    return array->data + index * array->element_size;
}


/* set element at index */
void obj_array_set_index(obj_array_t *array, int index, void *element_ptr) {
    obj_assert(array && array->data && array->size > index);
    obj_memcpy(array->data + index * array->element_size, element_ptr, array->element_size);
}

/* push back */
void obj_array_push_back(obj_array_t *array, void *element_ptr) {
    obj_assert(array);
    obj_array_ensure_capacity(array, array->size + 1);
    obj_memcpy(array->data + array->size * array->element_size, element_ptr, array->element_size);
    array->size++;
}


/* pop back */
void obj_array_pop_back(obj_array_t *array) {
    obj_assert(array && array->size > 0);
    int new_size = array->size - 1;
    if (array->free) {
        array->free(array->data + new_size * array->element_size);
    }
    array->size = new_size;
}

/* insert */
void obj_array_insert(obj_array_t *array, int index, void *element_ptr) {
    /* index == array->size means append */
    obj_array_ensure_capacity(array, array->size + 1);
    /* move */
    obj_memmove(array->data + (index + 1) * array->element_size, array->data + index * array->element_size, (array->size - index) * array->element_size);
    array->size++;
    /* set */
    obj_array_set_index(array, index, element_ptr);
}

/* remove */
void obj_array_remove(obj_array_t *array, int index) {
    obj_assert(array && array->size > index);
    /* clean */
    if (array->free) {
        array->free(array->data + index * array->element_size);
    }
    /* move */
    obj_memmove(array->data + index * array->element_size, array->data + (index + 1) * array->element_size, (array->size - index - 1) * array->element_size);
    array->size--;
}

/* use for debug */
void obj_array_dump(obj_array_t *array, void (*cb)(obj_array_t *array)) {
    printf("**************** dump array ****************\n");
    printf("array size: %d\n", array->size);
    printf("array element size: %d\n", array->element_size);
    printf("array capacity: %d\n", array->capacity);
    if (cb) {
        cb(array);
    }
}

/* sort the array with given compare function */
void obj_array_sort(obj_array_t *array, int (*compare)(const void *a, const void *b)) {
    qsort(array->data, array->size, array->element_size, compare);
}
