#ifndef OBJ_COMMON_H
#define OBJ_COMMON_H

#include <sys/types.h>

#define OBJ_VERSION 1.0

/* types */
typedef size_t obj_size_t;
typedef int8_t obj_int8_t;
typedef int16_t obj_int16_t;
typedef int32_t obj_int32_t;
typedef int64_t obj_int64_t;
typedef u_int8_t obj_uint8_t;
typedef u_int16_t obj_uint16_t;
typedef u_int32_t obj_uint32_t;
typedef u_int64_t obj_uint64_t;

typedef int obj_bool_t;
#define true 1
#define false 0

typedef unsigned int obj_rel_time_t; 

#define obj_assert(expr) assert(expr)

typedef enum obj_global_error_code obj_global_error_code_t;

#define OBJ_INT32_MAX 0x7fffffff
#define OBJ_UINT32_MAX 0xffffffff
#define OBJ_INT64_MAX 0x7fffffffffffffff
#define OBJ_UINT64_MAX 0xffffffffffffffff

/* time related */

/* absolute times */

/* second */
typedef long obj_abs_time_second;
/* millisecond */
typedef long obj_abs_time_msecond;
/* microsecond */
typedef long obj_abs_time_usecond;
/* nanosecond */
typedef long obj_abs_time_nsecond;

/* durations */

/* second */
typedef long obj_duration_second;
/* millisecond */
typedef long obj_duration_msecond;
/* microsecond */
typedef long obj_duration_usecond;
/* nanosecond */
typedef long obj_duration_nsecond;

#define OBJ_DURATION_SECOND_MAX OBJ_INT32_MAX
#define OBJ_DURATION_MSECOND_MAX OBJ_INT32_MAX
#define OBJ_DURATION_USECOND_MAX OBJ_INT32_MAX
#define OBJ_DURATION_NSECOND_MAX OBJ_INT32_MAX


/* alignment requiredment */
#define OBJ_ALIGNMENT sizeof(unsigned long)

/* align SIZE to given ALIGN */
#define OBJ_ALIGN(SIZE, ALIGN) (((SIZE) + (ALIGN - 1)) & ~(ALIGN - 1))

/* determine number of elements of an array */
#define OBJ_NELEM(a) (sizeof(a) / sizeof(a[0]))

/* error description */
#define OBJ_STRERROR(no) (strerror(no) != NULL ? strerror(no) : "unknown error")


/* error code */
enum obj_global_error_code {
    /* common error code */
    OBJ_CODE_OK = 0,
    /* bson error code */
    OBJ_CODE_BSON_VALIDATE_EXCEED_MAX_DEPTH = 10001,
    OBJ_CODE_BSON_VALIDATE_WRONG_SIZE = 10003,
    OBJ_CODE_BSON_INVALID_BSON = 10004,
    OBJ_CODE_BSON_INVALID_BSON_TYPE = 10005,
    OBJ_CODE_BSON_SIZE_OVERFLOW = 10006,
    OBJ_CODE_BSON_INVALID_BOOLEAN_VALUE = 10007,
    OBJ_CODE_BSON_NOT_NULL_TERMINATED_STRING = 10008,
    OBJ_CODE_BSON_NOT_NULL_TERMINATED_CSTRING = 10009,
    OBJ_CODE_BSON_ITERATOR_INIT_ERROR = 10010,
    /* expression */
    OBJ_CODE_EXPR_BAD_VALUE = 20001,
    OBJ_CODE_EXPR_PARSER_NOT_FOUND = 20003,
    OBJ_CODE_EXPR_TYPE_NOT_FOUND = 20004,
    /* database operation error code */
    OBJ_CODE_DB_NOT_EXISTS = 30001,
    OBJ_CODE_DB_NOMEM = 30002,
    OBJ_CODE_DB_NOT_OPENED = 30003,
    OBJ_CODE_DB_COLLECTION_ALREADY_EXISTS = 30004,
    /* query error code */
    OBJ_CODE_QUERY_WRONG_TYPE = 40002,
    OBJ_CODE_QUERY_MISSING_FIELD = 40003,
    /* query planner error code */
    OBJ_CODE_QUERY_PLAN_NOMEM = 50001,
    /* index */
    OBJ_CODE_INDEX_NUM_EXCEED = 60001,
    OBJ_CODE_INDEX_DUPLICATE = 60002,
    OBJ_CODE_INDEX_KEY_PATTERN_INVALID = 60003
};



#endif  /* OBJ_COMMON_H */