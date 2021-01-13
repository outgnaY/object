#ifndef OBJ_BUFFER_H
#define OBJ_BUFFER_H

#include "obj_core.h"

typedef struct obj_buffer_s obj_buffer_t;

/* buffer hold by connection */
struct obj_buffer_s {
    obj_uint8_t *buf;
    int buf_size;
    int read_index;
    int write_index;
};

#define OBJ_BUFFER_INIT_SIZE (1 << 12)    /* 4KB */

#define OBJ_BUFFER_MAX_SIZE (1 << 30)     /* 1GB */

obj_buffer_t *obj_buffer_init();
int obj_buffer_readable_bytes(obj_buffer_t *buf);
int obj_buffer_writable_bytes(obj_buffer_t *buf);
obj_bool_t obj_buffer_ensure_writable_bytes(obj_buffer_t *buf, int len);
obj_bool_t obj_buffer_can_read_len(obj_buffer_t *buf);
obj_int32_t obj_buffer_read_len_unsafe(obj_buffer_t *buf);
obj_bool_t obj_buffer_can_read_header(obj_buffer_t *buf);
obj_bool_t obj_buffer_append(obj_buffer_t *buf, const void *data, int len);
obj_bool_t obj_buffer_read(obj_buffer_t *buf, int fd, int *saved_errno, int *num);

#endif  /* OBJ_BUFFER_H */