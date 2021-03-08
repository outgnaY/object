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
    obj_status_t (*create_collection)(obj_stringdata_t *ns);
    obj_status_t (*drop_collection)(obj_stringdata_t *ns);
};

/* collection catalog entry */
struct obj_collection_catalog_entry_s {

};


#endif  /* OBJ_CATALOG_H */