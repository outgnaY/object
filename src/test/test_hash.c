#include "obj_core.h"

typedef struct test_arg_s test_arg_t;

struct test_arg_s {
    obj_hash_table_t *table;
    int index;
};

void *test_func(void *arg) {
    test_arg_t *argument = (test_arg_t *)arg;
    obj_hash_table_t *table = argument->table;
    int index = argument->index;
    char buffer[16];
    int i, len = 0;
    obj_global_error_code_t code;
    for (i = 0; i < 1000; i++) {
        len = sprintf(buffer, "key%d", i + index * 1000);
        buffer[len] = '\0';
        code = obj_hash_table_add(table, buffer, "value");
        
        if (code != OBJ_CODE_OK) {
            printf("error, code = %d\n", code);
        }
    }
    return NULL;
}

int main() {
    obj_global_mem_context_init();
    obj_hash_table_t *table;
    obj_hash_table_methods_t methods1 = {
        obj_hash_table_default_hash_func,
        obj_hash_table_default_key_dup,
        obj_hash_table_default_value_dup,
        obj_hash_table_default_key_compare,
        obj_hash_table_default_key_free,
        obj_hash_table_default_value_free
    };
    /*
    obj_hash_table_methods_t methods2 = {
        obj_hash_table
    }
    */
    /* test hash table with lock */
    table = obj_hash_table_create(&methods1, 32, true);
    obj_assert(table != NULL);
    /* test_func(table); */
    pthread_t id[16];
    test_arg_t *arg;
    int i = 0;
    for (i = 0; i < 16; i++) {
        arg = (test_arg_t *)obj_alloc(sizeof(test_arg_t));
        arg->index = i;
        arg->table = table;
        pthread_create(&id[i], NULL, test_func, arg);
    }
    for (i = 0; i < 16; i++) {
        pthread_join(id[i], NULL);
    }
    
    obj_hash_table_stats(table);
    obj_hash_table_destroy(table);
    table  = NULL;
    /* test hash table without lock */
    table = obj_hash_table_create(&methods, 32, false);
    obj_assert(table != NULL);
    char buffer[16];
    int len = 0;
    obj_global_error_code_t code;
    for (i = 0; i < 10000; i++) {
        len = sprintf(buffer, "key%d", i);
        buffer[len] = '\0';
        code = obj_hash_table_add(table, buffer, "value");
        if (code != OBJ_CODE_OK) {
            printf("error, code = %d\n", code);
        }
    }
    obj_hash_table_stats(table);
    obj_hash_table_destroy(table);
    
    return 0;
}