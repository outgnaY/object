#ifndef OBJ_V1_CATALOG_H
#define OBJ_V1_CATALOG_H

#include "obj_core.h"

typedef struct obj_db_catalog_pair_s obj_db_catalog_pair_t;
typedef struct obj_collection_catalog_pair_s obj_collection_catalog_pair_t;
typedef struct obj_v1_db_catalog_entry_s obj_v1_db_catalog_entry_t;
typedef struct obj_v1_collection_catalog_entry_s obj_v1_collection_catalog_entry_t;

struct obj_db_catalog_pair_s {
    obj_stringdata_t db;
    obj_v1_db_catalog_entry_t *entry;
};

struct obj_collection_catalog_pair_s {
    obj_stringdata_t collection;
    obj_v1_collection_catalog_entry_t *entry;
};

/* database catalog entry */
struct obj_v1_db_catalog_entry_s {
    obj_db_catalog_entry_t base;
    /* collection catalog entries */
    obj_prealloc_map_t collections;
};

/* collection catalog entry */
struct obj_v1_collection_catalog_entry_s {
    obj_collection_catalog_entry_t base;
    obj_v1_db_catalog_entry_t *db_entry;
    obj_v1_record_store_t *record_store;
};

extern obj_prealloc_map_methods_t db_catalog_entry_map_methods;

#endif  /* OBJ_V1_CATALOG_H */