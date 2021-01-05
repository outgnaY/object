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

obj_size_t obj_sds_len(const obj_sds s);
obj_size_t obj_sds_avail(const obj_sds s);
void obj_sds_setlen(obj_sds s, obj_size_t newlen);
void obj_sds_inclen(obj_sds s, obj_size_t inc);
obj_size_t obj_sds_alloc(const obj_sds s);
void obj_sds_setalloc(obj_sds s, obj_size_t newlen);
int obj_sds_header_size(char type);
char obj_sds_req_type(obj_size_t string_size);
obj_sds obj_sds_newlen(const void *init, obj_size_t initlen);
obj_sds obj_sds_empty();
obj_sds obj_sds_new(const char *init);
obj_sds obj_sds_dup(const obj_sds s);
void obj_sds_free(obj_sds s);
void obj_sds_updatelen(obj_sds s);
void obj_sds_clear(obj_sds s);
obj_sds obj_sds_make_room_for(obj_sds s, obj_size_t addlen);
obj_sds obj_sds_remove_free_space(obj_sds s);
obj_size_t obj_sds_alloc_size(obj_sds s);
void *obj_sds_alloc_ptr(obj_sds s);
void obj_sds_incr_len(obj_sds s, int incr);
obj_sds obj_sds_catlen(obj_sds s, const void *t, obj_size_t len);
obj_sds obj_sds_cat(obj_sds s, const char *t);
obj_sds obj_sds_catsds(obj_sds s, const obj_sds t);
obj_sds obj_sds_cpylen(obj_sds s, const char *t, obj_size_t len);
obj_sds obj_sds_cpy(obj_sds s, const char *t);
int obj_sds_ll2str(char *s, long long value);
int obj_sds_ull2str(char *s, unsigned long long value);
obj_sds obj_sds_fromlonglong(long long value);
obj_sds obj_sds_catvprintf(obj_sds s, const char *fmt, va_list ap);
obj_sds obj_sds_catprintf(obj_sds s, const char *fmt, ...);
obj_sds obj_sds_catfmt(obj_sds s, const char *fmt, ...);
void obj_sds_range(obj_sds s, int start, int end);
void obj_sds_tolower(obj_sds s);
void obj_sds_toupper(obj_sds s);
int obj_sds_cmp(const obj_sds s1, const obj_sds s2);
obj_sds obj_sds_join(const char **argv, int argc, const char *sep);
obj_sds obj_sds_joinsds(const obj_sds *argv, int argc, const char *sep, obj_size_t seplen);
void obj_sds_dump(const obj_sds s);

#endif  /* OBJ_SDS_H */