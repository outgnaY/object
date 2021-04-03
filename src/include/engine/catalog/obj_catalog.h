#ifndef OBJ_CATALOG_H
#define OBJ_CATALOG_H

#include "obj_core.h"

typedef struct obj_db_catalog_entry_pair_s obj_db_catalog_entry_pair_t;
typedef struct obj_collection_catalog_entry_pair_s obj_collection_catalog_entry_pair_t;
typedef struct obj_db_catalog_entry_s obj_db_catalog_entry_t;
typedef struct obj_collection_catalog_entry_s obj_collection_catalog_entry_t;
typedef struct obj_index_catalog_entry_s obj_index_catalog_entry_t;


struct obj_db_catalog_entry_pair_s {
    char *db;
    obj_db_catalog_entry_t *entry;
};

struct obj_collection_catalog_entry_pair_s {
    char *collection;
    obj_collection_catalog_entry_t *entry;
};

/* database entry */
struct obj_db_catalog_entry_s {
    obj_prealloc_map_t collections;
};

/* collection entry */
struct obj_collection_catalog_entry_s {
    /* record store */
    obj_record_store_t *record_store;
    /* type checker */
    obj_type_checker_t checker;
    /* [index catalog entry] */
    obj_array_t indexes;
};

/* an index */
struct obj_index_catalog_entry_s {
    /* number of fields, for compound indexes */
    int nfields;
    /* {"a": 1, "b": -1} */
    obj_bson_t *key_pattern;
    /* index */
    obj_skiplist_t *skiplist;
};



extern obj_prealloc_map_methods_t db_catalog_entry_map_methods;
/* database catalog entry methods */
obj_db_catalog_entry_t *obj_db_catalog_entry_create();
void obj_db_catalog_entry_destroy(obj_db_catalog_entry_t *db_entry);

/* collection catalog entry methods */
obj_collection_catalog_entry_t *obj_collection_catalog_entry_create(obj_bson_t *prototype);
void obj_collection_catalog_entry_destroy(obj_collection_catalog_entry_t *collection_entry);
obj_array_t *obj_collection_catalog_entry_get_indexes(obj_collection_catalog_entry_t *collection_entry);

#endif  /* OBJ_CATALOG_H */