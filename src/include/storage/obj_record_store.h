#ifndef OBJ_RECORD_STORE_H
#define OBJ_RECORD_STORE_H

#include "obj_core.h"

typedef struct obj_record_store_s obj_record_store_t;
typedef struct obj_record_store_methods_s obj_record_store_methods_t;
typedef struct obj_record_s obj_record_t;

/* record store, base type */
struct obj_record_store_s {
    obj_record_store_methods_t *methods;
};

/* record store methods */
struct obj_record_store_methods_s {
    int (*num_records)(obj_record_store_t *record_store);
};

struct obj_record_s {

};


#endif  /* OBJ_RECORD_STORE_H */