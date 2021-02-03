#include "obj_core.h"

/* status is ok */
inline obj_bool_t obj_status_isok(obj_status_with_t *status) {
    return status->code == 0;
}

/* create status struct */
inline obj_status_with_t obj_status_create(void *data, const char *message, obj_global_error_code_t code) {
    obj_status_with_t status = {data, message, code};
    return status;
}

