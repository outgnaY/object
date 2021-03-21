#include "obj_core.h"


/* ********** working set methods ********** */

/* create working set */
obj_exec_working_set_create() {
    obj_exec_working_set_t *working_set = (obj_exec_working_set_t *)obj_alloc(sizeof(obj_exec_working_set_t));
    obj_array_init(&working_set->data, sizeof(obj_exec_working_set_member_holder_t));
    obj_array_set_free(&working_set->data, );
    working_set->freelist = OBJ_EXEC_WORKING_SET_INVALID_ID;
    return working_set;
}

/* destroy working set */
obj_exec_working_set_destroy(obj_exec_working_set_t *working_set) {
    obj_assert(working_set);
    
}

/* clear working set */
obj_exec_working_set_clear(obj_exec_working_set_t *working_set) {
    obj_assert(working_set);
    obj_array_empty(&working_set->data);
    /* reset freelist */
    working_set->freelist = OBJ_EXEC_WORKING_SET_INVALID_ID;
}

/* get working set member */
obj_exec_working_set_member_t *obj_exec_working_set_get(obj_exec_working_set_t *working_set, obj_exec_working_set_id_t id) {
    obj_exec_working_set_member_holder_t *holder = (obj_exec_working_set_member_holder_t *)obj_array_get_index(&working_set->data, id);
    obj_assert(id < working_set->data.size);
    obj_assert(id == holder->next_free_or_self);
    return holder->member;
}

void obj_exec_working_set_free(obj_exec_working_set_t *working_set, obj_exec_working_set_id_t id) {
    obj_exec_working_set_member_holder_t *holder = (obj_exec_working_set_member_holder_t *)obj_array_get_index(&working_set->data, id);
    obj_assert(id < working_set->data.size);
    obj_assert(id == holder->next_free_or_self);
    /* clear resources */
    obj_exec_working_set_member_clear(holder->member);
    /* insert into freelist */
    holder->next_free_or_self = working_set->freelist;
    working_set->freelist = id;
}


/* ********** working set member holder methods ********** */
void obj_exec_working_set_member_holder_destroy(obj_exec_working_set_member_holder_t *holder) {
    obj_assert(holder);
    
}


/* ********** working set member methods ********** */

/* destroy working set member */
void obj_exec_working_set_member_destroy(obj_exec_working_set_member_t *member) {
    obj_assert(member);

}

/* clear working set member */
void obj_exec_working_set_member_clear(obj_exec_working_set_member_t *member) {

}