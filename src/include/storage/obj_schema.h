#ifndef OBJ_SCHEMA_H
#define OBJ_SCHEMA_H

#include "obj_core.h"

typedef enum obj_schema_type obj_schema_type_t;
typedef struct obj_schema_s obj_schema_t;
typedef struct obj_schema_def_s obj_schema_def_t;
typedef struct obj_schema_value_header_s obj_schema_value_header_t;
typedef struct obj_schema_value_s obj_schema_value_t;

/* types */
enum obj_schema_type {
    OBJ_SCHEMA_TYPE_DOUBLE = 0x01,              /* double */
    OBJ_SCHEMA_TYPE_UTF8 = 0x02,                /* utf-8 string */
    OBJ_SCHEMA_TYPE_OBJECT = 0x03,              /* object */
    OBJ_SCHEMA_TYPE_ARRAY = 0x04,               /* array */
    OBJ_SCHEMA_TYPE_BINARY = 0x05,              /* binary */
    OBJ_SCHEMA_TYPE_BOOL = 0x06,                /* bool */
    OBJ_SCHEMA_TYPE_INT32 = 0x07,               /* int32 */
    OBJ_SCHEMA_TYPE_INT64 = 0x08                /* int64 */
};



/* schema defined for objects */
struct obj_schema_s {                                      
    obj_hash_table_t *schema_table;                /* schema name -- schema definition */
};

/* schema definition */
struct obj_schema_def_s {
    obj_hash_table_t *def_table;                    /* schema key -- schema value */                                        
};

struct obj_schema_value_header_s {
    obj_schema_type_t type;                         /* type of the value */
};

struct obj_schema_value_s {
    obj_schema_value_header_t header;               /* header */
};

/* create a schema */

/* delete a schema */

/* update a schema */

/* find a schema */
/* obj_schema_def_t *obj_schema_find_schema(); */

#endif  /* OBJ_SCHEMA_H */