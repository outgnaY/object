#ifndef OBJ_BSON_H
#define OBJ_BSON_H

#include "obj_core.h"
#include "util/obj_endian.h"

#define OBJ_BSON_LITTLE_ENDIAN 0
#define OBJ_BSON_BIG_ENDIAN 1



#define OBJ_BSON_MAX_SIZE 0x7fffffff

#define OBJ_BSON_STACK_DATA_SIZE 128

#define OBJ_BSON_HEAP_DATA_INIT 128

typedef enum obj_bson_flag obj_bson_flag_t;
typedef enum obj_bson_type obj_bson_type_t;
typedef struct obj_bson_s obj_bson_t;
typedef struct obj_bson_stack_s obj_bson_stack_t;
typedef struct obj_bson_heap_s obj_bson_heap_t;
typedef struct obj_bson_kv_s obj_bson_kv_t;
typedef struct obj_bson_value_s obj_bson_value_t;

/* bson flags */
enum obj_bson_flag {
    OBJ_BSON_FLAG_STACK = 1,
    OBJ_BSON_FLAG_HEAP = 2,
    OBJ_BSON_FLAG_RDONLY = 4,
    OBJ_BSON_FLAG_DATA_EXTERNAL = 8
};

/* bson types */
enum obj_bson_type {
    OBJ_BSON_TYPE_EOO = 0x00,               /* end of document */
    OBJ_BSON_TYPE_DOUBLE = 0x01,            /* double */
    OBJ_BSON_TYPE_UTF8 = 0x02,              /* utf-8 string */
    OBJ_BSON_TYPE_OBJECT = 0x03,            /* object */
    OBJ_BSON_TYPE_ARRAY = 0x04,             /* array */
    OBJ_BSON_TYPE_BINARY = 0x05,            /* binary */
    OBJ_BSON_TYPE_BOOL = 0x08,              /* bool */
    OBJ_BSON_TYPE_NULL = 0x0A,              /* null */
    OBJ_BSON_TYPE_INT32 = 0x10,             /* 32 bit integer */
    OBJ_BSON_TYPE_INT64 = 0x12,             /* 64 bit integer */
};


struct obj_bson_s {
    obj_bson_flag_t flag;
    int len;
};

/* alloc on stack by default */
struct obj_bson_stack_s {
    obj_bson_t base;
    obj_uint8_t data[OBJ_BSON_STACK_DATA_SIZE];
};

/* alloc on heap */
struct obj_bson_heap_s {
    obj_bson_t base;
    obj_uint8_t *data;
};

/* a k-v pair of bson object */
struct obj_bson_kv_s {
    obj_uint8_t *data;
    int field_name_size;
    int total_size;
};

struct obj_bson_value_s {
    obj_bson_type_t type;
    union {
        double v_double;
        struct {
            char *str;
            int len;
        } v_utf8;
        struct {
            obj_uint8_t *data;
            int len;
        } v_doc;
        struct {
            obj_uint8_t *data;
            int len;
        } v_binary;
        obj_bool_t v_bool;
        obj_int32_t v_int32;
        obj_int64_t v_int64;
    } value;
};

void obj_bson_print(obj_bson_t *bson);

obj_bson_t *obj_bson_init_heap();

obj_bson_t *obj_bson_init_data(obj_uint8_t *data, int len);

void obj_bson_destroy(obj_bson_t *bson);

const char *obj_bson_type_to_name(obj_bson_type_t type);

obj_uint8_t *obj_bson_data(const obj_bson_t *bson);

obj_bool_t obj_bson_append_double(obj_bson_t *bson, const char *key, int key_len, double value);

obj_bool_t obj_bson_append_utf8(obj_bson_t *bson, const char *key, int key_len, const char *value, int value_len);

obj_bool_t obj_bson_append_object(obj_bson_t *bson, const char *key, int key_len, const obj_bson_t *value);

obj_bool_t obj_bson_append_array(obj_bson_t *bson, const char *key, int key_len, const obj_bson_t *value);

obj_bool_t obj_bson_append_binary(obj_bson_t *bson, const char *key, int key_len, const obj_uint8_t *value, int value_len);

obj_bool_t obj_bson_append_bool(obj_bson_t *bson, const char *key, int key_len, obj_bool_t value);

obj_bool_t obj_bson_append_null(obj_bson_t *bson, const char *key, int key_len);

obj_bool_t obj_bson_append_int32(obj_bson_t *bson, const char *key, int key_len, obj_int32_t value);

obj_bool_t obj_bson_append_int64(obj_bson_t *bson, const char *key, int key_len, obj_int64_t value);



#endif  /* OBJ_BSON_H */