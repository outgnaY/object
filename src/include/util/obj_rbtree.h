#ifndef OBJ_RBTREE_H
#define OBJ_RBTREE_H

#include "obj_core.h"

typedef unsigned char obj_rbtree_color_t;
typedef struct obj_rbtree_s obj_rbtree_t;
typedef struct obj_rbtree_node_s obj_rbtree_node_t;
typedef struct obj_rbtree_methods_s obj_rbtree_methods_t;

/* red-black tree */
struct obj_rbtree_s {
    obj_rbtree_node_t *root;
    obj_rbtree_node_t *sentinel;
    obj_rbtree_methods_t *methods;
    int node_cnt;
};

/* red-black tree node */
struct obj_rbtree_node_s {
    obj_rbtree_color_t color;           /* color of the node */
    obj_rbtree_node_t *left;            /* left child */
    obj_rbtree_node_t *right;           /* right child */
    obj_rbtree_node_t *parent;          /* parent */
    const void *key;                    /* key */
    const void *value;                  /* value */
};

/* rbtree related methods */
struct obj_rbtree_methods_s {
    int (*key_compare)(const void *key1, const void *key2);             /* key comparer */
    obj_bool_t (*insert)(obj_rbtree_t *tree, obj_rbtree_node_t *node);  /* insert implemention */
    void (*dump)(obj_rbtree_t *tree);
};


#define obj_rbtree_red(n) ((n)->color = 1)
#define obj_rbtree_black(n) ((n)->color = 0)
#define obj_rbtree_is_red(n) ((n)->color)
#define obj_rbtree_is_black(n) (!obj_rbtree_is_red(n))
#define obj_rbtree_parent(n) ((n)->parent)

#define obj_rbtree_sentinel_init(n) (obj_rbtree_black(n))


#endif  /* OBJ_RBTREE_H */