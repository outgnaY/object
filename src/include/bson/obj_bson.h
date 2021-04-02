#ifndef OBJ_BSON_H
#define OBJ_BSON_H

#include "obj_core.h"


#define OBJ_BSON_MAX_SIZE 0x7fffffff

#define OBJ_BSON_INIT_SIZE 128

typedef enum obj_bson_flag obj_bson_flag_t;
typedef enum obj_bson_type obj_bson_type_t;
typedef struct obj_bson_s obj_bson_t;
typedef struct obj_bson_kv_s obj_bson_kv_t;
typedef struct obj_bson_value_s obj_bson_value_t;

/* bson flags */
enum obj_bson_flag {
    OBJ_BSON_FLAG_NONE = 0,
    OBJ_BSON_FLAG_STATIC = (1 << 0),               /* init from a static context */
    OBJ_BSON_FLAG_RDONLY = (1 << 1),               /* read only */
    OBJ_BSON_FLAG_NOFREE = (1 << 2),               /* don't free data */
    OBJ_BSON_FLAG_CHILD = (1 << 3),                
    OBJ_BSON_FLAG_IN_CHILD = (1 << 4)
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
    /* only used internal */
    OBJ_BSON_TYPE_MIN = 0x13,               /* for special use */
    OBJ_BSON_TYPE_MAX = 0x14                /* for special use */
};



struct obj_bson_s {
    obj_bson_flag_t flag;                   /* flag */
    obj_int32_t len;                        /* length of bson */
    obj_uint8_t *data;                      /* data pointer */
    obj_int32_t cap;                        /* capacity */
    
    int depth;                              /* current depth */
    obj_bson_t *parent;                     /* parent bson if this is a child */
    int offset;                             /* offset in buffer */
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
            obj_int32_t len;
        } v_utf8;
        struct {
            obj_uint8_t *data;
            obj_int32_t len;
        } v_object;
        struct {
            obj_uint8_t *data;
            obj_int32_t len;
        } v_array;
        struct {
            obj_uint8_t *data;
            obj_int32_t len;
        } v_binary;
        obj_bool_t v_bool;
        obj_int32_t v_int32;
        obj_int64_t v_int64;
    } value;
};

void obj_bson_print(obj_bson_t *bson);
obj_bson_t *obj_bson_create();
obj_bson_t *obj_bson_create_with_data(obj_uint8_t *data, obj_int32_t len);
void obj_bson_init_static_with_len(obj_bson_t *bson, obj_uint8_t *data, obj_int32_t len);
obj_bool_t obj_bson_is_empty(obj_bson_t *bson);
void obj_bson_destroy(obj_bson_t *bson);
obj_bson_value_t *obj_bson_get_path(obj_bson_t *bson, char *path);
/* int obj_bson_compare(obj_bson_t *bson1, obj_bson_t *bson2, obj_bson_t *pattern); */
char *obj_bson_type_to_name(obj_bson_type_t type);
obj_uint8_t *obj_bson_data(obj_bson_t *bson);
void obj_bson_append_double(obj_bson_t *bson, char *key, int key_len, double value);
void obj_bson_append_utf8(obj_bson_t *bson, char *key, int key_len, char *value, int value_len);
void obj_bson_append_object(obj_bson_t *bson, char *key, int key_len, obj_bson_t *value);
void obj_bson_append_array(obj_bson_t *bson, char *key, int key_len, obj_bson_t *value);
void obj_bson_append_binary(obj_bson_t *bson, char *key, int key_len, obj_uint8_t *value, int value_len);
void obj_bson_append_bool(obj_bson_t *bson, char *key, int key_len, obj_bool_t value);
void obj_bson_append_null(obj_bson_t *bson, char *key, int key_len);
void obj_bson_append_int32(obj_bson_t *bson, char *key, int key_len, obj_int32_t value);
void obj_bson_append_int64(obj_bson_t *bson, char *key, int key_len, obj_int64_t value);
void obj_bson_append_array_begin(obj_bson_t *bson, char *key, int key_len, obj_bson_t *child);
void obj_bson_append_array_end(obj_bson_t *bson, obj_bson_t *child);
void obj_bson_append_object_begin(obj_bson_t *bson, char *key, int key_len, obj_bson_t *child);
void obj_bson_append_object_end(obj_bson_t *bson, obj_bson_t *child);
void obj_bson_append_min(obj_bson_t *bson, char *key, int key_len);
void obj_bson_append_max(obj_bson_t *bson, char *key, int key_len);
void obj_bson_append_value(obj_bson_t *bson, char *key, int key_len, obj_bson_value_t *value);

#endif  /* OBJ_BSON_H */