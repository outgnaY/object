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

#define OBJ_BUFFER_MAX_SIZE (1 << 26)     /* 64MB */

void obj_buffer_dump(obj_buffer_t *buf);
obj_buffer_t *obj_buffer_create();
obj_buffer_t *obj_buffer_create_with_size(int size);
void obj_buffer_destroy(obj_buffer_t *buf);
int obj_buffer_readable_bytes(obj_buffer_t *buf);
int obj_buffer_writable_bytes(obj_buffer_t *buf);
void obj_buffer_ensure_writable_bytes(obj_buffer_t *buf, int len);
obj_bool_t obj_buffer_can_read_int32(obj_buffer_t *buf);
obj_int32_t obj_buffer_read_int32_unsafe(obj_buffer_t *buf);
obj_int32_t obj_buffer_peek_int32_unsafe(obj_buffer_t *buf);
obj_bson_t *obj_buffer_read_bson_unsafe(obj_buffer_t *buf, obj_int32_t len);
void obj_buffer_retrieve(obj_buffer_t *buf, int len);
void obj_buffer_append(obj_buffer_t *buf, void *data, int len);
void obj_buffer_append_int32(obj_buffer_t *buf, obj_int32_t data);
void obj_buffer_append_int64(obj_buffer_t *buf, obj_int64_t data);
void obj_buffer_append_bson(obj_buffer_t *buf, obj_bson_t *bson);
void obj_buffer_read_fd(obj_buffer_t *buf, int fd, int *saved_errno, int *num);
int obj_buffer_write_fd(obj_buffer_t *buf, int fd, int *saved_errno);

#endif  /* OBJ_BUFFER_H */