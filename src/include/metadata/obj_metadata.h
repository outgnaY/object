#ifndef OBJ_METADATA_H
#define OBJ_METADATA_H

#include "obj_core.h"

typedef struct obj_db_manager_s obj_db_manager_t;
typedef struct obj_db_pair_s obj_db_pair_t;
typedef struct obj_collection_pair_s obj_collection_pair_t;
typedef struct obj_db_handler_s obj_db_handler_t;
typedef struct obj_collection_handler_s obj_collection_handler_t;
typedef struct obj_db_catalog_entry_s obj_db_catalog_entry_t;
typedef struct obj_collection_catalog_s obj_collection_catalog_t;

/* manage databases */
struct obj_db_manager_s {
    obj_prealloc_map_t dbs;
};

struct obj_db_pair_s {
    obj_stringdata_t db;
    obj_db_handler_t *db_handler;
};

struct obj_collection_pair_s {
    obj_stringdata_t collection;
    obj_collection_handler_t *collection_handler;
};

/* database handler */
struct obj_db_handler_s {
    obj_stringdata_t name;
    obj_prealloc_map_t collections;
    obj_db_catalog_entry_t *db_entry;
    
};

/* collection handler */
struct obj_collection_handler_s {
    /* namespace string */
    obj_namespace_string_t nss;
    obj_collection_catalog_entry_t *collection_entry;
    obj_db_catalog_entry_t *db_entry;
    obj_record_store_t *record_store;
    /* index catalog */

};






/* global database manager */
extern obj_db_manager_t *g_db_manager;

#endif  /* OBJ_METADATA_H */