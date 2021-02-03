#ifndef OBJ_STATUS_WITH_H
#define OBJ_STATUS_WITH_H

#include "obj_core.h"

typedef struct obj_status_with_s obj_status_with_t;

/* wrap data and error */
struct obj_status_with_s {
    void *data;
    const char *message;
    obj_global_error_code_t code;
};

inline obj_bool_t obj_status_isok(obj_status_with_t *status);
inline obj_status_with_t obj_status_create(void *data, const char *message, obj_global_error_code_t code);

#endif  /* OBJ_STATUS_WITH_H */