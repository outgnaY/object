#include "obj_core.h"

char *g_empty_str = "";

/* unsafe string */

inline void obj_stringdata_init(obj_stringdata_t *stringdata, char *data, int size) {
    stringdata->data = data;
    stringdata->size = size;
}

obj_stringdata_t obj_stringdata_create_with_size(char *data, int size) {
    obj_stringdata_t stringdata;
    obj_stringdata_init(&stringdata, data, size);
    return stringdata;
}

obj_stringdata_t obj_stringdata_create(char *data) {
    return obj_stringdata_create_with_size(data, obj_strlen(data));
}

/* destroy a copied stringdata */
void obj_stringdata_destroy(obj_stringdata_t *stringdata) {
    obj_free(stringdata->data);
}

obj_stringdata_t obj_stringdata_copy_with_size(char *data, int size) {
    obj_stringdata_t stringdata;
    stringdata.data = NULL;
    stringdata.size = 0;
    char *copy_str = obj_alloc(size + 1);
    if (copy_str == NULL) {
        return stringdata;
    }
    obj_memcpy(copy_str, data, size);
    copy_str[size] = '\0';
    stringdata.data = copy_str;
    stringdata.size = size;
    return stringdata;
}

obj_stringdata_t obj_stringdata_copy(char *data) {
    return obj_stringdata_copy_with_size(data, obj_strlen(data));
}

obj_stringdata_t obj_stringdata_copy_stringdata(obj_stringdata_t *stringdata) {
    return obj_stringdata_copy_with_size(stringdata->data, stringdata->size);
}

int obj_stringdata_compare(obj_stringdata_t *stringdata1, obj_stringdata_t *stringdata2) {
    obj_assert(stringdata1->data);
    obj_assert(stringdata2->data);
    int res = 0;
    int min_size = stringdata1->size < stringdata2->size ? stringdata1->size : stringdata2->size;
    /*
    if (stringdata1->data && stringdata2->data) {
        res = obj_memcmp(stringdata1->data, stringdata2->data, min_size);
    }
    */
    res = obj_memcmp(stringdata1->data, stringdata2->data, min_size);
    if (res != 0) {
        return res > 0 ? 1 : -1;
    }
    if (stringdata1->size == stringdata2->size) {
        return 0;
    }
    return stringdata1->size > stringdata2->size ? 1 : -1;
}

int obj_stringdata_find(obj_stringdata_t *stringdata, char c, int from_pos) {
    if (from_pos >= stringdata->size) {
        return obj_stringdata_invalid;
    }
    void *x = obj_memchr(stringdata->data + from_pos, c, stringdata->size - from_pos);
    if (x == NULL) {
        return obj_stringdata_invalid;
    }
    return ((char *)x - stringdata->data);
}

int obj_stringdata_findstr(obj_stringdata_t *stringdata, obj_stringdata_t *target, int from_pos) {
    if (target->size == 0) {
        return 0;
    } else if (target->size > stringdata->size) {
        return obj_stringdata_invalid;
    }
    if (from_pos > stringdata->size) {
        return obj_stringdata_invalid;
    }
    int mx = stringdata->size - target->size;
    int i;
    for (i = 0; i <= mx; i++) {
        if (obj_memcmp(stringdata->data + i, target->data, target->size) == 0) {
            return i;
        }
    }
    return obj_stringdata_invalid;

}

obj_stringdata_t obj_stringdata_substr(obj_stringdata_t *stringdata, int pos, int n) {
    obj_assert(pos <= stringdata->size);
    if (n > stringdata->size - pos) {
        n = stringdata->size - pos;
    }
    return obj_stringdata_create_with_size(stringdata->data + pos, n);
}

obj_bool_t obj_stringdata_startwith(obj_stringdata_t *stringdata, obj_stringdata_t *prefix) {
    obj_stringdata_t substr = obj_stringdata_substr(stringdata, 0, prefix->size);
    return obj_stringdata_compare(&substr, prefix) == 0;
}

/*
obj_namespace_string_t obj_namespace_string_create(obj_stringdata_t *stringdata) {

}
*/

obj_stringdata_t obj_namespace_string_get_coll(obj_namespace_string_t *nss) {
    if (nss->dot_index == obj_stringdata_invalid) {
        return obj_stringdata_create_with_size(g_empty_str, 0);
    } else {
        return obj_stringdata_create_with_size(nss->str.data + nss->dot_index + 1, nss->str.size - 1 - nss->dot_index);
    }
}

obj_stringdata_t obj_namespace_string_get_db(obj_namespace_string_t *nss) {
    if (nss->dot_index == obj_stringdata_invalid) {
        return nss->str;
    } else {
        return obj_stringdata_create_with_size(nss->str.data, nss->dot_index);
    }
}


