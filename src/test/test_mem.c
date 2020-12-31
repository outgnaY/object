#include "obj_core.h"

typedef struct test_s test_t;

struct test_s {
    char arr[1506];
};

#define ARR_LEN 100000
test_t *test_arr[ARR_LEN];

int main() {
    obj_global_mem_context_init();
    int i;
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    for (i = 0; i < ARR_LEN; i++) {
        test_arr[i] = (test_t *)obj_alloc(sizeof(test_t));
        test_arr[i]->arr[1505] = 'c';
        // test_t *p = (test_t *)obj_lib_alloc(sizeof(test_t));
        // void *p = obj_alloc(rand() % 8192);
        // obj_lib_free(p);
    }
    gettimeofday(&end, NULL);
    printf("time: %ld %ld\n", end.tv_sec - start.tv_sec, end.tv_usec - start.tv_usec);
    for (i = 0; i < ARR_LEN; i++) {
        assert(test_arr[i]->arr[1505] == 'c');
    }
    obj_global_mem_context_stats();
    obj_global_mem_context_reset();
    
    return 0;
}