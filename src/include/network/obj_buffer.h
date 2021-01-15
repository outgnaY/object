#ifndef OBJ_BUFFER_H
#define OBJ_BUFFER_H

#include "obj_core.h"

typedef struct obj_buffer_s obj_buffer_t;

/* buffer hold by connection */
struct obj_buffer_s {
    obj_uint8_t *buf;
    int buf_size;
    int v_read_index;                     /* virtual index, move during command parsing */
    int read_index;
    int write_index;
};

#define OBJ_BUFFER_INIT_SIZE (1 << 12)    /* 4KB */

#define OBJ_BUFFER_MAX_SIZE (1 << 26)     /* 64MB */

void obj_buffer_dump(obj_buffer_t *buf);
obj_buffer_t *obj_buffer_init();
obj_buffer_t *obj_buffer_init_with_size(int size);
void obj_buffer_destroy(obj_buffer_t *buf);
int obj_buffer_readable_bytes(obj_buffer_t *buf);
int obj_buffer_v_readable_bytes(obj_buffer_t *buf);
int obj_buffer_writable_bytes(obj_buffer_t *buf);
obj_bool_t obj_buffer_ensure_writable_bytes(obj_buffer_t *buf, int len);
obj_bool_t obj_buffer_can_read_int32(obj_buffer_t *buf);
obj_bool_t obj_buffer_v_can_read_int32(obj_buffer_t *buf);
obj_int32_t obj_buffer_read_int32_unsafe(obj_buffer_t *buf);
obj_int32_t obj_buffer_v_read_int32_unsafe(obj_buffer_t *buf);
obj_int32_t obj_buffer_v_peek_int32_unsafe(obj_buffer_t *buf);
obj_msg_header_t obj_buffer_v_peek_msg_header_unsafe(obj_buffer_t *buf);
char *obj_buffer_v_read_string_unsafe(obj_buffer_t *buf, int *len);
obj_bson_t *obj_buffer_v_read_bson_unsafe(obj_buffer_t *buf, obj_int32_t len);
void obj_buffer_retrieve(obj_buffer_t *buf, int len);
void obj_buffer_v_init(obj_buffer_t *buf);
void obj_buffer_v_retrieve(obj_buffer_t *buf, int len);
obj_bool_t obj_buffer_append(obj_buffer_t *buf, const void *data, int len);
obj_bool_t obj_buffer_append_int32(obj_buffer_t *buf, obj_int32_t data);
obj_bool_t obj_buffer_append_int64(obj_buffer_t *buf, obj_int64_t data);
obj_bool_t obj_buffer_append_bson(obj_buffer_t *buf, obj_bson_t *bson);
obj_bool_t obj_buffer_read_fd(obj_buffer_t *buf, int fd, int *saved_errno, int *num);
int obj_buffer_write_fd(obj_buffer_t *buf, int fd, int *saved_errno);

#endif  /* OBJ_BUFFER_H */