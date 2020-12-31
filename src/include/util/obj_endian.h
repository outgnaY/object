#ifndef OBJ_ENDIAN_H
#define OBJ_ENDIAN_H

#include "obj_core.h"

static inline obj_uint16_t obj_uint16_swap(obj_uint16_t v) {
    return ((v & 0x00ff) << 8) | ((v & 0xff00) >> 8);
}

static inline obj_int16_t obj_int16_swap(obj_int16_t v) {
    return (obj_int16_t)obj_uint16_swap((obj_uint16_t)v);
}

static inline obj_uint32_t obj_uint32_swap(obj_uint32_t v) {
    return ((v & 0x000000ffu) << 24) | ((v & 0x0000ff00u) << 8) | ((v & 0x00ff0000u) >> 8) | ((v & 0xff000000u) >> 24);
}

static inline obj_int32_t obj_int32_swap(obj_int32_t v) {
    return (obj_int32_t)obj_uint32_swap((obj_uint32_t)v);
}

static inline obj_uint64_t obj_uint64_swap(obj_uint64_t v) {
    return ((v & 0x00000000000000ffull) << 56) |
           ((v & 0x000000000000ff00ull) << 40) |
           ((v & 0x0000000000ff0000ull) << 24) |
           ((v & 0x00000000ff000000ull) << 8) |
           ((v & 0x000000ff00000000ull) >> 8) |
           ((v & 0x0000ff0000000000ull) >> 24) |
           ((v & 0x00ff000000000000ull) >> 40) |
           ((v & 0xff00000000000000ull) >> 56);
}

static inline obj_int64_t obj_int64_t_swap(obj_int64_t v) {
    return (obj_int64_t)obj_uint64_swap((obj_uint64_t)v);
}

static inline double obj_double_swap(double v) {
    obj_uint64_t uv;
    memcpy(&uv, &v, sizeof(v));
    uv = obj_uint64_swap(uv);
    memcpy(&v, &uv, sizeof(v));
    return v;
}

#define obj_to_le(v, type) obj_##type##_to_le(v)

#define obj_uint16_swap_le_be(v) obj_uint16_swap(v) 
#define obj_int16_swap_le_be(v) obj_int16_swap(v)
#define obj_uint32_swap_le_be(v) obj_uint32_swap(v)
#define obj_int32_swap_le_be(v) obj_int32_swap(v)
#define obj_uint64_swap_le_be(v) obj_uint64_swap(v)
#define obj_int64_swap_le_be(v) obj_int64_swap(v)
#define obj_double_swap_le_be(v) obj_double_swap(v)

#define obj_uint8_from_le(v) ((obj_uint8_t)(v))
#define obj_uint8_to_le(v) ((obj_uint8_t)(v))
#define obj_uint8_from_be(v) ((obj_uint8_t)(v))
#define obj_uint8_to_be ((obj_uint8_t)(v))

#define obj_int8_from_le(v) ((obj_int8_t)(v))
#define obj_int8_to_le(v) ((obj_int8_t)(v))
#define obj_int8_from_be(v) ((obj_int8_t)(v))
#define obj_int8_to_be ((obj_int8_t)(v))

#if OBJ_BYTE_ORDER == OBJ_LITTLE_ENDIAN

#define obj_uint16_from_le(v) ((obj_uint16_t)(v))
#define obj_uint16_to_le(v) ((obj_uint16_t)(v))
#define obj_uint16_from_be(v) (obj_uint16_swap_le_be(v))
#define obj_uint16_to_be(v) (obj_uint16_swap_le_be(v))

#define obj_int16_from_le(v) ((obj_int16_t)(v))
#define obj_int16_to_le(v) ((obj_int16_t)(v))
#define obj_int16_from_be(v) (obj_int16_swap_le_be(v))
#define obj_int16_to_be(v) (obj_int16_swap_le_be(v))

#define obj_uint32_from_le(v) ((obj_uint32_t)(v))
#define obj_uint32_to_le(v) ((obj_uint32_t)(v))
#define obj_uint32_from_be(v) (obj_uint32_swap_le_be(v))
#define obj_uint32_to_be(v) (obj_uint32_swap_le_be(v))

#define obj_int32_from_le(v) ((obj_int32_t)(v))
#define obj_int32_to_le(v) ((obj_int32_t)(v))
#define obj_int32_from_be(v) (obj_int32_swap_le_be(v))
#define obj_int32_to_be(v) (obj_int32_swap_le_be(v))

#define obj_uint64_from_le(v) ((obj_uint64_t)(v))
#define obj_uint64_to_le(v) ((obj_uint64_t)(v))
#define obj_uint64_from_be(v) (obj_uint64_swap_le_be(v))
#define obj_uint64_to_be(v) (obj_uint64_swap_le_be(v))

#define obj_int64_from_le(v) ((obj_int64_t)(v))
#define obj_int64_to_le(v) ((obj_int64_t)(v))
#define obj_int64_from_be(v) (obj_int64_swap_le_be(v))
#define obj_int64_to_be(v) (obj_int64_swap_le_be(v))

#define obj_double_from_le(v) ((double)(v))
#define obj_double_to_le(v) ((double)(v))

#else

#define obj_uint16_from_be(v) ((obj_uint16_t)(v))
#define obj_uint16_to_be(v) ((obj_uint16_t)(v))
#define obj_uint16_from_le(v) (obj_uint16_swap_le_be(v))
#define obj_uint16_to_le(v) (obj_uint16_swap_le_be(v))

#define obj_int16_from_be(v) ((obj_int16_t)(v))
#define obj_int16_to_be(v) ((obj_int16_t)(v))
#define obj_int16_from_le(v) (obj_int16_swap_le_be(v))
#define obj_int16_to_le(v) (obj_int16_swap_le_be(v))

#define obj_uint32_from_be(v) ((obj_uint32_t)(v))
#define obj_uint32_to_be(v) ((obj_uint32_t)(v))
#define obj_uint32_from_le(v) (obj_uint32_swap_le_be(v))
#define obj_uint32_to_le(v) (obj_uint32_swap_le_be(v))

#define obj_int32_from_be(v) ((obj_int32_t)(v))
#define obj_int32_to_be(v) ((obj_int32_t)(v))
#define obj_int32_from_le(v) (obj_int32_swap_le_be(v))
#define obj_int32_to_le(v) (obj_int32_swap_le_be(v))

#define obj_uint64_from_be(v) ((obj_uint64_t)(v))
#define obj_uint64_to_be(v) ((obj_uint64_t)(v))
#define obj_uint64_from_le(v) (obj_uint64_swap_le_be(v))
#define obj_uint64_to_le(v) (obj_uint64_swap_le_be(v))

#define obj_int64_from_be(v) ((obj_int64_t)(v))
#define obj_int64_to_be(v) ((obj_int64_t)(v))
#define obj_int64_from_le(v) (obj_int64_swap_le_be(v))
#define obj_int64_to_le(v) (obj_int64_swap_le_be(v))

#define obj_double_from_le(v) (obj_double_swap_le_be(v))
#define obj_double_to_le(v) (obj_double_swap_le_be(v))

#endif

#endif  /* OBJ_ENDIAN_H */