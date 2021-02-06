#include "obj_core.h"

static const char *s_obj_bson_bcon_magic = "$";

static void obj_bson_bcon_stack_push_array(obj_bson_bcon_append_ctx_t *ctx);
static void obj_bson_bcon_stack_push_object(obj_bson_bcon_append_ctx_t *ctx);
static void obj_bson_bcon_stack_pop_array(obj_bson_bcon_append_ctx_t *ctx);
static void obj_bson_bcon_stack_pop_object(obj_bson_bcon_append_ctx_t *ctx);
static void obj_bson_bcon_append_single(obj_bson_t *bson, obj_bson_bcon_type_t type, const char *key, obj_bson_bcon_append_t *val);
static obj_bson_bcon_type_t obj_bson_bcon_append_tokenize(va_list *ap, obj_bson_bcon_append_t *u);

inline const char *obj_bson_bcon_magic() {
    return s_obj_bson_bcon_magic;
}

static inline void obj_bson_bcon_stack_push_array(obj_bson_bcon_append_ctx_t *ctx) {
    obj_assert(ctx->n < (OBJ_BSON_BCON_STACK_MAX - 1));
    ctx->n++;
    obj_bson_bcon_stack_element(ctx, 0, i) = 0;
    obj_bson_bcon_stack_element(ctx, 0, is_array) = true;
}

static inline void obj_bson_bcon_stack_push_object(obj_bson_bcon_append_ctx_t *ctx) {
    obj_assert(ctx->n < (OBJ_BSON_BCON_STACK_MAX - 1));
    ctx->n++;
    obj_bson_bcon_stack_element(ctx, 0, i) = 0;
    obj_bson_bcon_stack_element(ctx, 0, is_array) = false;
}

static inline void obj_bson_bcon_stack_pop_array(obj_bson_bcon_append_ctx_t *ctx) {
    obj_assert(ctx->n > 0);
    obj_assert(obj_bson_bcon_stack_element(ctx, 0, is_array));
    ctx->n--;
}

static inline void obj_bson_bcon_stack_pop_object(obj_bson_bcon_append_ctx_t *ctx) {
    obj_assert(ctx->n > 0);
    obj_assert(!obj_bson_bcon_stack_element(ctx, 0, is_array));
    ctx->n--;
}

/* new bcon */
obj_bson_t *obj_bson_bcon_new(void *unused, ...) {
    va_list ap;
    obj_bson_bcon_append_ctx_t ctx;
    obj_bson_t *bson;
    obj_bson_bcon_append_ctx_init(&ctx);
    bson = obj_bson_init();
    va_start(ap, unused);
    obj_bson_bcon_append_ctx_va(bson, &ctx, &ap);
    va_end(ap);
    return bson;
}



/* init append context */
void obj_bson_bcon_append_ctx_init(obj_bson_bcon_append_ctx_t *ctx) {
    ctx->n = 0;
    ctx->stack[0].is_array = 0;
}

void obj_bson_bcon_append_ctx_va(obj_bson_t *bson, obj_bson_bcon_append_ctx_t *ctx, va_list *ap) {
    obj_bson_bcon_type_t type;
    const char *key;
    char i_str[16];
    obj_bson_bcon_append_t u = {0};
    key = i_str;
    while (true) {
        if (obj_bson_bcon_stack_element(ctx, 0, is_array)) {
            /* array element key */
            snprintf(i_str, sizeof(i_str), "%u", obj_bson_bcon_stack_element(ctx, 0, i));
            key = i_str;
            obj_bson_bcon_stack_element(ctx, 0, i)++;
        } else {
            type = obj_bson_bcon_append_tokenize(ap, &u);
            if (type == OBJ_BSON_BCON_TYPE_END) {
                return;
            }
            /* inside array */
            if (type == OBJ_BSON_BCON_TYPE_OBJECT_END) {
                obj_bson_append_object_end(obj_bson_bcon_stack_bson(ctx, -1), obj_bson_bcon_stack_bson(ctx, 0));
                obj_bson_bcon_stack_pop_object(ctx);
                continue;
            }
            /*
            if (type == OBJ_BSON_BCON_TYPE_BCON) {

            }
            */
            obj_assert(type == OBJ_BSON_BCON_TYPE_UTF8);
            key = u.v_utf8;

        }
        type = obj_bson_bcon_append_tokenize(ap, &u);
        obj_assert(type != OBJ_BSON_BCON_TYPE_END);
        switch (type) {
            /*
            case OBJ_BSON_BCON_TYPE_BCON:

                break;
            */
            case OBJ_BSON_BCON_TYPE_OBJECT_START:
                obj_bson_bcon_stack_push_object(ctx);
                obj_bson_append_object_begin(obj_bson_bcon_stack_bson(ctx, -1), key, obj_strlen(key), obj_bson_bcon_stack_bson(ctx, 0));
                break;
            case OBJ_BSON_BCON_TYPE_OBJECT_END:
                obj_bson_append_object_end(obj_bson_bcon_stack_bson(ctx, -1), obj_bson_bcon_stack_bson(ctx, 0));
                obj_bson_bcon_stack_pop_object(ctx);
                break;
            case OBJ_BSON_BCON_TYPE_ARRAY_START:
                obj_bson_bcon_stack_push_array(ctx);
                obj_bson_append_array_begin(obj_bson_bcon_stack_bson(ctx, -1), key, obj_strlen(key), obj_bson_bcon_stack_bson(ctx, 0));
                break;
            case OBJ_BSON_BCON_TYPE_ARRAY_END:
                obj_bson_append_array_end(obj_bson_bcon_stack_bson(ctx, -1), obj_bson_bcon_stack_bson(ctx, 0));
                obj_bson_bcon_stack_pop_array(ctx);
                break;
            default:
                obj_bson_bcon_append_single(obj_bson_bcon_stack_bson(ctx, 0), type, key, &u);
                break;
        }
    }
}

/* wrapper */
static void obj_bson_bcon_append_single(obj_bson_t *bson, obj_bson_bcon_type_t type, const char *key, obj_bson_bcon_append_t *val) {
    switch (type) {
        case OBJ_BSON_BCON_TYPE_UTF8:
            obj_bson_append_utf8(bson, key, obj_strlen(key), val->v_utf8, obj_strlen(val->v_utf8));
            break;
        case OBJ_BSON_BCON_TYPE_DOUBLE:
            obj_bson_append_double(bson, key, obj_strlen(key), val->v_double);
            break;
        case OBJ_BSON_BCON_TYPE_ARRAY:
            obj_bson_append_array(bson, key, obj_strlen(key), val->v_array);
            break;
        case OBJ_BSON_BCON_TYPE_OBJECT:
            obj_bson_append_object(bson, key, obj_strlen(key), val->v_object);
            break;
        case OBJ_BSON_BCON_TYPE_BINARY:
            obj_bson_append_binary(bson, key, obj_strlen(key), val->v_binary.data, val->v_binary.len);
            break;
        case OBJ_BSON_BCON_TYPE_BOOL:
            obj_bson_append_bool(bson, key, obj_strlen(key), val->v_bool);
            break;
        case OBJ_BSON_BCON_TYPE_NULL:
            obj_bson_append_null(bson, key, obj_strlen(key));
            break;
        case OBJ_BSON_BCON_TYPE_INT32:
            obj_bson_append_int32(bson, key, obj_strlen(key), val->v_int32);
            break;
        case OBJ_BSON_BCON_TYPE_INT64:
            obj_bson_append_int64(bson, key, obj_strlen(key), val->v_int64);
            break;
        default:
            /* unknown type */
            obj_assert(0);
            break;
    }
}

/* fetch next */
static obj_bson_bcon_type_t obj_bson_bcon_append_tokenize(va_list *ap, obj_bson_bcon_append_t *u) {
    char *mark;
    obj_bson_bcon_type_t type;
    mark = va_arg(*ap, char *);
    if (mark == NULL) {
        type = OBJ_BSON_BCON_TYPE_END;
    } else if (mark == (char *)s_obj_bson_bcon_magic) {
        type = va_arg(*ap, obj_bson_bcon_type_t);
        switch (type) {
            case OBJ_BSON_BCON_TYPE_UTF8:
                u->v_utf8 = va_arg(*ap, char *);
                break;
            case OBJ_BSON_BCON_TYPE_DOUBLE:
                u->v_double = va_arg(*ap, double);
                break;
            case OBJ_BSON_BCON_TYPE_OBJECT:
                u->v_object = va_arg(*ap, obj_bson_t *);
                break;
            case OBJ_BSON_BCON_TYPE_ARRAY:
                u->v_array = va_arg(*ap, obj_bson_t *);
                break;
            case OBJ_BSON_BCON_TYPE_BINARY:
                u->v_binary.data = va_arg(*ap, obj_uint8_t *);
                u->v_binary.len = va_arg(*ap, obj_int32_t);
                break;
            case OBJ_BSON_BCON_TYPE_BOOL:
                u->v_bool = va_arg(*ap, obj_bool_t);
                break;
            case OBJ_BSON_BCON_TYPE_NULL:
                break;
            case OBJ_BSON_BCON_TYPE_INT32:
                u->v_int32 = va_arg(*ap, obj_int32_t);
                break;
            case OBJ_BSON_BCON_TYPE_INT64:
                u->v_int64 = va_arg(*ap, obj_int64_t);
                break;
            /*
            case OBJ_BSON_BCON_TYPE_BCON:
                u->v_bcon = va_arg(*ap, obj_bson_t *);
                break;
            */
            default:
                /* can't reach here */
                obj_assert(0);
                break;
        }
    } else {
        switch (mark[0]) {
            case '{':
                type = OBJ_BSON_BCON_TYPE_OBJECT_START;
                break;
            case '}':
                type = OBJ_BSON_BCON_TYPE_OBJECT_END;
                break;
            case '[':
                type = OBJ_BSON_BCON_TYPE_ARRAY_START;
                break;
            case ']':
                type = OBJ_BSON_BCON_TYPE_ARRAY_END;
                break;
            default:
                /* key */
                type = OBJ_BSON_BCON_TYPE_UTF8;
                u->v_utf8 = mark;
                break;
        }
    }
    return type;
}