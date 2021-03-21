#include "obj_core.h"

/* little endian */

static void obj_buffer_make_room(obj_buffer_t *buf, int len);

/* debug */
void obj_buffer_dump(obj_buffer_t *buf) {
    int i;
    printf("read_index: %d\n", buf->read_index);
    printf("write_inex: %d\n", buf->write_index);
    for (i = buf->read_index; i < buf->write_index; i++) {
        printf("%02x ", buf->buf[i]);
    }
    printf("\n");
}

/* ensure we have at least len bytes space to write */
static void obj_buffer_make_room(obj_buffer_t *buf, int len) {
    if (obj_buffer_writable_bytes(buf) + buf->read_index < len) {
        /* check */
        if (buf->write_index + len > OBJ_BUFFER_MAX_SIZE) {
            return;
        }
        /* reallocate space */
        buf->buf = obj_realloc(buf->buf, buf->write_index + len);
        buf->buf_size = buf->write_index + len;
    } else {
        /* move data to start */
        int readable = obj_buffer_readable_bytes(buf);
        obj_memmove(buf->buf, buf->buf + buf->read_index, readable);
        buf->read_index = 0;
        buf->v_read_index = 0; 
        buf->write_index = readable;
    }
}

/* create a buffer */
obj_buffer_t *obj_buffer_create() {
    obj_buffer_t *buffer;
    buffer = obj_alloc(sizeof(obj_buffer_t));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->buf = obj_alloc(OBJ_BUFFER_INIT_SIZE);
    buffer->buf_size = OBJ_BUFFER_INIT_SIZE;
    buffer->v_read_index = 0;
    buffer->read_index = 0;
    buffer->write_index = 0;
    return buffer;
}

/* create with size */
obj_buffer_t *obj_buffer_create_with_size(int size) {
    if (size > OBJ_BUFFER_MAX_SIZE) {
        return NULL;
    }
    obj_buffer_t *buffer;
    buffer = obj_alloc(sizeof(obj_buffer_t));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->buf = obj_alloc(size);
    buffer->buf_size = size;
    buffer->v_read_index = 0;
    buffer->read_index = 0;
    buffer->write_index = 0;
    return buffer;
}

/* free a buffer */
void obj_buffer_destroy(obj_buffer_t *buf) {
    obj_assert(buf != NULL);
    if (buf->buf != NULL) {
        obj_free(buf->buf);
    }
    obj_free(buf);
}

/* readable bytes of buffer */
int obj_buffer_readable_bytes(obj_buffer_t *buf) {
    return buf->write_index - buf->read_index;
}

int obj_buffer_v_readable_bytes(obj_buffer_t *buf) {
    return buf->write_index - buf->v_read_index;
}

/* writable bytes of buffer */
int obj_buffer_writable_bytes(obj_buffer_t *buf) {
    return buf->buf_size - buf->write_index;
}

/* ensure we have at least len bytes space to write */
void obj_buffer_ensure_writable_bytes(obj_buffer_t *buf, int len) {
    if (obj_buffer_writable_bytes(buf) < len) {
        /* make space */
        obj_buffer_make_room(buf, len);
    }
}

/* test if we can read message length */
obj_bool_t obj_buffer_can_read_int32(obj_buffer_t *buf) {
    return obj_buffer_readable_bytes(buf) >= sizeof(obj_int32_t);
}

obj_bool_t obj_buffer_v_can_read_int32(obj_buffer_t *buf) {
    return obj_buffer_v_readable_bytes(buf) >= sizeof(obj_int32_t);
}

obj_int32_t obj_buffer_read_int32_unsafe(obj_buffer_t *buf) {
    obj_int32_t len_le;
    len_le = (obj_int32_t)(*((obj_int32_t *)(buf->buf + buf->read_index)));
    obj_buffer_retrieve(buf, sizeof(obj_int32_t));
    return obj_int32_from_le(len_le);
}

obj_int32_t obj_buffer_v_read_int32_unsafe(obj_buffer_t *buf) {
    obj_int32_t len_le;
    len_le = (obj_int32_t)(*((obj_int32_t *)(buf->buf + buf->v_read_index)));
    obj_buffer_v_retrieve(buf, sizeof(obj_int32_t));
    return obj_int32_from_le(len_le);
}

obj_int32_t obj_buffer_v_peek_int32_unsafe(obj_buffer_t *buf) {
    obj_int32_t len_le;
    len_le = (obj_int32_t)(*((obj_int32_t *)(buf->buf + buf->v_read_index)));
    return obj_int32_from_le(len_le);
}

obj_msg_header_t obj_buffer_v_peek_msg_header_unsafe(obj_buffer_t *buf) {
    obj_int32_t len_le;
    obj_int32_t op_le;
    len_le = (obj_int32_t)(*((obj_int32_t *)(buf->buf + buf->v_read_index)));
    op_le = (obj_int32_t)(*((obj_int32_t *)(buf->buf + buf->v_read_index + sizeof(obj_int32_t))));
    obj_msg_header_t header;
    header.len = obj_int32_to_le(len_le);
    header.opCode = obj_int32_to_le(op_le);
    return header;
}


/* read cstring */
char *obj_buffer_v_read_string_unsafe(obj_buffer_t *buf, int *len) {
    int old_index = buf->read_index;
    /* TODO probably unsafe? */
    *len = (int)obj_strlen(buf->buf + buf->v_read_index);
    /* do not allow length == 0 */
    if (*len == 0 || *len > obj_buffer_v_readable_bytes(buf)) {
        return NULL;
    }
    obj_buffer_v_retrieve(buf, *len + 1);
    return buf->buf + old_index;
}

/* read bson and validate */
obj_bson_t *obj_buffer_v_read_bson_unsafe(obj_buffer_t *buf, obj_int32_t len) {
    obj_bson_t *bson;
    obj_bool_t validate;
    bson = obj_bson_create_with_data(buf->buf + buf->v_read_index, len);
    if (bson == NULL) {
        return NULL;
    }
    obj_buffer_v_retrieve(buf, len);
    validate = obj_bson_visit_validate_visit(bson, OBJ_BSON_VALIDATE_FLAG_NONE);
    if (!validate) {
        /* invalid bson */
        obj_bson_destroy(bson);
        return NULL;
    }
    return bson;
}

void obj_buffer_retrieve(obj_buffer_t *buf, int len) {
    int readable = obj_buffer_readable_bytes(buf);
    obj_assert(len <= readable);
    if (len < readable) {
        buf->read_index += len;
    } else {
        buf->read_index = buf->write_index = 0;
    }
}

void obj_buffer_v_reset(obj_buffer_t *buf) {
    buf->v_read_index = buf->read_index;
}

/* move virtual read index */
void obj_buffer_v_retrieve(obj_buffer_t *buf, int len) {
    obj_assert(len <= obj_buffer_v_readable_bytes(buf));
    buf->v_read_index += len;
}

/* append */
void obj_buffer_append(obj_buffer_t *buf, void *data, int len) {
    obj_buffer_ensure_writable_bytes(buf, len);
    /* copy data */
    obj_memcpy(buf->buf + buf->write_index, data, len);
    buf->write_index += len;
}

/* append int32 */
void obj_buffer_append_int32(obj_buffer_t *buf, obj_int32_t data) {
    obj_int32_t data_le = obj_int32_to_le(data);
    obj_buffer_append(buf, &data_le, sizeof(obj_int32_t));
}

/* append int64 */
void obj_buffer_append_int64(obj_buffer_t *buf, obj_int64_t data) {
    obj_int64_t data_le = obj_int64_to_le(data);
    obj_buffer_append(buf, &data_le, sizeof(obj_int64_t));
}

/* append bson */
void obj_buffer_append_bson(obj_buffer_t *buf, obj_bson_t *bson) {
    obj_buffer_append(buf, bson->data, bson->len);
}


void obj_buffer_read_fd(obj_buffer_t *buf, int fd, int *saved_errno, int *num) {
    char extrabuf[65536];
    struct iovec vec[2];
    int writable = obj_buffer_writable_bytes(buf);
    int iovcnt;
    int n;
    /* use stack space */
    vec[0].iov_base = buf->buf + buf->write_index;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    n = (int)readv(fd, vec, iovcnt);
    *num = n;
    if (n < 0) {
        *saved_errno = errno;
    } else if (n <= writable) {
        /* stack space not used */
        buf->write_index += n;
    } else {
        /* stack space used */
        buf->write_index = buf->buf_size;
        obj_buffer_append(buf, extrabuf, n - writable);
    }
}

/* write to fd */
int obj_buffer_write_fd(obj_buffer_t *buf, int fd, int *saved_errno) {
    int n;
    int readable = obj_buffer_readable_bytes(buf);
    n = (int)write(fd, buf->buf + buf->read_index, readable);
    if (n < 0) {
        *saved_errno = errno;
    } else if (n < readable) {
        buf->read_index += n;
    } else {
        buf->read_index = buf->write_index = 0;
    }
    return n;
}