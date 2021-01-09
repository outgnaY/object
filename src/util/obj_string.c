#include "obj_core.h"

static void obj_utf8_get_sequence(const char *utf8, obj_uint8_t *seq_length, obj_uint8_t *first_mask) {
    unsigned char c = *(const unsigned char *)utf8;
    obj_uint8_t m;
    obj_uint8_t n;
    if ((c & 0x80) == 0) {
        n = 1;
        m = 0x7f;
    } else if ((c & 0xe0) == 0xc0) {
        n = 2;
        m = 0x1f;
    } else if ((c & 0xf0) == 0xe0) {
        n = 3;
        m = 0x0f;
    } else if ((c & 0xf8) == 0xf0) {
        n = 4;
        m = 0x07;
    } else {
        n = 0;
        m = 0;
    }
    *seq_length = n;
    *first_mask = m;
}

void obj_print_utf8_string(const char *utf8, obj_size_t len) {
    int i;
    for (i = 0; i < len; i++) {
        /* printf("%02x ", utf8[i]); */
        printf("%02x ", utf8[i]);
    }
    printf("\n");
}

obj_bool_t obj_validate_utf8_string(const char *utf8, obj_size_t utf8_len, obj_bool_t allow_null) {
    obj_uint8_t first_mask;
    obj_uint8_t seq_length;
    obj_uint32_t c;
    unsigned i;
    unsigned j;
    for (i = 0; i < utf8_len; i += seq_length) {
        obj_utf8_get_sequence(&utf8[i], &seq_length, &first_mask);
        if (!seq_length) {
            return false;
        }
        if ((utf8_len - i) < seq_length) {
            return false;
        }
        c = utf8[i] & first_mask;
        for (j = i + 1; j < (i + seq_length); j++) {
            c = (c << 6) | (utf8[j] & 0x3f);
            if ((utf8[j] & 0xc0) != 0x80) {
                return false;
            }
        }
        
        if (!allow_null) {
            for (j = 0; j < seq_length; j++) {
                if (((i + j) > utf8_len) || !utf8[i + j]) {
                    return false;
                }
            }
        }
        if (c > 0x0010ffff) {
            return false;
        }
        if ((c & 0xfffff800) == 0xd800) {
            return false;
        }
        switch (seq_length) {
            case 1:
                if (c <= 0x007f) {
                    continue;
                }
                return false;
            case 2:
                if ((c >= 0x0080) && (c <= 0x07ff)) {
                    continue;
                } else if (c == 0) {
                    if (!allow_null) {
                        return false;
                    }
                    continue;
                }
                return false;
            case 3:
                if (((c >= 0x0800) && (c <= 0x0fff)) || ((c >= 0x1000) && (c <= 0xffff))) {
                    continue;
                }
                return false;
            case 4:
                if (((c >= 0x10000) && (c <= 0x3ffff)) || ((c >= 0x40000) && (c <= 0xfffff)) || ((c >= 0x100000) && (c <= 0x10ffff))) {
                    continue;
                }
                return false;
            default:
                return false;
        }
    }
    return true;
}