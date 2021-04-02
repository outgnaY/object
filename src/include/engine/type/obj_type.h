#ifndef OBJ_TYPE_H
#define OBJ_TYPE_H

#include "obj_core.h"

/* object type related */

typedef struct obj_type_checker_s obj_type_checker_t;

/* check type */
struct obj_type_checker_s {
    /* type define */
    obj_bson_t *prototype;
};

obj_bool_t obj_type_checker_check_type(obj_type_checker_t *checker, obj_bson_t *data);
obj_bool_t obj_check_type_define(obj_bson_t *prototype);

#endif  /* OBJ_TYPE_H */