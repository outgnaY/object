#include "obj_core.h"

typedef struct locker_with_deadline_s locker_with_deadline_t;

struct locker_with_deadline_s {
    obj_locker_t *locker;
    struct timeval *tv;
};

void *fn1(void *arg) {
    obj_locker_t *locker = (obj_locker_t *)arg;
    obj_global_lock_t global_lock1;
    obj_db_lock_t db_lock1;
    obj_collection_lock_t collection_lock1;
    obj_lock_result_t result;
    char *db = "db1";
    char *collection = "db1.coll1";
    
    obj_global_lock_init(&global_lock1, locker, OBJ_LOCK_MODE_IS);
    obj_db_lock_init(&db_lock1, locker, OBJ_LOCK_MODE_IS, db);
    obj_collection_lock_init(&collection_lock1, locker, OBJ_LOCK_MODE_X, collection);
    
    result = obj_global_lock_lock(&global_lock1);
    printf("th1 global_lock1 lock result = %d\n", result);
    result = obj_db_lock_lock(&db_lock1);
    printf("th1 db_lock1 lock result = %d\n", result);
    result = obj_collection_lock_lock(&collection_lock1);
    printf("th1 collection_lock1 lock result = %d\n", result);
    
    result = obj_collection_lock_unlock(&collection_lock1);
    printf("th1 collection_lock1 unlock result = %d\n", result);
    result = obj_db_lock_unlock(&db_lock1);
    printf("th1 db_lock1 unlock result = %d\n", result);
    result = obj_global_lock_unlock(&global_lock1);
    printf("th1 global_lock1 unlock result = %d\n", result);
    
    return NULL;
}


void *fn2(void *arg) {
    obj_locker_t *locker = (obj_locker_t *)arg;
    obj_global_lock_t global_lock2;
    obj_db_lock_t db_lock2;
    obj_collection_lock_t collection_lock2;
    obj_lock_result_t result;
    char *db = "db1";
    char *collection = "db1.coll2";

    obj_global_lock_init(&global_lock2, locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init(&db_lock2, locker, OBJ_LOCK_MODE_IX, db);
    obj_collection_lock_init(&collection_lock2, locker, OBJ_LOCK_MODE_S, collection);

    result = obj_global_lock_lock(&global_lock2);
    printf("th2 global_lock2 lock result = %d\n", result);
    result = obj_db_lock_lock(&db_lock2);
    printf("th2 db_lock2 lock result = %d\n", result);
    result = obj_collection_lock_lock(&collection_lock2);
    printf("th2 collection_lock2 lock result = %d\n", result);
    
    result = obj_collection_lock_unlock(&collection_lock2);
    printf("th2 collection_lock2 unlock result = %d\n", result);
    result = obj_db_lock_unlock(&db_lock2);
    printf("th2 db_lock2 unlock result = %d\n", result);
    result = obj_global_lock_unlock(&global_lock2);
    printf("th2 global_lock2 unlock result = %d\n", result);
    
    return NULL;
}

void *fn1_time(void *arg) {  
    obj_locker_t *locker = ((locker_with_deadline_t  *)arg)->locker;
    struct timeval *tv = ((locker_with_deadline_t *)arg)->tv;
    obj_global_lock_t global_lock1;
    obj_db_lock_t db_lock1;
    obj_lock_result_t result;
    char *db = "db1";
    obj_global_lock_init(&global_lock1, locker, OBJ_LOCK_MODE_IS);
    obj_db_lock_init(&db_lock1, locker, OBJ_LOCK_MODE_IX, db);

    result = obj_global_lock_lock(&global_lock1);
    printf("th1 global_lock1 lock result = %d\n", result);
    result = obj_db_lock_lock(&db_lock1);
    printf("th1 db_lock1 lock result = %d\n", result);
    /*
    result = obj_db_lock_unlock(&db_lock1);
    printf("th1 db_lock1 unlock result = %d\n", result);
    result = obj_global_lock_unlock(&global_lock1);
    printf("th1 global_lock1 unlock result = %d\n", result);
    */
    obj_locker_clean_requests(locker);
    obj_locker_destroy(locker);
    return NULL;
}

void *fn2_time(void *arg) {
    obj_locker_t *locker = ((locker_with_deadline_t  *)arg)->locker;
    struct timeval *tv = ((locker_with_deadline_t *)arg)->tv;
    obj_global_lock_t global_lock2;
    obj_db_lock_t db_lock2;
    obj_lock_result_t result;
    char *db = "db1";
    obj_abs_time_msecond deadline = tv->tv_sec * 1000 + tv->tv_usec / 1000;

    obj_global_lock_init(&global_lock2, locker, OBJ_LOCK_MODE_IX);
    obj_db_lock_init_with_deadline(&db_lock2, locker, OBJ_LOCK_MODE_X, db, deadline);

    result = obj_global_lock_lock(&global_lock2);
    printf("th2 global_lock2 lock result = %d\n", result);
    result = obj_db_lock_lock(&db_lock2);
    printf("th2 db_lock2 lock result = %d\n", result);

    result = obj_db_lock_unlock(&db_lock2);
    printf("th2 db_lock2 unlock result = %d\n", result);
    result = obj_global_lock_unlock(&global_lock2);
    printf("th2 global_lock2 unlock result = %d\n", result);
    return NULL;
}



int main() {
    obj_global_mem_context_init();
    obj_global_lock_manager_init();
    obj_locker_t *locker1 = obj_locker_create();
    obj_locker_t *locker2 = obj_locker_create();
    struct timeval tv, tv1, tv2;
    locker_with_deadline_t arg1, arg2;
    arg1.tv = &tv1;
    arg1.locker = locker1;
    arg2.tv = &tv2;
    arg2.locker = locker2;
    pthread_t tid1, tid2;
    gettimeofday(&tv, NULL);
    tv1.tv_sec = tv.tv_sec + 5;
    tv1.tv_usec = 0;
    tv2.tv_sec = tv.tv_sec + 5;
    tv2.tv_usec = 0;
    /*
    pthread_create(&tid1, NULL, fn1, locker1);
    pthread_create(&tid2, NULL, fn2, locker2);
    */
    pthread_create(&tid1, NULL, fn1_time, &arg1);
    pthread_create(&tid2, NULL, fn2_time, &arg2);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    obj_lock_dump_lock_manager(g_lock_manager);
    obj_lock_cleanup_unused_locks(g_lock_manager);
    obj_global_lock_manager_destroy();
    return 0;
}