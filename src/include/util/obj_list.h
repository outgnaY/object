#ifndef OBJ_LIST_H
#define OBJ_LIST_H

#include "obj_core.h"

typedef struct obj_list_node_s obj_list_node_t;
typedef struct obj_list_s obj_list_t;

#define obj_list_is_empty(l) (((l)->len) == 0)
#define obj_list_length(l) ((l)->len)
#define obj_list_get_head(l) ((l)->head)
#define obj_list_get_tail(l) ((l)->tail)
#define obj_list_prev_node(n) ((n)->prev)
#define obj_list_next_node(n) ((n)->next)
#define obj_list_node_value(n) ((n)->value)
#define obj_list_set_free(l, fn) ((l)->free = fn)

struct obj_list_node_s {
    obj_list_node_t *prev;
    obj_list_node_t *next;
    void *value;
};

struct obj_list_s {
    obj_list_node_t *head;
    obj_list_node_t *tail;
    int len;
    void (*free)(void *ptr);
};


obj_list_t *obj_list_create();

void obj_list_empty(obj_list_t *list);

void obj_list_destroy_static(obj_list_t *list);

void obj_list_destroy(obj_list_t *list);

void obj_list_add_node_head(obj_list_t *list, void *value);

void obj_list_add_node_tail(obj_list_t *list, void *value);

void obj_list_del_node(obj_list_t *list, obj_list_node_t *node);

void obj_list_del_node_head(obj_list_t *list);

void obj_list_del_node_tail(obj_list_t *list);


#endif  /* OBJ_LIST_H */