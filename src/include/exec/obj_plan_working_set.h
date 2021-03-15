#ifndef OBJ_PLAN_WORKING_SET_H
#define OBJ_PLAN_WORKING_SET_H

#include "obj_core.h"

/* working set */

typedef int obj_plan_working_set_id_t;
typedef struct obj_plan_working_set_s obj_plan_working_set_t;
typedef struct obj_plan_working_set_member_s obj_plan_working_set_member_t;
typedef struct obj_plan_working_set_member_holder_s obj_plan_working_set_member_holder_t;


/* working set */
struct obj_plan_working_set_s {
    /* freelist */
    obj_plan_working_set_id_t freelist;
    /* current active */
    obj_array_t data;
};

/* wrapper of working set member */
struct obj_plan_working_set_member_holder_s {
    /* if in freelist, return next element in freelist; else return self */
    obj_plan_working_set_id_t next_free_or_self;
    obj_plan_working_set_member_t *member;
};

enum obj_plan_working_set_member_state {
    OBJ_PLAN_WORKING_SET_MEMBER_STATE_INVALID,

};

/* member of working set */
struct obj_plan_working_set_member_s {

};

#define OBJ_PLAN_WORKING_SET_INVALID_ID -1

#endif  /* OBJ_PLAN_WORKING_SET_H */