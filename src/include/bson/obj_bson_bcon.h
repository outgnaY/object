#ifndef OBJ_BSON_BCON_H
#define OBJ_BSON_BCON_H

#include "obj_core.h"

/* bcon -- bson c object notation */

#define OBJ_BSON_BCON_STACK_MAX 100

typedef enum obj_bson_bcon_type obj_bson_bcon_type_t;
typedef struct obj_bson_bcon_append_frame_s obj_bson_bcon_append_frame_t;
typedef struct obj_bson_bcon_append_ctx_s obj_bson_bcon_append_ctx_t;
typedef union obj_bson_bcon_append_u obj_bson_bcon_append_t;

enum obj_bson_bcon_type {
    /* base bson type */
    OBJ_BSON_BCON_TYPE_UTF8,
    OBJ_BSON_BCON_TYPE_DOUBLE,
    OBJ_BSON_BCON_TYPE_OBJECT,
    OBJ_BSON_BCON_TYPE_ARRAY,
    OBJ_BSON_BCON_TYPE_BINARY,
    OBJ_BSON_BCON_TYPE_BOOL,
    OBJ_BSON_BCON_TYPE_NULL,
    OBJ_BSON_BCON_TYPE_INT32,
    OBJ_BSON_BCON_TYPE_INT64,

    /* OBJ_BSON_BCON_TYPE_BCON, */
    OBJ_BSON_BCON_TYPE_ARRAY_START,
    OBJ_BSON_BCON_TYPE_ARRAY_END,
    OBJ_BSON_BCON_TYPE_OBJECT_START,
    OBJ_BSON_BCON_TYPE_OBJECT_END,
    OBJ_BSON_BCON_TYPE_END
};

struct obj_bson_bcon_append_frame_s {
    int i;
    obj_bool_t is_array;
    obj_bson_t bson;
};

struct obj_bson_bcon_append_ctx_s {
    int n;
    obj_bson_bcon_append_frame_t stack[OBJ_BSON_BCON_STACK_MAX];
};

union obj_bson_bcon_append_u {
    char *v_utf8;
    double v_double;
    obj_bson_t *v_object;
    obj_bson_t *v_array;
    obj_bson_t *v_bcon;
    struct {
        obj_uint8_t *data;
        obj_int32_t len;
    } v_binary;
    obj_bool_t v_bool;
    obj_int32_t v_int32;
    obj_int64_t v_int64;
};

#define obj_bson_bcon_stack_element(ctx, delta, name) (ctx->stack[(delta) + ctx->n].name)
#define obj_bson_bcon_stack_bson(ctx, delta) (((delta) + ctx->n) == 0 ? bson : &obj_bson_bcon_stack_element(ctx, delta, bson))

char *obj_bson_bcon_magic();
obj_bson_t *obj_bson_bcon_new(void *unused, ...);
void obj_bson_bcon_append_ctx_init(obj_bson_bcon_append_ctx_t *ctx);
void obj_bson_bcon_append_ctx_va(obj_bson_t *bson, obj_bson_bcon_append_ctx_t *ctx, va_list *ap);

#define OBJ_BSON_BCON_MAGIC obj_bson_bcon_magic()
#define OBJ_BSON_BCON_NEW(...) obj_bson_bcon_new(NULL, __VA_ARGS__, (void *)NULL)
#define OBJ_BSON_BCON_UTF8(val) OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_UTF8, (( char *)val)
#define OBJ_BSON_BCON_DOUBLE(val) OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_DOUBLE, ((double)val)
#define OBJ_BSON_BCON_OBJECT(val) OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_OBJECT, (( obj_bson_t *)val)
#define OBJ_BSON_BCON_ARRAY(val) OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_ARRAY, (( obj_bson_t *)val)
#define OBJ_BSON_BCON_BINARY(val, len) OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_BINARY, (( obj_uint8_t *)val), ((obj_int32_t)len)
#define OBJ_BSON_BCON_BOOL(val) OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_BOOL, ((obj_bool_t )val)
#define OBJ_BSON_BCON_NULL OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_NULL
#define OBJ_BSON_BCON_INT32(val) OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_INT32, ((obj_int32_t)val)
#define OBJ_BSON_BCON_INT64(val) OBJ_BSON_BCON_MAGIC, OBJ_BSON_BCON_TYPE_INT64, ((obj_int64_t)val)

#endif  /* OBJ_BSON_BCON_H */