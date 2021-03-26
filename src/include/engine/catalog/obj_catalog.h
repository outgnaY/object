#ifndef OBJ_CATALOG_H
#define OBJ_CATALOG_H

#include "obj_core.h"

typedef struct obj_db_catalog_entry_pair_s obj_db_catalog_entry_pair_t;
typedef struct obj_collection_catalog_entry_pair_s obj_collection_catalog_entry_pair_t;
typedef struct obj_db_catalog_entry_s obj_db_catalog_entry_t;
typedef struct obj_collection_catalog_entry_s obj_collection_catalog_entry_t;
typedef struct obj_index_catalog_entry_s obj_index_catalog_entry_t;
typedef struct obj_index_catalog_s obj_index_catalog_t;


struct obj_db_catalog_entry_pair_s {
    char *db;
    obj_db_catalog_entry_t *entry;
};

struct obj_collection_catalog_entry_pair_s {
    char *collection;
    obj_db_catalog_entry_t *entry;
};

/* database entry */
struct obj_db_catalog_entry_s {
    obj_prealloc_map_t collections;
};

/* collection entry */
struct obj_collection_catalog_entry_s {
    /* record store */
    obj_record_store_t *record_store;
    /* object prototype */
    /* obj_bson_t *prototype; */
};

/* an index */
struct obj_index_catalog_entry_s {

};

/* indexes */
struct obj_index_catalog_s {

};


extern obj_prealloc_map_methods_t db_catalog_entry_map_methods;
/* database catalog entry methods */
obj_db_catalog_entry_t *obj_db_catalog_entry_create();
void obj_db_catalog_entry_destroy(obj_db_catalog_entry_t *db_entry);

/* collection catalog entry methods */
obj_collection_catalog_entry_t *obj_collection_catalog_entry_create();
void obj_colleciton_catalog_entry_destroy(obj_collection_catalog_entry_t *collection_entry);

#endif  /* OBJ_CATALOG_H */