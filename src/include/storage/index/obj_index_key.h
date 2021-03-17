#ifndef OBJ_INDEX_KEY_H
#define OBJ_INDEX_KEY_H

#include "obj_core.h"

typedef enum obj_index_key_type obj_index_key_type_t;
typedef unsigned obj_index_key_order_t;


enum obj_index_key_type {
    OBJ_INDEX_KEY_TYPE_DOUBLE = 0x01,
    OBJ_INDEX_KEY_TYPE_UTF8 = 0x02,
    OBJ_INDEX_KEY_TYPE_BINARY = 0x03,
    OBJ_INDEX_KEY_TYPE_INT32 = 0x04,
    OBJ_INDEX_KEY_TYPE_INT64 = 0x05,
    OBJ_INDEX_KEY_TYPE_MASK = 0x0f,
    OBJ_INDEX_KEY_TYPE_HASMORE = 0x40
};


/* index key order */
int obj_index_key_order_get(obj_index_key_order_t key_order, int i);
unsigned obj_index_key_order_descending(obj_index_key_order_t key_order, unsigned mask);



#endif  /* OBJ_INDEX_KEY_H */