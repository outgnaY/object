#ifndef OBJ_V1_INDEX_H
#define OBJ_V1_INDEX_H

#include "obj_core.h"

typedef struct obj_v1_index_s obj_v1_index_t;
typedef struct obj_v1_index_cursor_s obj_v1_index_cursor_t;

/* index */
struct obj_v1_index_s {
    obj_skiplist_t *skiplist;
};

/* index cursor */
struct obj_v1_index_cursor_s {

};


#endif  /* OBJ_V1_INDEX_H */