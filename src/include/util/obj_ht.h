#ifndef OBJ_HT_H
#define OBJ_HT_H

#include "obj_core.h"

typedef struct obj_ht_s obj_ht_t;

#define OBJ_HT_ENTRY(K, V)\
typedef struct {\
    K key;\
    V value;\
    obj_ht_entry_t *next;\
}obj_ht_entry_t \

/* hashtable without lock support */
struct obj_ht_s {
    unsigned long n_buckets;
    obj_ht_entry_t **table;
};


#endif  /* OBJ_HT_H */