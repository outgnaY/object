#ifndef OBJ_V1_INDEX_KEY_H
#define OBJ_V1_INDEX_KEY_H

#include "obj_core.h"

typedef struct obj_v1_index_key_s obj_v1_index_key_t;


/* key of index */
struct obj_v1_index_key_s {
    /* index key data */
    obj_uint8_t *data;
};


/* index key */
obj_v1_index_key_t *obj_v1_index_key_create(obj_bson_t *bson);
void obj_v1_index_key_destroy(obj_v1_index_key_t *index_key);
int obj_v1_index_key_compare(obj_v1_index_key_t *index_key1, obj_v1_index_key_t *index_key2, obj_index_key_order_t key_order);
void obj_v1_index_key_dump(obj_v1_index_key_t *index_key);

#endif  /* OBJ_V1_INDEX_KEY_H */