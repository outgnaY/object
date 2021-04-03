#include "obj_core.h"

static obj_uint64_t obj_record_iter_set_hash_func(void *key);
static int obj_record_iter_set_key_compare(void *key1, void *key2);
static void obj_record_iter_set_key_free(void *data);
static void *obj_record_iter_set_key_get(void *data);
static void obj_record_iter_set_key_set(void *data, void *key);


static obj_record_t *obj_record_store_get_first_record(obj_record_store_t *record_store);
static obj_record_t *obj_record_store_get_last_record(obj_record_store_t *record_store);
static obj_record_t *obj_record_store_get_next_record(obj_record_t *record);
static obj_record_t *obj_record_store_get_prev_record(obj_record_t *record);
static void obj_record_store_remove_record(obj_record_store_t *record_store, obj_record_t *record);
static void obj_record_store_record_destroy(obj_record_t *record);

static void obj_record_store_iterator_advance(obj_record_store_iterator_t *iter);
static obj_bool_t obj_record_store_iterator_is_eof(obj_record_store_iterator_t *iter);

static void obj_record_store_iterator_set_init(obj_record_store_iterator_set_t *iter_set);
static void obj_record_store_iterator_set_register(obj_record_store_iterator_set_t *iter_set, obj_record_store_iterator_t *iter);
static void obj_record_store_iterator_set_unregister(obj_record_store_iterator_set_t *iter_set, obj_record_store_iterator_t *iter);



/* record itertor set */
static obj_prealloc_map_methods_t record_iter_set_methods = {
    obj_record_iter_set_hash_func,
    obj_record_iter_set_key_compare,
    obj_record_iter_set_key_free,
    NULL,
    obj_record_iter_set_key_get,
    NULL,
    obj_record_iter_set_key_set,
    NULL,
    NULL,
    NULL
};

static obj_uint64_t obj_record_iter_set_hash_func(void *key) {
    obj_record_store_iterator_t *iter = *(obj_record_store_iterator_t **)key;
    return obj_set_hash_function(iter, sizeof(obj_record_store_iterator_t *));
};

static int obj_record_iter_set_key_compare(void *key1, void *key2) {
    obj_record_store_iterator_t *iter1 = *(obj_record_store_iterator_t **)key1;
    obj_record_store_iterator_t *iter2 = *(obj_record_store_iterator_t **)key2;
    if (iter1 - iter2 < 0) {
        return -1;
    } else if (iter1 - iter2 > 0) {
        return 1;
    } else {
        return 0;
    }
}

static void obj_record_iter_set_key_free(void *data) {
    obj_record_store_iterator_t *iter = *(obj_record_store_iterator_t **)data;
    obj_record_store_iterator_destroy(iter);
}

static void *obj_record_iter_set_key_get(void *data) {
    return data;
}

static void obj_record_iter_set_key_set(void *data, void *key) {
    obj_memcpy(data, key, sizeof(obj_record_store_iterator_t *));
}


/* ********** record store methods ********** */

/* create a record store */
obj_record_store_t *obj_record_store_create() {
    obj_record_store_t *record_store = obj_alloc(sizeof(obj_record_store_t));
    obj_record_store_init(record_store);
    return record_store;
}

/* init record store */
void obj_record_store_init(obj_record_store_t *record_store) {
    OBJ_EMBEDDED_LIST_INIT(record_store->record_list);
    /* init iterator set */
    obj_record_store_iterator_set_init(&record_store->iter_set);
}


/* destroy */
void obj_record_store_destroy(obj_record_store_t *record_store) {
    obj_record_t *record = NULL;
    obj_record_t *next_record = NULL;
    record = OBJ_EMBEDDED_LIST_GET_FIRST(record_store->record_list);
    while (record != NULL) {
        next_record = OBJ_EMBEDDED_LIST_GET_NEXT(list, record);
        obj_record_store_remove_record(record_store, record);
        record = next_record;
    }
}

/* number of record */
int obj_record_store_num_records(obj_record_store_t *record_store) {
    return record_store->record_list.count;
}

/* first record */
static inline obj_record_t *obj_record_store_get_first_record(obj_record_store_t *record_store) {
    return OBJ_EMBEDDED_LIST_GET_FIRST(record_store->record_list);
}

/* last record */
static inline obj_record_t *obj_record_store_get_last_record(obj_record_store_t *record_store) {
    return OBJ_EMBEDDED_LIST_GET_LAST(record_store->record_list);
}

/* next record */
static inline obj_record_t *obj_record_store_get_next_record(obj_record_t *record) {
    return OBJ_EMBEDDED_LIST_GET_NEXT(list, record);
}


/* prev record */
static inline obj_record_t *obj_record_store_get_prev_record(obj_record_t *record) {
    return OBJ_EMBEDDED_LIST_GET_PREV(list, record);
}

/* add record to the record store */
void obj_record_store_add_record(obj_record_store_t *record_store, obj_record_t *record) {
    OBJ_EMBEDDED_LIST_ADD_LAST(list, record_store->record_list, record);
}

/* remove record */
static void obj_record_store_remove_record(obj_record_store_t *record_store, obj_record_t *record) {
    OBJ_EMBEDDED_LIST_REMOVE(list, record_store->record_list, record);
    obj_record_store_record_destroy(record);
}

void obj_record_store_add(obj_record_store_t *record_store, obj_bson_t *bson) {
    obj_record_t *record = obj_alloc(sizeof(obj_record_t));
    record->bson = bson;
    obj_record_store_add_record(record_store, record);
}


static inline void obj_record_store_record_destroy(obj_record_t *record) {
    obj_bson_destroy(record->bson);
    obj_free(record);
}

/* for debug */
int obj_record_store_iterator_num(obj_record_store_t *record_store) {
    return record_store->iter_set.iters.map.size;
}



/* ********** record store iterator methods ********** */

/* create an iterator */
obj_record_store_iterator_t *obj_record_store_iterator_create(obj_record_store_t *record_store){
    obj_record_store_iterator_t *iter = obj_alloc(sizeof(obj_record_store_iterator_t));
    obj_record_store_iterator_init(iter, record_store);
    return iter;
}

/* init an iterator */
void obj_record_store_iterator_init(obj_record_store_iterator_t *iter, obj_record_store_t *record_store) {
    iter->record_store = record_store;
    iter->current = obj_record_store_get_first_record(record_store);
}

void obj_record_store_iterator_destroy(obj_record_store_iterator_t *iter) {
    obj_free(iter);
}

/* get record and move to next */
inline obj_record_t *obj_record_store_iterator_next(obj_record_store_iterator_t *iter) {
    if (obj_record_store_iterator_is_eof(iter)) {
        return NULL;
    }
    obj_record_t *current = iter->current;
    obj_record_store_iterator_advance(iter);
    return current;
}

/* move to next */
static inline void obj_record_store_iterator_advance(obj_record_store_iterator_t *iter) {
    if (!obj_record_store_iterator_is_eof(iter)) {
        iter->current = obj_record_store_get_next_record(iter->current);
    }
}

/* is eof */
static inline obj_bool_t obj_record_store_iterator_is_eof(obj_record_store_iterator_t *iter) {
    return iter->current == NULL;
}


/* ********** record store iterator set methods ********** */

/* init iterator set */
static void obj_record_store_iterator_set_init(obj_record_store_iterator_set_t *iter_set) {
    obj_set_init(&iter_set->iters, &record_iter_set_methods, sizeof(obj_record_store_iterator_t *));
    pthread_mutex_init(&iter_set->mutex, NULL);
}

static void obj_record_store_iterator_set_register(obj_record_store_iterator_set_t *iter_set, obj_record_store_iterator_t *iter) {
    obj_set_add(&iter_set->iters, &iter);
}

static void obj_record_store_iterator_set_unregister(obj_record_store_iterator_set_t *iter_set, obj_record_store_iterator_t *iter) {
    obj_set_delete(&iter_set->iters, &iter);
}