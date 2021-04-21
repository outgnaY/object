#ifndef OBJ_STATUS_H
#define OBJ_STATUS_H

#include "obj_core.h"

typedef struct obj_status_s obj_status_t;
typedef struct obj_status_with_s obj_status_with_t;

struct obj_status_s {
    const char *message;
    obj_global_error_code_t code;
};

/* wrap data and error */
struct obj_status_with_s {
    const char *message;
    obj_global_error_code_t code;
    void *data;
};


obj_bool_t obj_status_with_is_ok(obj_status_with_t *status);
obj_bool_t obj_status_is_ok(obj_status_t *status);
obj_status_t obj_status_create(const char *message, obj_global_error_code_t code);
obj_status_with_t obj_status_with_create(void *data, const char *message, obj_global_error_code_t code);

#endif  /* OBJ_STATUS_H */