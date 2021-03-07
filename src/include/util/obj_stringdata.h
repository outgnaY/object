#ifndef OBJ_STRINGDATA_H
#define OBJ_STRINGDATA_H

#include "obj_core.h"

typedef struct obj_stringdata_s obj_stringdata_t;
typedef struct obj_namespace_string_s obj_namespace_string_t;

struct obj_stringdata_s {
    char *data;
    int size;
};

struct obj_namespace_string_s {
    obj_stringdata_t str;
    int dot_index;
};

#define obj_stringdata_invalid -1

void obj_stringdata_init(obj_stringdata_t *stringdata, char *data, int size);
obj_stringdata_t obj_stringdata_create_with_size(char *data, int size);
obj_stringdata_t obj_stringdata_create(char *data);
void obj_stringdata_destroy(obj_stringdata_t *stringdata);
obj_stringdata_t obj_stringdata_copy_with_size(char *data, int size);
obj_stringdata_t obj_stringdata_copy(char *data);
obj_stringdata_t obj_stringdata_copy_stringdata(obj_stringdata_t *stringdata);
int obj_stringdata_compare(obj_stringdata_t *stringdata1, obj_stringdata_t *stringdata2);
int obj_stringdata_find(obj_stringdata_t *stringdata, char c, int from_pos);
int obj_stringdata_findstr(obj_stringdata_t *stringdata, obj_stringdata_t *target, int from_pos);
obj_stringdata_t obj_stringdata_substr(obj_stringdata_t *stringdata, int pos, int n);
obj_bool_t obj_stringdata_startwith(obj_stringdata_t *stringdata, obj_stringdata_t *prefix);

obj_namespace_string_t obj_namespace_string_create(obj_stringdata_t *stringdata);
obj_stringdata_t obj_namespace_string_get_coll(obj_namespace_string_t *nss);
obj_stringdata_t obj_namespace_string_get_db(obj_namespace_string_t *nss);

#endif  /* OBJ_STRINGDATA_H */