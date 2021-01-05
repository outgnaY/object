#ifndef OBJ_SDS_H
#define OBJ_SDS_H

#include "obj_core.h"

/* simple dynamic string implementation */
typedef char *obj_sds;

struct __attribute__ ((__packed__)) obj_sds_header5 {
    unsigned char flags;            /* 3 lsb of type, and 5 msb of string length */
    char buf[];
};

struct __attribute__ ((__packed__)) obj_sds_header8 {
    obj_uint8_t len;                /* used */
    obj_uint8_t alloc;              /* excluding the header and null terminator */
    unsigned char flags;            /* 3 lsb of type, 5 unused bits */
    char buf[];
};

struct __attribute__ ((__packed__)) obj_sds_header16 {
    obj_uint16_t len;               /* used */
    obj_uint16_t alloc;             /* excluding the header and null terminator */
    unsigned char flags;            /* 3 lsb of type, 5 unused bits */
    char buf[];
};

struct __attribute__ ((__packed__)) obj_sds_header32 {
    obj_uint32_t len;               /* used */
    obj_uint32_t alloc;             /* excluding the header and null terminator */
    unsigned char flags;            /* 3 lsb of type, 5 unused bits */
    char buf[];
};

struct __attribute__ ((__packed__)) obj_sds_header64 {
    obj_uint64_t len;               /* used */
    obj_uint64_t alloc;             /* excluding the header and null terminator */
    unsigned char flags;            /* 3 lsb of type, 5 unused bits */
    char buf[];
};

#define OBJ_SDS_TYPE_5 0
#define OBJ_SDS_TYPE_8 1
#define OBJ_SDS_TYPE_16 2
#define OBJ_SDS_TYPE_32 3
#define OBJ_SDS_TYPE_64 4
#define OBJ_SDS_TYPE_MASK 7
#define OBJ_SDS_TYPE_BITS 3

#define OBJ_SDS_MAX_PREALLOC (1024 * 1024)
#define OBJ_SDS_LLSTR_SIZE 21

#define OBJ_SDS_HEADER_VAR(T, s) struct obj_sds_header##T *sh = (struct obj_sds_header##T *)((s) - (sizeof(struct obj_sds_header##T)));
#define OBJ_SDS_HEADER(T, s) ((struct obj_sds_header##T *)((s) - (sizeof(struct obj_sds_header##T))))
#define OBJ_SDS_TYPE_5_LEN(f) ((f)>>OBJ_SDS_TYPE_BITS)


obj_sds obj_sds_newlen(const void *init, obj_size_t initlen);
obj_sds obj_sds_new(const char *init);



#endif  /* OBJ_SDS_H */