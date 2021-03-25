#ifndef OBJ_RECORD_STORE_H
#define OBJ_RECORD_STORE_H

#include "obj_core.h"

/* data structures for records store */

#define OBJ_RECORD_STORE_MAX_SIZE 0x7fffffff

typedef struct obj_record_s obj_record_t;
typedef struct obj_record_store_iterator_set_s obj_record_store_iterator_set_t;
typedef struct obj_record_store_s obj_record_store_t;
typedef struct obj_record_store_iterator_s obj_record_store_iterator_t;

/* record */
struct obj_record_s {
    OBJ_EMBEDDED_LIST_NODE_T(obj_record_t) list;
    /* data */
    obj_bson_t *bson;
};

/* store current iterators */
struct obj_record_store_iterator_set_s {
    obj_set_t iters;
    pthread_mutex_t mutex;
};

/* record store */
struct obj_record_store_s {
    /* records */
    OBJ_EMBEDDED_LIST_BASE_NODE_T(obj_record_t) record_list;
    obj_record_store_iterator_set_t iter_set;
};


/* iterate records */
struct obj_record_store_iterator_s {
    /* record_store */
    obj_record_store_t *record_store;
    /* 1/-1 */
    int direction;
    obj_record_t *current;
};


/* record store methods */
obj_record_store_t *obj_record_store_create();
void obj_record_store_init(obj_record_store_t *record_store);
void obj_record_store_destroy(obj_record_store_t *record_store);
int obj_record_store_num_records(obj_record_store_t *record_store);
void obj_record_store_add(obj_record_store_t *record_store, obj_bson_t *bson);
int obj_record_store_iterator_num(obj_record_store_t *record_store);

/* record store iterator methods */
obj_record_store_iterator_t *obj_record_store_iterator_create(obj_record_store_t *record_store, int direction);
void obj_record_store_iterator_init(obj_record_store_iterator_t *iter, obj_record_store_t *record_store, int direction);
void obj_record_store_iterator_destroy(obj_record_store_iterator_t *iter);
obj_record_t *obj_record_store_iterator_next(obj_record_store_iterator_t *iter);



#endif  /* OBJ_RECORD_STORE_H */