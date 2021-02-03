#include "obj_core.h"

typedef struct simple_s simple_t;
typedef struct complicated_s complicated_t;

struct simple_s {
    int a;
    int b;
};

struct complicated_s {
    int a;
    char *b;
    double c;
};

void clean(void *ptr) {
    complicated_t *p = (complicated_t *)ptr;
    if (p->b) {
        obj_free(p->b);
    }
}

char *make_string(const char *s) {
    int len = obj_strlen(s);
    char *scpy = obj_alloc(len + 1);
    obj_memcpy(scpy, s, len);
    scpy[len] = '\0';
    return scpy;
}

void array1_cb(obj_array_t *array) {
    int i;
    for (i = 0; i < array->size; i++) {
        printf("[%d]: %d ", i, *((int *)(obj_array_get_index(array, i))));
    }
    printf("\n");
}

void array3_cb(obj_array_t *array) {
    int i;
    simple_t *ptr;
    for (i = 0; i < array->size; i++) {
        ptr = ((simple_t *)(obj_array_get_index(array, i)));
        printf("[%d]: {%d, %d} ", i, ptr->a, ptr->b);
    }
    printf("\n");
}

void array4_cb(obj_array_t *array) {
    int i;
    complicated_t *ptr;
    for (i = 0; i < array->size; i++) {
        ptr = ((complicated_t *)(obj_array_get_index(array, i)));
        printf("[%d]: {%d, %s %lf} ", i, ptr->a, ptr->b, ptr->c);
    }
    printf("\n");
}

void time_interval(struct timeval t1, struct timeval t2) {
    int us = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
    printf("total time: %ds %dus\n", us / 1000000, us - (us / 1000000) * 1000000);
}

/* array test */
int main() {
    obj_global_mem_context_init();
    /* array with int */
    /*
    obj_array_t *array1 = obj_array_create(sizeof(int));
    int a1 = 2;
    obj_array_push_back(array1, &a1);
    obj_array_dump(array1, array1_cb);
    a1 = 3;
    obj_array_push_back(array1, &a1);
    obj_array_dump(array1, array1_cb);
    a1 = 4;
    obj_array_insert(array1, 1, &a1);
    obj_array_dump(array1, array1_cb);
    obj_array_remove(array1, 0);
    obj_array_dump(array1, array1_cb);
    obj_array_remove(array1, 1);
    obj_array_dump(array1, array1_cb);
    obj_array_push_back(array1, &a1);
    obj_array_push_back(array1, &a1);
    obj_array_push_back(array1, &a1);
    obj_array_push_back(array1, &a1);
    obj_array_push_back(array1, &a1);
    obj_array_push_back(array1, &a1);
    obj_array_push_back(array1, &a1);
    obj_array_push_back(array1, &a1);
    obj_array_dump(array1, array1_cb);
    obj_array_destroy(array1);
    */
    /* array from stack */
    /*
    obj_array_t array2;
    obj_array_init(&array2, sizeof(int));
    int a1 = 2;
    obj_array_push_back(&array2, &a1);
    obj_array_dump(&array2, array1_cb);
    a1 = 3;
    obj_array_push_back(&array2, &a1);
    obj_array_dump(&array2, array1_cb);
    a1 = 4;
    obj_array_insert(&array2, 1, &a1);
    obj_array_dump(&array2, array1_cb);
    obj_array_remove(&array2, 0);
    obj_array_dump(&array2, array1_cb);
    obj_array_remove(&array2, 1);
    obj_array_dump(&array2, array1_cb);
    obj_array_push_back(&array2, &a1);
    obj_array_push_back(&array2, &a1);
    obj_array_push_back(&array2, &a1);
    obj_array_push_back(&array2, &a1);
    obj_array_push_back(&array2, &a1);
    obj_array_push_back(&array2, &a1);
    obj_array_push_back(&array2, &a1);
    obj_array_push_back(&array2, &a1);
    obj_array_dump(&array2, array1_cb);
    obj_array_destroy(&array2);
    */
    /* array with simple struct */
    /*
    obj_array_t array3;
    obj_array_init(&array3, sizeof(simple_t));
    simple_t a1 = {3, 7};
    obj_array_push_back(&array3, &a1);
    obj_array_dump(&array3, array3_cb);
    a1.b = 5;
    obj_array_push_back(&array3, &a1);
    obj_array_dump(&array3, array3_cb);
    a1.a = 4;
    a1.b = 7;
    obj_array_insert(&array3, 1, &a1);
    obj_array_dump(&array3, array3_cb);
    obj_array_remove(&array3, 0);
    obj_array_dump(&array3, array3_cb);
    obj_array_remove(&array3, 1);
    obj_array_dump(&array3, array3_cb);
    obj_array_push_back(&array3, &a1);
    obj_array_push_back(&array3, &a1);
    obj_array_push_back(&array3, &a1);
    obj_array_push_back(&array3, &a1);
    obj_array_push_back(&array3, &a1);
    obj_array_push_back(&array3, &a1);
    obj_array_push_back(&array3, &a1);
    obj_array_push_back(&array3, &a1);
    obj_array_dump(&array3, array3_cb);
    obj_array_destroy(&array3);
    */
    /* array with complicated struct */
    /*
    obj_array_t array4;
    obj_array_init(&array4, sizeof(complicated_t));
    obj_array_set_free(&array4, clean);
    complicated_t a1 = {4, make_string("abcdefg"), 5.06};
    obj_array_push_back(&array4, &a1);
    obj_array_dump(&array4, array4_cb);
    a1.b = make_string("bcd");
    a1.c = 6.06;
    obj_array_push_back(&array4, &a1);
    obj_array_dump(&array4, array4_cb);
    a1.a = 4;
    a1.b = make_string("cde");
    a1.c = 7.07;
    obj_array_insert(&array4, 1, &a1);
    obj_array_dump(&array4, array4_cb);
    obj_array_remove(&array4, 0);
    obj_array_dump(&array4, array4_cb);
    obj_array_remove(&array4, 1);
    obj_array_dump(&array4, array4_cb);
    a1.b = make_string("cdef");
    obj_array_push_back(&array4, &a1);
    a1.b = make_string("cdefg");
    obj_array_push_back(&array4, &a1);
    a1.b = make_string("cdefgh");
    obj_array_push_back(&array4, &a1);
    a1.b = make_string("cdefghi");
    obj_array_push_back(&array4, &a1);
    a1.b = make_string("cdefghij");
    obj_array_push_back(&array4, &a1);
    a1.b = make_string("cdefghijk");
    obj_array_push_back(&array4, &a1);
    a1.b = make_string("cdefghijklm");
    obj_array_push_back(&array4, &a1);
    a1.b = make_string("cdefghijklmn");
    obj_array_push_back(&array4, &a1);
    obj_array_dump(&array4, array4_cb);
    obj_array_destroy(&array4);
    */
    /* time */
    int i;
    int a = 1;
    struct timeval start;
    struct timeval end;
    /*
    gettimeofday(&start, NULL);
    obj_list_t *list = obj_list_create();
    for (i = 0; i < 1000000; i++) {
        obj_list_add_node_tail(list, &a);
    }
    gettimeofday(&end, NULL);
    time_interval(start, end);
    */
    gettimeofday(&start, NULL);
    obj_array_t *array = obj_array_create(sizeof(int));
    for (i = 0; i < 1000000; i++) {
        obj_array_push_back(array, &a);
    }
    gettimeofday(&end, NULL);
    time_interval(start, end);
    return 0;
}