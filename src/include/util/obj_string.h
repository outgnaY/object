#ifndef OBJ_STRING_H
#define OBJ_STRING_H

#include "obj_core.h"

#define obj_memzero(buf, n) (void) memset(buf, 0, n)
#define obj_memset(buf, c, n) (void) memset(buf, c, n)
#define obj_memcpy(dst, src, n) (void) memcpy(dst, src, n)
#define obj_memchr(buf, c, n) memchr(buf, c, n)
#define obj_memcmp(s1, s2, n) memcmp(s1, s2, n)
#define obj_memmove(dst, src, n) (void) memmove(dst, src, n)

#define obj_strlen(s)   strlen(s)
#define obj_strcpy(dst, src) strcpy(dst, src);
#define obj_strcat(dst, src)    strcat(dst, src)
#define obj_strncmp(s1, s2, n) strncmp((const char *)s1, (const char *)s2, n)
#define obj_strcmp(s1, s2) strcmp((const char *)s1, (const char *)s2)
#define obj_strchr(str, c) strchr((const char *)str, c)

#define obj_tolower(c) tolower(c)
#define obj_toupper(c) toupper(c)

void obj_print_utf8_string(const char *utf8, obj_size_t len);
obj_bool_t obj_validate_utf8_string(const char *utf8, obj_size_t utf8_len, obj_bool_t allow_null);

#endif  /* OBJ_STRING_H */