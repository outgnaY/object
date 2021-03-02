#include "obj_core.h"

void *fn1(void *arg) {
    obj_locker_t *locker = (obj_locker_t *)arg;
    obj_global_lock_t global_lock1;
    obj_global_lock_init(&global_lock1, locker, OBJ_LOCK_MODE_IS);
    obj_db_lock_t db_lock1;
    obj_db_lock_init(&db_lock1, locker, OBJ_LOCK_MODE_IS, "db1", 3);
    obj_lock_result_t result;
    result = obj_global_lock_lock(&global_lock1);
    printf("th1 global_lock1 result = %d\n", result);
    result = obj_db_lock_lock(&db_lock1);
    printf("th1 db_lock1 result = %d\n", result);
    result = obj_global_lock_unlock(&db_lock1);
    result = obj_global_lock_unlock(&global_lock1);
}

void *fn2(void *arg) {
    obj_locker_t *locker = (obj_locker_t *)arg;
    obj_global_lock_t global_lock2;
    obj_global_lock_init(&global_lock2, locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_t db_lock2;
    obj_db_lock_init(&db_lock2, locker, OBJ_LOCK_MODE_X, "db1", 3);
    obj_lock_result_t result;
    result = obj_global_lock_lock(&global_lock2);
    printf("th2 global_lock2 result = %d\n", result);
    result = obj_db_lock_lock(&db_lock2);
    printf("th2 db_lock2 result = %d\n", result);
    result = obj_global_lock_unlock(&db_lock2);
    result = obj_global_lock_unlock(&global_lock2);
}

int main() {
    obj_global_mem_context_init();
    obj_global_lock_manager_init();
    obj_locker_t *locker1 = obj_locker_create();
    obj_locker_t *locker2 = obj_locker_create();
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, fn1, locker1);
    pthread_create(&tid2, NULL, fn2, locker2);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return 0;
}