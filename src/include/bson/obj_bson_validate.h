#ifndef OBJ_BSON_VALIDATE_H
#define OBJ_BSON_VALIDATE_H

#include "obj_core.h"

/* bson max allowable depth */
#define OBJ_BSON_VALIDATE_MAX_DEPTH 10

typedef enum obj_bson_validate_state obj_bson_validate_state_t;
typedef struct obj_bson_validate_s obj_bson_validate_t;
typedef struct obj_bson_validate_frame_s obj_bson_validate_frame_t;


/* bson validate state */
enum obj_bson_validate_state {
    OBJ_BSON_VALIDATE_BEGIN_OBJ,
    OBJ_BSON_VALIDATE_WITHIN_OBJ,
    OBJ_BSON_VALIDATE_END_OBJ,
    OBJ_BSON_VALIDATE_DONE
};


struct obj_bson_validate_s {
    obj_uint8_t *data;
    int position;
    int len;
};

struct obj_bson_validate_frame_s {
    int start_position;
    obj_int32_t expected_size;
};

/* validate a bson */
obj_global_error_code_t obj_bson_validate(obj_uint8_t *data, int len);


#endif  /* OBJ_BSON_VALIDATE_H */