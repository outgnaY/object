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

#endif  /* OBJ_SDS_H */