#ifndef OBJ_CATALOG_H
#define OBJ_CATALOG_H

#include "obj_core.h"

typedef struct obj_db_catalog_entry_s obj_db_catalog_entry_t;
typedef struct obj_db_catalog_entry_methods_s obj_db_catalog_entry_methods_t;
typedef struct obj_collection_catalog_entry_s obj_collection_catalog_entry_t;

/* database catalog entry */
struct obj_db_catalog_entry_s {
    obj_db_catalog_entry_methods_t *methods;
};

struct obj_db_catalog_entry_methods_s {
    obj_collection_catalog_entry_t *(*get_collection)(obj_stringdata_t *ns);
    obj_collection_catalog_entry_t *(*create_collection_if_not_exists)(obj_stringdata_t *ns);
    obj_record_store_t *(*get_record_store)(obj_stringdata_t *ns);
    obj_status_t (*drop_collection)(obj_stringdata_t *ns);
    void (*get_collections)(obj_array_t *array);
};

/* collection catalog entry */
struct obj_collection_catalog_entry_s {

};


#endif  /* OBJ_CATALOG_H */