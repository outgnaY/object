#include "obj_core.h"

static obj_record_store_methods_t methods = {
    obj_V1_record_store_record_num_records
};

/* create a record store */
obj_v1_record_store_t *obj_v1_record_store_create() {
    obj_v1_record_store_t *record_store = NULL;
    record_store = obj_alloc(sizeof(obj_v1_record_store_t));
    if (record_store == NULL) {
        return NULL;
    }
    obj_v1_record_store_init(record_store);
    return record_store;
}

void obj_v1_record_store_init(obj_v1_record_store_t *record_store) {
    OBJ_EMBEDDED_LIST_INIT(record_store->record_list);
    record_store->num_records = 0;
}

/* destroy a record store */
void obj_v1_record_store_destroy(obj_v1_record_store_t *record_store) {
    obj_v1_record_t *record = NULL;
    obj_v1_record_t *next_record = NULL;
    record = OBJ_EMBEDDED_LIST_GET_FIRST(record_store->record_list);
    while (record != NULL) {
        next_record = OBJ_EMBEDDED_LIST_GET_NEXT(list, record);
        obj_v1_record_store_remove_record(record_store, record);
        record = next_record;
    }
}

obj_v1_record_t *obj_v1_record_store_record_create(obj_bson_t *bson) {
    obj_v1_record_t *record = obj_alloc(sizeof(obj_v1_record_t));
    if (record == NULL) {
        return NULL;
    }
    record->bson = bson;
    return record;
}

void obj_v1_record_store_record_destroy(obj_v1_record_t *record) {
    obj_bson_destroy(record->bson);
    obj_free(record);
}

/* number of records */
int obj_V1_record_store_record_num_records(obj_record_store_t *record_store) {
    obj_v1_record_store_t *v1_record_store = (obj_v1_record_store_t *)record_store;
    return v1_record_store->num_records;
}

inline obj_v1_record_t *obj_v1_record_store_get_next_record(obj_v1_record_t *record) {
    return OBJ_EMBEDDED_LIST_GET_NEXT(list, record);
}

inline obj_v1_record_t *obj_v1_record_store_get_prev_record(obj_v1_record_t *record) {
    return OBJ_EMBEDDED_LIST_GET_PREV(list, record);
}

/* add record to the record store */
void obj_v1_record_store_add_record_last(obj_v1_record_store_t *record_store, obj_v1_record_t *record) {
    OBJ_EMBEDDED_LIST_ADD_LAST(list, record_store->record_list, record);
    record_store->num_records++;
}

/* remove record from the record store */
void obj_v1_record_store_remove_record(obj_v1_record_store_t *record_store, obj_v1_record_t *record) {
    OBJ_EMBEDDED_LIST_REMOVE(list, record_store->record_list, record);
    obj_v1_record_store_record_destroy(record);
    record_store->num_records--;
}

