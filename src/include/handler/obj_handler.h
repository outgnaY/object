#ifndef OBJ_HANDLER_H
#define OBJ_HANDLER_H

#include "obj_core.h"

typedef struct obj_db_manager_s obj_db_manager_t;
typedef struct obj_db_pair_s obj_db_pair_t;
typedef struct obj_collection_pair_s obj_collection_pair_t;
typedef struct obj_db_handler_s obj_db_handler_t;
typedef struct obj_collection_handler_s obj_collection_handler_t;
typedef struct obj_db_catalog_entry_s obj_db_catalog_entry_t;
typedef struct obj_collection_catalog_s obj_collection_catalog_t;
typedef struct obj_index_descriptor_s obj_index_descriptor_t;
typedef struct obj_index_catalog_entry_s obj_index_catalog_entry_t;
typedef struct obj_index_catalog_s obj_index_catalog_t;

/* manage databases */
struct obj_db_manager_s {
    obj_prealloc_map_t dbs;
    pthread_mutex_t mutex;
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

struct obj_index_descriptor_s {

};

struct obj_index_catalog_entry_s {

};

struct obj_index_catalog_s {
    /* index catalog entry container */
    obj_array_t entries;
};

/* global database manager */
extern obj_db_manager_t *g_db_manager;

void obj_global_db_manager_init();
void obj_global_db_manager_destroy();

void obj_db_manager_dump(obj_db_manager_t *db_manager);
obj_status_with_t obj_db_manager_open_db(obj_conn_context_t *context, obj_db_manager_t *db_manager, obj_stringdata_t *db_name);
obj_status_with_t obj_db_manager_open_db_create_if_not_exists(obj_conn_context_t *context, obj_db_manager_t *db_manager, obj_stringdata_t *db_name, obj_bool_t *create);
obj_status_t obj_db_manager_close_db(obj_conn_context_t *context, obj_db_manager_t *db_manager, obj_stringdata_t *db_name);
obj_status_t obj_db_manager_close_all_dbs(obj_conn_context_t *context, obj_db_manager_t *db_manager);
obj_status_t obj_db_manager_drop_db(obj_conn_context_t *context, obj_db_manager_t *db_manager, obj_stringdata_t *db_name);

void obj_db_handler_dump(obj_db_handler_t *db_handler);
obj_status_with_t obj_db_handler_create_collection_if_not_exists(obj_db_handler_t *db_handler, obj_stringdata_t *full_name, obj_bool_t *create);
obj_status_t obj_db_handler_drop_collection(obj_db_handler_t *db_handler, obj_stringdata_t *full_name);

void obj_collection_handler_dump(obj_collection_handler_t *collection_handler);
/*
void obj_collection_handler_insert_object(obj_collection_handler_t *collection_handler);
void obj_collection_handler_insert_objects(obj_collection_handler_t *collection_handler);
void obj_collection_handler_delete_object(obj_collection_handler_t *collection_handler);
void obj_collection_handler_update_object(obj_collection_handler_t *collection_handler);
int obj_collection_handler_num_records(obj_collection_handler_t *collection_handler);
*/

#endif  /* OBJ_HANDLER_H */