#ifndef OBJ_V1_RECORD_STORE
#define OBJ_V1_RECORD_STORE

#include "obj_core.h"

/* data structure for records store */

#define OBJ_V1_RECORD_STORE_MAX_SIZE 0x7fffffff

typedef struct obj_v1_record_s obj_v1_record_t;
typedef struct obj_v1_record_store_s obj_v1_record_store_t;

/* a record */
struct obj_v1_record_s {
    OBJ_EMBEDDED_LIST_NODE_T(obj_v1_record_t) list;
    /* data */
    obj_bson_t *bson;
};

struct obj_v1_record_store_s {
    /* base type */
    obj_record_store_t base;
    OBJ_EMBEDDED_LIST_BASE_NODE_T(obj_v1_record_t) record_list;
    int num_records;
};

obj_v1_record_store_t *obj_v1_record_store_create();
void obj_v1_record_store_init(obj_v1_record_store_t *record_store);
void obj_v1_record_store_destroy(obj_v1_record_store_t *record_store);
obj_v1_record_t *obj_v1_record_store_record_create(obj_bson_t *bson);
void obj_v1_record_store_record_destroy(obj_v1_record_t *record);
int obj_v1_record_store_num_records(obj_record_store_t *record_store);
obj_v1_record_t *obj_v1_record_store_get_next_record(obj_v1_record_t *record);
obj_v1_record_t *obj_v1_record_store_get_prev_record(obj_v1_record_t *record);
void obj_v1_record_store_add_record_last(obj_v1_record_store_t *record_store, obj_v1_record_t *record);
void obj_v1_record_store_remove_record(obj_v1_record_store_t *record_store, obj_v1_record_t *record);

#endif  /* OBJ_V1_RECORD_STORE */