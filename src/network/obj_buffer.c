#include "obj_core.h"

static obj_bool_t obj_buffer_make_room(obj_buffer_t *buf, int len);

/* ensure we have at least len bytes space to write */
static obj_bool_t obj_buffer_make_room(obj_buffer_t *buf, int len) {
    if (obj_buffer_writable_bytes(buf) + buf->read_index < len) {
        /* check */
        if (buf->write_index + len > OBJ_BUFFER_MAX_SIZE) {
            return false;
        }
        /* reallocate space */
        buf->buf = obj_realloc(buf->buf, buf->write_index + len);
        if (buf->buf == NULL) {
            return false;
        }
        buf->buf_size = buf->write_index + len;
        return true;
    } else {
        /* move data to start */
        int readable = obj_buffer_readable_bytes(buf);
        obj_memmove(buf->buf, buf->buf + buf->read_index, readable);
        buf->read_index = 0;
        buf->write_index = readable;
        return true;
    }
}

/* init a buffer */
obj_buffer_t *obj_buffer_init() {
    obj_buffer_t *buffer;
    buffer = obj_alloc(sizeof(obj_buffer_t));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->buf = obj_alloc(OBJ_BUFFER_INIT_SIZE);
    if (buffer->buf == NULL) {
        obj_free(buffer);
        return NULL;
    }
    buffer->buf_size = OBJ_BUFFER_INIT_SIZE;
    buffer->read_index = 0;
    buffer->write_index = 0;
    return buffer;
}


/* readable bytes of buffer */
int obj_buffer_readable_bytes(obj_buffer_t *buf) {
    return buf->write_index - buf->read_index;
}

/* writable bytes of buffer */
int obj_buffer_writable_bytes(obj_buffer_t *buf) {
    return buf->buf_size - buf->write_index;
}

/* ensure we have at least len bytes space to write */
obj_bool_t obj_buffer_ensure_writable_bytes(obj_buffer_t *buf, int len) {
    if (obj_buffer_writable_bytes(buf < len)) {
        /* make space */
        return obj_buffer_make_room(buf, len);
    }
}

/* test if we can read message length */
obj_bool_t obj_buffer_can_read_len(obj_buffer_t *buf) {
    return obj_buffer_readable_bytes(buf) >= sizeof(obj_int32_t);
}

/* get message length */
obj_int32_t obj_buffer_read_len_unsafe(obj_buffer_t *buf) {
    obj_assert(obj_buffer_can_read_len(buf));
    obj_int32_t len_le;
    len_le = (obj_int32_t)(*((obj_int32_t *)(buf->buf + buf->read_index)));
    return obj_int32_from_le(len_le);
}

/* test if we can read message header */
obj_bool_t obj_buffer_can_read_header(obj_buffer_t *buf) {
    return obj_buffer_readable_bytes(buf) >= sizeof(obj_msg_header_t);
}

/* append */
obj_bool_t obj_buffer_append(obj_buffer_t *buf, const void *data, int len) {
    obj_bool_t res;
    res = obj_buffer_ensure_writable_bytes(buf, len);
    if (!res) {
        return false;
    }
    /* copy data */
    obj_memcpy(buf->buf + buf->write_index, data, len);
    return true;
}

/* false: out of memory */
obj_bool_t obj_buffer_read(obj_buffer_t *buf, int fd, int *saved_errno, int *num) {
    char extrabuf[65536];
    struct iovec vec[2];
    int writable = obj_buffer_writable_bytes(buf);
    int iovcnt;
    int n;
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
        if (!obj_buffer_append(buf, extrabuf, n - writable)) {
            return false;
        }
    }
    return true;
}


