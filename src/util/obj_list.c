#include "obj_core.h"

/* create list */
obj_list_t *obj_list_create() {
    obj_list_t *list;
    if ((list = obj_alloc(sizeof(obj_list_t))) == NULL) {
        return NULL;
    }
    list->head = list->tail = NULL;
    list->len = 0;
    list->free = NULL;
    return list;
}

/* remove all elements from list */
void obj_list_empty(obj_list_t *list) {
    obj_assert(list);
    int len = list->len;
    obj_list_node_t *cur, *next;
    cur = list->head;
    while (len--) {
        next = cur->next;
        if (list->free) {
            list->free(cur->value);
        }
        obj_free(cur);
        cur = next;
    }
    list->head = list->tail = NULL;
    list->len = 0;
}


/* destroy a list */
void obj_list_destroy(obj_list_t *list) {
    obj_assert(list);
    obj_list_empty(list);
    obj_free(list);
}

/* add a new node to the head of the list */
obj_bool_t obj_list_add_node_head(obj_list_t *list, void *value) {
    obj_assert(list);
    obj_assert(value);
    obj_list_node_t *node;
    if ((node = obj_alloc(sizeof(obj_list_node_t))) == NULL) {
        return false;
    }
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }

    list->len++;
    return true;
}

/* add a new node to the tail of the list */
obj_bool_t obj_list_add_node_tail(obj_list_t *list, void *value) {
    obj_assert(list);
    obj_assert(value);
    obj_list_node_t *node;
    if ((node = obj_alloc(sizeof(obj_list_node_t))) == NULL) {
        return false;
    }
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return true;
}


/* delete a node */
void obj_list_del_node(obj_list_t *list, obj_list_node_t *node) {
    obj_assert(list);
    obj_assert(node);
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }
    if (list->free) {
        list->free(node->value);
    }
    obj_free(node);
    list->len--;
}

/* delete from head */
void obj_list_del_node_head(obj_list_t *list) {
    obj_assert(list);
    obj_assert(list->len > 0);
    obj_list_del_node(list, list->head);
}

/* delete from tail */
void obj_list_del_node_tail(obj_list_t *list) {
    obj_assert(list);
    obj_assert(list->len > 0);
    obj_list_del_node(list, list->tail);
}