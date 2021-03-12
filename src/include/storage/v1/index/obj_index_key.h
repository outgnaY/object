#ifndef OBJ_INDEX_KEY_H
#define OBJ_INDEX_KEY_H

#include "obj_core.h"

typedef enum obj_index_key_type obj_index_key_type_t;
typedef unsigned obj_index_key_order_t;
typedef struct obj_index_key_s obj_index_key_t;

enum obj_index_key_type {
    OBJ_INDEX_KEY_TYPE_DOUBLE = 0x01,
    OBJ_INDEX_KEY_TYPE_UTF8 = 0x02,
    OBJ_INDEX_KEY_TYPE_BINARY = 0x03,
    OBJ_INDEX_KEY_TYPE_INT32 = 0x04,
    OBJ_INDEX_KEY_TYPE_INT64 = 0x05,
    OBJ_INDEX_KEY_TYPE_MASK = 0x0f,
    OBJ_INDEX_KEY_TYPE_HASMORE = 0x40
};

/* key of index */
struct obj_index_key_s {
    /* index key data */
    obj_uint8_t *data;
};

/* index key order */
int obj_index_key_order_get(obj_index_key_order_t key_order, int i);
unsigned obj_index_key_order_descending(obj_index_key_order_t key_order, unsigned mask);
/* index key */
obj_index_key_t *obj_index_key_create(obj_bson_t *bson);
void obj_index_key_destroy(obj_index_key_t *index_key);
int obj_index_key_compare(obj_index_key_t *index_key1, obj_index_key_t *index_key2, obj_index_key_order_t key_order);
void obj_index_key_dump(obj_index_key_t *index_key);

#endif  /* OBJ_INDEX_KEY_H */