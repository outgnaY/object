#ifndef OBJ_EXEC_WORKING_SET_H
#define OBJ_EXEC_WORKING_SET_H

#include "obj_core.h"

/* working set */

typedef int obj_exec_working_set_id_t;
typedef struct obj_exec_working_set_s obj_exec_working_set_t;
typedef struct obj_exec_working_set_member_s obj_exec_working_set_member_t;
typedef struct obj_exec_working_set_member_holder_s obj_exec_working_set_member_holder_t;


/* working set */
struct obj_exec_working_set_s {
    /* freelist */
    obj_exec_working_set_id_t freelist;
    /* current active [member_holder] */
    obj_array_t data;
};

/* wrapper of working set member */
struct obj_exec_working_set_member_holder_s {
    /* if in freelist, return next element in freelist; else return self */
    obj_exec_working_set_id_t next_free_or_self;
    obj_exec_working_set_member_t *member;
};


/* member of working set */
struct obj_exec_working_set_member_s {
    /* record */
    obj_record_t *record;
};

#define OBJ_EXEC_WORKING_SET_INVALID_ID -1

obj_exec_working_set_t *obj_exec_working_set_create();
void obj_exec_working_set_destroy(obj_exec_working_set_t *working_set);
void obj_exec_working_set_clear(obj_exec_working_set_t *working_set);
obj_exec_working_set_id_t obj_exec_working_set_allocate(obj_exec_working_set_t *working_set);
obj_exec_working_set_member_t *obj_exec_working_set_get(obj_exec_working_set_t *working_set, obj_exec_working_set_id_t id);
void obj_exec_working_set_free(obj_exec_working_set_t *working_set, obj_exec_working_set_id_t id);



#endif  /* OBJ_EXEC_WORKING_SET_H */