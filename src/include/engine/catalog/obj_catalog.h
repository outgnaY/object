#ifndef OBJ_CATALOG_H
#define OBJ_CATALOG_H

#include "obj_core.h"

/* forward declaration */
typedef struct obj_engine_s obj_engine_t;

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
    char *name;
    /* debug */
    int index;
};


#define OBJ_INDEX_NUM_MAX 64

extern obj_prealloc_map_methods_t db_catalog_entry_map_methods;
/* database catalog entry methods */
obj_db_catalog_entry_t *obj_db_catalog_entry_get(obj_engine_t *engine, char *db_name);
void obj_delete_db(obj_engine_t *engine, char *db_name);
void obj_create_db(obj_engine_t *engine, char *db_name);

/* collection catalog entry methods */
obj_collection_catalog_entry_t *obj_collection_catalog_entry_get(obj_db_catalog_entry_t *db_entry, char *collection_name);
obj_bool_t obj_delete_collection(obj_db_catalog_entry_t *db_entry, char *collection_name);
obj_bool_t obj_create_collection(obj_db_catalog_entry_t *db_entry, char *collection_name, obj_bson_t *prototype);
void obj_insert_objects(obj_collection_catalog_entry_t *collection_entry, obj_bson_t *objects);

/* index methods */
obj_array_t *obj_collection_catalog_entry_get_indexes(obj_collection_catalog_entry_t *collection_entry);
obj_status_t obj_create_index(obj_collection_catalog_entry_t *collection_entry, obj_bson_t *key_pattern, char *index_name);
obj_bool_t obj_delete_index(obj_collection_catalog_entry_t *collection_entry, char *index_name);


#endif  /* OBJ_CATALOG_H */