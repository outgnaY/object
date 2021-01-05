#include "obj_core.h"


static obj_size_t obj_sds_len(const obj_sds s);
static obj_size_t obj_sds_avail(const obj_sds s);
static void obj_sds_setlen(obj_sds s, obj_size_t newlen);
static void obj_sds_inclen(obj_sds s, obj_size_t inc);
static obj_size_t obj_sds_alloc(const obj_sds s);
static void obj_sds_setalloc(obj_sds s, obj_size_t newlen);


/* length */
static obj_size_t obj_sds_len(const obj_sds s) {
    unsigned char flags = s[-1];
    switch (flags & OBJ_SDS_TYPE_MASK) {
        case OBJ_SDS_TYPE_5:
            return OBJ_SDS_TYPE_5_LEN(flags);
        case OBJ_SDS_TYPE_8:
            return OBJ_SDS_HEADER(8, s)->len;
        case OBJ_SDS_TYPE_16:
            return OBJ_SDS_HEADER(16, s)->len;
        case OBJ_SDS_TYPE_32:
            return OBJ_SDS_HEADER(32, s)->len;
        case OBJ_SDS_TYPE_64:
            return OBJ_SDS_HEADER(64, s)->len;
    }
    return 0;
}

/* available */
static obj_size_t obj_sds_avail(const obj_sds s) {
    unsigned char flags = s[-1];
    switch (flags & OBJ_SDS_TYPE_MASK) {
        case OBJ_SDS_TYPE_5: {
            return 0;
        }
        case OBJ_SDS_TYPE_8: {
            OBJ_SDS_HEADER_VAR(8, s);
            return sh->alloc - sh->len;
        }
        case OBJ_SDS_TYPE_16: {
            OBJ_SDS_HEADER_VAR(16, s);
            return sh->alloc - sh->len;
        }
        case OBJ_SDS_TYPE_32: {
            OBJ_SDS_HEADER_VAR(32, s);
            return sh->alloc - sh->len;
        }
        case OBJ_SDS_TYPE_64: {
            OBJ_SDS_HEADER_VAR(64, s);
            return sh->alloc - sh->len;
        }
    }
    return 0;
}

/* set length */
static void obj_sds_setlen(obj_sds s, obj_size_t newlen) {
    unsigned char flags = s[-1];
    switch (flags & OBJ_SDS_TYPE_MASK) {
        case OBJ_SDS_TYPE_5: {
            unsigned char *fp = ((unsigned char *)s) - 1;
            *fp = (unsigned char)(OBJ_SDS_TYPE_5 | (newlen << OBJ_SDS_TYPE_BITS));
            break;
        }
        case OBJ_SDS_TYPE_8:
            OBJ_SDS_HEADER(8, s)->len = (obj_uint8_t)newlen;
            break;
        case OBJ_SDS_TYPE_16:
            OBJ_SDS_HEADER(16, s)->len = (obj_uint16_t)newlen;
            break;
        case OBJ_SDS_TYPE_32:
            OBJ_SDS_HEADER(32, s)->len = (obj_uint32_t)newlen;
            break;
        case OBJ_SDS_TYPE_64:
            OBJ_SDS_HEADER(64, s)->len = (obj_uint64_t)newlen;
            break;
    }
}

/* increase length */
static void obj_sds_inclen(obj_sds s, obj_size_t inc) {
    unsigned char flags = s[-1];
    switch (flags & OBJ_SDS_TYPE_MASK) {
        case OBJ_SDS_TYPE_5: {
            unsigned char *fp = ((unsigned char *)s) - 1;
            unsigned char newlen = OBJ_SDS_TYPE_5_LEN(flags) + (unsigned char)inc;
            *fp = OBJ_SDS_TYPE_5 | (newlen << OBJ_SDS_TYPE_BITS);
            break;
        }
        case OBJ_SDS_TYPE_8:
            OBJ_SDS_HEADER(8, s)->len += (obj_uint8_t)inc;
            break;
        case OBJ_SDS_TYPE_16:
            OBJ_SDS_HEADER(16, s)->len += (obj_uint16_t)inc;
            break;
        case OBJ_SDS_TYPE_32:
            OBJ_SDS_HEADER(32, s)->len += (obj_uint32_t)inc;
            break;
        case OBJ_SDS_TYPE_64:
            OBJ_SDS_HEADER(64, s)->len += (obj_uint64_t)inc;
            break;
    }
}

/* alloc size */
static obj_size_t obj_sds_alloc(const obj_sds s) {
    unsigned char flags = s[-1];
    switch (flags & OBJ_SDS_TYPE_MASK) {
        case OBJ_SDS_TYPE_5:
            return OBJ_SDS_TYPE_5_LEN(flags);
        case OBJ_SDS_TYPE_8:
            return OBJ_SDS_HEADER(8, s)->alloc;
        case OBJ_SDS_TYPE_16:
            return OBJ_SDS_HEADER(16, s)->alloc;
        case OBJ_SDS_TYPE_32:
            return OBJ_SDS_HEADER(32, s)->alloc;
        case OBJ_SDS_TYPE_64:
            return OBJ_SDS_HEADER(64, s)->alloc;
    }
    return 0;
}

/* set alloc size */
static void obj_sds_setalloc(obj_sds s, obj_size_t newlen) {
    unsigned char flags = s[-1];
    switch (flags & OBJ_SDS_TYPE_MASK) {
        case OBJ_SDS_TYPE_5:
            break;
        case OBJ_SDS_TYPE_8:
            OBJ_SDS_HEADER(8, s)->alloc = (obj_uint8_t)newlen;
            break;
        case OBJ_SDS_TYPE_16:
            OBJ_SDS_HEADER(16, s)->alloc = (obj_uint16_t)newlen;
            break;
        case OBJ_SDS_TYPE_32:
            OBJ_SDS_HEADER(32, s)->alloc = (obj_uint32_t)newlen;
            break;
        case OBJ_SDS_TYPE_64:
            OBJ_SDS_HEADER(64, s)->alloc = (obj_uint64_t)newlen;
            break;
    }
}

static int obj_sds_header_size(char type) {
    switch (type & OBJ_SDS_TYPE_MASK) {
        case OBJ_SDS_TYPE_5:
            return sizeof(struct obj_sds_header5);
        case OBJ_SDS_TYPE_8:
            return sizeof(struct obj_sds_header8);
        case OBJ_SDS_TYPE_16:
            return sizeof(struct obj_sds_header16);
        case OBJ_SDS_TYPE_32:
            return sizeof(struct obj_sds_header32);
        case OBJ_SDS_TYPE_64:
            return sizeof(struct obj_sds_header64);
    }
    return 0;
}

static char obj_sds_req_type(obj_size_t string_size) {
    if (string_size < 32) {
        return OBJ_SDS_TYPE_5;
    }
    if (string_size < 0xff) {
        return OBJ_SDS_TYPE_8;
    }
    if (string_size < 0xffff) {
        return OBJ_SDS_TYPE_16;
    }
    if (string_size < 0xffffffff) {
        return OBJ_SDS_TYPE_32;
    }
    return OBJ_SDS_TYPE_64;
}

obj_sds obj_sds_newlen(const void *init, obj_size_t initlen) {
    void *sh;
    obj_sds s;
    char type = obj_sds_req_type(initlen);
    /* empty string are usually created in order to append */
    if (type == OBJ_SDS_TYPE_5 && initlen == 0) {
        type = OBJ_SDS_TYPE_8;
    }
    int headerlen = obj_sds_header_size(type);
    unsigned char *fp;
    sh = obj_alloc(headerlen + initlen + 1);
    if (sh == NULL) {
        return NULL;
    }
    if (!init) {
        obj_memset(sh, 0, headerlen + initlen + 1);
    }
    s = (char *)sh + headerlen;
    fp = ((unsigned char *)s) - 1;
    switch (type) {
        case OBJ_SDS_TYPE_5: {
            *fp = type | (initlen << OBJ_SDS_TYPE_BITS);
            break;
        }
        case OBJ_SDS_TYPE_8: {
            OBJ_SDS_HEADER_VAR(8, s);
            sh->len = initlen;
            sh->alloc = initlen;
            *fp = type;
            break;
        }
        case OBJ_SDS_TYPE_16: {
            OBJ_SDS_HEADER_VAR(16, s);
            sh->len = initlen;
            sh->alloc = initlen;
            *fp = type;
            break;
        }
        case OBJ_SDS_TYPE_32: {
            OBJ_SDS_HEADER_VAR(32, s);
            sh->len = initlen;
            sh->alloc = initlen;
            *fp = type;
            break;
        }
        case OBJ_SDS_TYPE_64: {
            OBJ_SDS_HEADER_VAR(64, s);
            sh->len = initlen;
            sh->alloc = initlen;
            *fp = type;
            break;
        }
    }
    if (initlen && init) {
        obj_memcpy(s, init, initlen);
    }
    s[initlen] = '\0';
    return s;
}

obj_sds obj_sds_empty() {
    return obj_sds_newlen("", 0);
}

obj_sds obj_sds_new(const char *init) {
    obj_size_t initlen = (init == NULL) ? 0 : obj_strlen(init);
    return obj_sds_newlen(init, initlen);
}

obj_sds obj_sds_dup(const obj_sds s) {
    return obj_sds_newlen(s, obj_sds_len(s));
}

void obj_sds_free(obj_sds s) {
    if (s == NULL) {
        return;
    }
    obj_free((char *)s - obj_sds_header_size(s[-1]));
}

void obj_sds_updatelen(obj_sds s) {
    int newlen = obj_strlen(s);
    obj_sds_setlen(s, newlen);
}

void obj_sds_clear(obj_sds s) {
    obj_sds_setlen(s, 0);
    s[0] = '\0';
}

obj_sds obj_sds_make_room_for(obj_sds s, obj_size_t addlen) {
    void *sh, *newsh;
    obj_size_t avail = obj_sds_avail(s);
    obj_size_t len, newlen;
    char type, oldtype = s[-1] & OBJ_SDS_TYPE_MASK;
    int headerlen;
    /* if there is enough space, return */
    if (avail >= addlen) {
        return s;
    }
    len = obj_sds_len(s);
    sh = (char *)s - obj_sds_header_size(oldtype);
    newlen = len + addlen;
    if (newlen < OBJ_SDS_MAX_PREALLOC) {
        newlen *= 2;
    } else {
        newlen += OBJ_SDS_MAX_PREALLOC;
    }
    type = obj_sds_req_type(newlen);
    if (type == OBJ_SDS_TYPE_5) {
        type = OBJ_SDS_TYPE_8;
    }
    headerlen = obj_sds_header_size(type);
    if (oldtype == type) {
        newsh = obj_realloc(sh, headerlen + newlen + 1);
        if (newsh == NULL) {
            return NULL;
        }
        s = (char *)newsh + headerlen;
    } else {
        /* header size change */
        newsh = obj_alloc(headerlen + newlen + 1);
        if (newsh == NULL) {
            return NULL;
        }
        obj_memcpy((char *)newsh + headerlen, s, len + 1);
        obj_free(sh);
        s = (char *)newsh + headerlen;
        s[-1] = type;
        obj_sds_setlen(s, len);
    }
    obj_sds_setalloc(s, newlen);
    return s;
}

obj_sds obj_sds_remove_free_space(obj_sds s) {
    void *sh, *newsh;
    char type, oldtype = s[-1] & OBJ_SDS_TYPE_MASK;
    int headerlen;
    obj_size_t len = obj_sds_len(s);
    sh = (char *)s - obj_sds_header_size(oldtype);
    type = obj_sds_req_type(len);
    headerlen = obj_sds_header_size(type);
    if (oldtype == type) {
        newsh = obj_realloc(sh, headerlen + len + 1);
        if (newsh == NULL) {
            return NULL;
        }
        s = (char *)newsh + headerlen;
    } else {
        newsh = obj_alloc(headerlen + len + 1);
        if (newsh == NULL) {
            return NULL;
        }
        obj_memcpy((char *)newsh + headerlen, s, len + 1);
        obj_free(sh);
        s = (char *)newsh + headerlen;
        s[-1] = type;
        obj_sds_setlen(s, len);
    }
    obj_sds_setalloc(s, len);
    return s;
}

/* total alloc size */
obj_size_t obj_sds_alloc_size(obj_sds s) {
    obj_size_t alloc = obj_sds_alloc(s);
    return obj_sds_header_size(s[-1]) + alloc + 1;
}

/* alloc pointer */
void obj_sds_alloc_ptr(obj_sds s) {
    return (void *)(s - obj_sds_header_size(s[-1]));
}

/* usually use after obj_sds_make_room_for */
void obj_sds_incr_len(obj_sds s, int incr) {
    unsigned char flags = s[-1];
    obj_size_t len;
    switch (flags & OBJ_SDS_TYPE_MASK) {
        case OBJ_SDS_TYPE_5: {
            unsigned char *fp = ((unsigned char *)s) - 1;
            unsigned char oldlen = OBJ_SDS_TYPE_5_LEN(flags);
            obj_assert((incr > 0 && oldlen + incr < 32) || (incr < 0 && oldlen >= (unsigned int)(-incr)));
            *fp = OBJ_SDS_TYPE_5 | ((oldlen + incr) << OBJ_SDS_TYPE_BITS);
            len = oldlen + incr;
            break;
        }
        case OBJ_SDS_TYPE_8: {
            OBJ_SDS_HEADER_VAR(8, s);
            obj_assert((incr >= 0 && sh->alloc - sh->len >= incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
            len = (sh->len += incr);
            break;
        }
        case OBJ_SDS_TYPE_16: {
            OBJ_SDS_HEADER_VAR(16, s);
            obj_assert((incr >= 0 && sh->alloc - sh->len >= incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
            len = (sh->len += incr);
            break;
        }
        case OBJ_SDS_TYPE_32: {
            OBJ_SDS_HEADER_VAR(32, s);
            obj_assert((incr >= 0 && sh->alloc - sh->len >= (unsigned int)incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
            len = (sh->len += incr);
            break;
        }
        case OBJ_SDS_TYPE_64: {
            OBJ_SDS_HEADER_VAR(64, s);
            obj_assert((incr >= 0 && sh->alloc - sh->len >= (obj_uint64_t)incr) || (incr < 0 && sh->len >= (obj_uint64_t)(-incr)));
            len = (sh->len += incr);
            break;
        }
    }
    s[len] = '\0';
}

obj_sds obj_sds_catlen(obj_sds s, const void *t, obj_size_t len) {
    obj_size_t curlen = obj_sds_len(s);
    s = obj_sds_make_room_for(s, len);
    if (s == NULL) {
        return NULL;
    }
    obj_memcpy(s + curlen, t, len);
    obj_sds_setlen(s, curlen + len);
    s[curlen + len] = '\0';
    return s;
}

obj_sds obj_sds_cat(obj_sds s, const char *t) {
    return obj_sds_catlen(s, t, obj_strlen(t));
}

obj_sds obj_sds_catsds(obj_sds s, const obj_sds t) {
    return obj_sds_catlen(s, t, obj_sds_len(t));
}

obj_sds obj_sds_cpylen(obj_sds s, const char *t, obj_size_t len) {
    if (obj_sds_alloc(s) < len) {
        s = obj_sds_make_room_for(s, len - obj_sds_len(s));
        if (s == NULL) {
            return NULL;
        }
    }
    obj_memcpy(s, t, len);
    s[len] = '\0';
    obj_sds_setlen(s, len);
    return s;
}

obj_sds obj_sds_cpy(obj_sds s, const char *t) {
    return obj_sds_cpylen(s, t, obj_strlen(t));
}

int obj_sds_ll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    obj_size_t l;
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v);
    if (value < 0) {
        *p++ = '-';
    }
    l = p - s;
    *p = '\0';
    p--;
    /* reverse */
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

int obj_sds_ull2str(char *s, unsigned long long value) {
    char *p, aux;
    obj_size_t l;
    p = s;
    do {
        *p++ = '0' + (value % 10);
        value /= 10;
    } while (value);
    l = p - s;
    *p = '\0';
    p--;
    /* reverse */
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

obj_sds obj_sds_fromlonglong(long long value) {
    char buf[OBJ_SDS_LLSTR_SIZE];
    int len = obj_sds_ll2str(buf, value);
    return obj_sds_newlen(buf, len);
}

obj_sds obj_sds_catvprintf(obj_sds s, const char *fmt, va_list ap) {
    va_list cpy;
    char staticbuf[1024], *buf = staticbuf, *t;
    obj_size_t buflen = obj_strlen(fmt) * 2;
    if (buflen > sizeof(staticbuf)) {
        buf = obj_alloc(buflen);
        if (buf == NULL) {
            return NULL;
        }
    } else {
        buflen = sizeof(staticbuf);
    }
    while (1) {
        buf[buflen - 2] = '\0';
        va_copy(cpy, ap);
        vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);

    }
    t = obj_sds_cat(s, buf);
    if (buf != staticbuf) {
        obj_free(buf);
    }
    return t;
}

obj_sds obj_sds_catprintf(obj_sds s, const char *fmt, ...) {
    va_list ap;
    char *t;
    va_start(ap, fmt);
    t = obj_sds_catvprintf(s, fmt, ap);
    va_end(ap);
    return t;
}

/**
 * %s - C string
 * %S - SDS string
 * %i - signed int
 * %I - 64 bit signed int
 * %u - unsigned int
 * %U - 64 bit unsigned int
 * %% - %
 */
obj_sds obj_sds_catfmt(obj_sds s, const char *fmt, ...) {
    const char *f = fmt;
    int i;
    va_list ap;
    va_start(ap, fmt);
    i = obj_sds_len(s);
    while (*f) {
        char next, *str;
        obj_size_t l;
        long long num;
        unsigned long long unum;
        if (obj_sds_avail(s) == 0) {
            s = obj_sds_make_room_for(s, 1);
            if (s == NULL) {
                goto fmt_error;
            }
        }
        switch (*f) {
            case '%':
                next = *(f + 1);
                f++;
                switch (next) {
                    case 's':
                    case 'S':
                        str = va_arg(ap, char *);
                        l = (next == 's') ? obj_strlen(str) : obj_sds_len(str);
                        if (obj_sds_avail(s) < l) {
                            s = obj_sds_make_room_for(s, l);
                            if (s == NULL) {
                                goto fmt_error;
                            }
                        }
                        obj_memcpy(s + i, str, l);
                        obj_sds_inclen(s, l);
                        i += l;
                        break;
                    case 'i':
                    case 'I':
                        if (next == 'i') {
                            num = va_arg(ap, int);
                        } else {
                            num = va_arg(ap, long long);
                        }
                        {
                            char buf[OBJ_SDS_LLSTR_SIZE];
                            l = obj_sds_ll2str(buf, num);
                            if (obj_sds_avail(s) < l) {
                                s = obj_sds_make_room_for(s, l);
                                if (s == NULL) {
                                    goto fmt_error;
                                }
                            }
                            obj_memcpy(s + i, buf, l);
                            obj_sds_inclen(s, l);
                            i += l;
                        }
                        break;
                    case 'u':
                    case 'U':
                        if (next == 'u') {
                            unum = va_arg(ap, unsigned int);
                        } else {
                            unum = va_arg(ap, unsigned long long);
                        }
                        {
                            char buf[OBJ_SDS_LLSTR_SIZE];
                            l = obj_sds_ull2str(buf, num);
                            if (obj_sds_avail(s) < l) {
                                s = obj_sds_make_room_for(s, l);
                                if (s == NULL) {
                                    goto fmt_error;
                                }
                            }
                            obj_memcpy(s + i, buf, l);
                            obj_sds_inclen(s, l);
                            i += l;
                        }
                        break;
                    default:
                        /* handle %% */
                        s[i++] = next;
                        obj_sds_inclen(s, 1);
                        break;
                }
                break;
            default:
                s[i++] = *f;
                obj_sds_inclen(s, 1);
                break;
        }
        f++;
    }
    va_end(ap);
    s[i] = '\0';
    return s;
fmt_error:
    va_end(ap);
    return NULL;
}

void obj_sds_range(obj_sds s, int start, int end) {
    obj_size_t newlen, len = obj_sds_len(s);
    if (len == 0) {
        return;
    }
    if (start < 0) {
        start = len + start;
        if (start < 0) {
            start = 0;
        }
    }
    if (end < 0) {
        end = len + end;
        if (end < 0) {
            end = 0;
        }
    }
    newlen = (start > end) ? 0 : (end - start + 1);
    if (newlen != 0) {

    } else {
        start = 0;
    }
    if (start && newlen) {
        obj_memmove(s, s + start, newlen);
    }
    s[newlen] = '\0';
    obj_sds_setlen(s, newlen);
}

void obj_sds_tolower(obj_sds s) {
    int len = obj_sds_len(s);
    int i;
    for (i = 0; i < len; i++) {
        s[i] = obj_tolower(s[i]);
    }
}

void obj_sds_toupper(obj_sds s) {
    int len = obj_sds_len(s);
    int i;
    for (i = 0; i < len; i++) {
        s[i] = obj_toupper(s[i]);
    }
}

int obj_sds_cmp(const obj_sds s1, const obj_sds s2) {
    obj_size_t l1, l2, minlen;
    int cmp;
    l1 = obj_sds_len(s1);
    l2 = obj_sds_len(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = obj_memcmp(s1, s2, minlen);
    if (cmp == 0) {
        return l1 - l2;
    }
    return cmp;
}