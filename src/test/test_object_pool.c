#include "obj_core.h"

typedef struct test_s test_t;

struct test_s {
    char c;
    int i;
    unsigned long l;
};

void *fn(void *arg) {
    obj_object_pool_t *pool = (obj_object_pool_t *)arg;
    pthread_t id = pthread_self();
    test_t *test1, *test2;
    int i;
    for (i = 0; i < 1; i++) {
        test1 = (test_t *)obj_object_pool_get_object(pool);
        /* test2 = (test_t *)obj_object_pool_get_object(pool); */
        test1->c = 'c';
        test1->i = i;
        test1->l = id;
        /* fprintf(stderr, "thread[%d]: %p %p\n", id, test1, test2); */
        /*
        obj_object_pool_return_object(pool, test2);
        obj_object_pool_return_object(pool, test1);
        */
    }
    return NULL;
}

void *fn_alloc(void *arg) {
    int i;
    test_t *test1, *test2;
    for (i = 0; i < 100000; i++) {
        test1 = (test_t *)obj_alloc(sizeof(test_t));
        test2 = (test_t *)obj_alloc(sizeof(test_t));
    }
    return NULL;
}

void time_interval(struct timeval t1, struct timeval t2) {
    int us = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
    printf("total time: %ds %dus\n", us / 1000000, us - (us / 1000000) * 1000000);
}

int main() {
    pthread_t tid[16];
    int i;
    struct timeval start;
    struct timeval end;
    obj_global_mem_context_init();
    gettimeofday(&start, NULL);
    obj_global_object_pool_list_init();
    obj_object_pool_t *pool = obj_object_pool_create(sizeof(test_t));
    for (i = 0; i < 16; i++) {
        pthread_create(&tid[i], NULL, fn, pool);
        /* pthread_create(&tid[i], NULL, fn_alloc, NULL); */
    }
    for (i = 0; i < 16; i++) {
        pthread_join(tid[i], NULL);
    }
    obj_global_object_pool_list_destroy();
    gettimeofday(&end, NULL);
    time_interval(start, end);
    return 0;
}