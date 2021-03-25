#include "obj_core.h"


static void obj_exec_working_set_member_holder_destroy_static(void *ptr);
static obj_exec_working_set_member_t *obj_exec_working_set_member_create();
static void obj_exec_working_set_member_destroy(obj_exec_working_set_member_t *member);

/* ********** working set methods ********** */

/* create working set */
obj_exec_working_set_t *obj_exec_working_set_create() {
    obj_exec_working_set_t *working_set = (obj_exec_working_set_t *)obj_alloc(sizeof(obj_exec_working_set_t));
    obj_array_init(&working_set->data, sizeof(obj_exec_working_set_member_holder_t));
    obj_array_set_free(&working_set->data, obj_exec_working_set_member_holder_destroy_static);
    working_set->freelist = OBJ_EXEC_WORKING_SET_INVALID_ID;
    return working_set;
}

/* destroy working set */
void obj_exec_working_set_destroy(obj_exec_working_set_t *working_set) {
    obj_assert(working_set);
    obj_array_destroy_static(&working_set->data);
}

/* clear working set */
void obj_exec_working_set_clear(obj_exec_working_set_t *working_set) {
    obj_assert(working_set);
    obj_array_empty(&working_set->data);
    /* reset freelist */
    working_set->freelist = OBJ_EXEC_WORKING_SET_INVALID_ID;
}

/* allocate new member */
obj_exec_working_set_id_t obj_exec_working_set_allocate(obj_exec_working_set_t *working_set) {
    obj_exec_working_set_id_t id;
    obj_exec_working_set_member_holder_t *holder = NULL;
    if (working_set->freelist == OBJ_EXEC_WORKING_SET_INVALID_ID) {
        /* all in use */
        id = working_set->data.size;
        obj_array_resize(&working_set->data, id + 1);
        holder = (obj_exec_working_set_member_holder_t *)obj_array_get_index(&working_set->data, id);
        holder->next_free_or_self = id;
        holder->member = obj_exec_working_set_member_create();
        return id;
    }
    /* pop head */
    id = working_set->freelist;
    holder = (obj_exec_working_set_member_holder_t *)obj_array_get_index(&working_set->data, id);
    working_set->freelist = holder->next_free_or_self;
    /* in-use */
    holder->next_free_or_self = id;
    return id;
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
    /*
    obj_exec_working_set_member_clear(holder->member);
    */
    /* insert into freelist */
    holder->next_free_or_self = working_set->freelist;
    working_set->freelist = id;
}


/* ********** working set member holder methods ********** */
static void obj_exec_working_set_member_holder_destroy_static(void *ptr) {
    obj_exec_working_set_member_holder_t *holder = (obj_exec_working_set_member_holder_t *)ptr;
    obj_exec_working_set_member_destroy(holder->member);
    holder->member = NULL;
}


/* ********** working set member methods ********** */

static inline obj_exec_working_set_member_t *obj_exec_working_set_member_create() {
    obj_exec_working_set_member_t *member = obj_alloc(sizeof(obj_exec_working_set_member_t));
    member->record = NULL;
    return member;
}

/* destroy working set member */
static inline void obj_exec_working_set_member_destroy(obj_exec_working_set_member_t *member) {
    obj_assert(member);
    obj_free(member);
}
