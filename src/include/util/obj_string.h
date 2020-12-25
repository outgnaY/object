#ifndef OBJ_STRING_H
#define OBJ_STRING_H

#include "obj_common.h"

#define obj_memzero(buf, n)     (void) memset(buf, 0, n)
#define obj_memset(buf, c, n)   (void) memset(buf, c, n)
#define obj_memcpy(dst, src, n) (void) memcpy(dst, src, n)
#define obj_memchr(buf, c, n)   memchr(buf, c, n)

#define obj_strlen(s)   strlen(s)
#define obj_strcat(dst, src)    strcat(dst, src)
#define obj_strncmp(s1, s2, n) strncmp((const char *)s1, (const char *)s2, n)
#define obj_strcmp(s1, s2) strcmp((const char *)s1, (const char *)s2)

#endif  /* OBJ_STRING_H */