#include "obj_core.h"

/* sentinel black node */
static obj_rbtree_node_t s_sentinel;

static int obj_rbtree_default_key_compare(const void *key1, const void *key2);
static obj_rbtree_node_t *obj_rbtree_create_node(const void *key, const void *value);
static obj_rbtree_node_t *obj_rbtree_leftmost(obj_rbtree_node_t *node);
static obj_rbtree_node_t *obj_rbtree_rightmost(obj_rbtree_node_t *node);
static void obj_rbtree_rotate_left(obj_rbtree_node_t *node, obj_rbtree_node_t **root);
static void obj_rbtree_rotate_right(obj_rbtree_node_t *node, obj_rbtree_node_t **root);
static void obj_rbtree_rebalance_for_insert(obj_rbtree_node_t *node, obj_rbtree_node_t **root);
static void obj_rbtree_rebalance_for_delete(obj_rbtree_node_t *node, obj_rbtree_node_t **root);
static obj_bool_t obj_rbtree_insert_node(obj_rbtree_t *tree, obj_rbtree_node_t *node);
static obj_bool_t obj_rbtree_insert_node_unique(obj_rbtree_t *tree, obj_rbtree_node_t *node);
static obj_rbtree_node_t *obj_rbtree_find_node(obj_rbtree_t *tree, const void *key);
static void obj_rbtree_delete_node(obj_rbtree_t *tree, obj_rbtree_node_t *node);
static void obj_rbtree_default_dump(obj_rbtree_t *tree);
static void obj_rbtree_pretty_print(obj_rbtree_node_t *node);
static void obj_rbtree_pretty_print_helper(obj_rbtree_node_t *node, int skip);
static void obj_rbtree_default_preorder_traverse(obj_rbtree_node_t *node);
static void obj_rbtree_default_midorder_traverse(obj_rbtree_node_t *node);
static void obj_rbtree_default_postorder_traverse(obj_rbtree_node_t *node);


static obj_rbtree_methods_t default_methods = {
    obj_rbtree_default_key_compare,
    obj_rbtree_insert_node_unique,
    obj_rbtree_default_dump,
    NULL,
    NULL
};

/* TODO implement iterator? */

static int obj_rbtree_default_key_compare(const void *key1, const void *key2) {
    obj_assert(key1 != NULL && key2 != NULL);
    return obj_strcmp(key1, key2);
}

/* default init */
obj_rbtree_t *obj_rbtree_default_init() {
    return obj_rbtree_init(&default_methods);
}

/* init a red-black tree */
obj_rbtree_t *obj_rbtree_init(obj_rbtree_methods_t *methods) {
    obj_rbtree_t *tree = obj_alloc(sizeof(obj_rbtree_t));
    if (tree == NULL) {
        return NULL;
    }
    tree->sentinel = &s_sentinel;
    tree->root = tree->sentinel;
    /* obj_rbtree_sentinel_init(tree->sentinel); */
    tree->node_cnt = 0;
    tree->methods = methods;
    return tree;
}

/* create new red-black tree node */
static inline obj_rbtree_node_t *obj_rbtree_create_node(const void *key, const void *value) {
    obj_rbtree_node_t *node = obj_alloc(sizeof(obj_rbtree_node_t));
    if (node == NULL) {
        return NULL;
    }
    obj_memset(node, 0, sizeof(obj_rbtree_node_t));
    node->key = key;
    node->value = value;
    return node;
}

/* left most node of tree rooted at node */
static inline obj_rbtree_node_t *obj_rbtree_leftmost(obj_rbtree_node_t *node) {
    while (node->left != &s_sentinel) {
        node = node->left;
    }
    return node;
}

/* right most node of tree rooted at node */
static inline obj_rbtree_node_t *obj_rbtree_rightmost(obj_rbtree_node_t *node) {
    while (node->right != &s_sentinel) {
        node = node->right;
    }
    return node;
}

/* rotate left */
/**
 *     p                            p
 *    /                            /
 *   node                         temp
 *  /    \                       /    \
 * lnode  temp          --->    node   rtemp
 *       /    \                /    \
 *      ltemp  rtemp          lnode  ltemp
 */
static void obj_rbtree_rotate_left(obj_rbtree_node_t *node, obj_rbtree_node_t **root) {
    obj_rbtree_node_t *temp;
    temp = node->right;
    node->right = temp->left;
    if (temp->left != &s_sentinel) {
        temp->left->parent = node;
    }
    temp->parent = node->parent;
    /* update parent pointer */
    if (node == *root) {
        *root = temp;
    } else if (node == node->parent->left) {
        node->parent->left = temp;
    } else {
        node->parent->right = temp;
    }
    temp->left = node;
    node->parent = temp;
}

/* rotate right */
/**
 *       p                      p
 *      /                      /
 *     node                   temp
 *    /    \                 /    \
 *   temp   rnode   --->    ltemp  node
 *  /    \                        /    \
 * ltemp  rtemp                  rtemp  rnode
 */
static void obj_rbtree_rotate_right(obj_rbtree_node_t *node, obj_rbtree_node_t **root) {
    obj_rbtree_node_t *temp = node->left;
    node->left = temp->right;
    if (temp->right != &s_sentinel) {
        temp->right->parent = node;
    }
    temp->parent = node->parent;
    /* update parent pointer */
    if (node == *root) {
        *root = temp;
    } else if (node == node->parent->left) {
        node->parent->left = temp;
    } else {
        node->parent->right = temp;
    }
    temp->right = node;
    node->parent = temp;
}

/* balance the tree after insert */
static void obj_rbtree_rebalance_for_insert(obj_rbtree_node_t *node, obj_rbtree_node_t **root) {
    obj_rbtree_node_t *parent, *gparent, *uncle, *temp;
    while ((parent = obj_rbtree_parent(node)) && obj_rbtree_is_red(parent)) {
        /* has parent and parent is a red */
        /* must has grandparent, because the root node is always black */
        gparent = obj_rbtree_parent(parent);
        if (parent == gparent->left) {
            /* uncle node */
            uncle = gparent->right;
            if (uncle && obj_rbtree_is_red(uncle)) {
                /* case 1 */
                obj_rbtree_black(uncle);
                obj_rbtree_black(parent);
                obj_rbtree_red(gparent);
                node = gparent;
                continue;
            }
            if (parent->right == node) {
                /* case 2 */
                /* rotate left */
                obj_rbtree_rotate_left(parent, root);
                temp = parent;
                parent = node;
                node = temp;
                /* continue to case 3 */
            }
            /* case 3 */
            obj_rbtree_black(parent);
            obj_rbtree_red(gparent);
            obj_rbtree_rotate_right(gparent, root);
        } else {
            /* symmetric operations */
            uncle = gparent->left;
            if (uncle && obj_rbtree_is_red(uncle)) {
                /* case 1 */
                obj_rbtree_black(uncle);
                obj_rbtree_black(parent);
                obj_rbtree_red(gparent);
                node = gparent;
                continue;
            }
            if (parent->left == node) {
                obj_rbtree_rotate_right(parent, root);
                temp = parent;
                parent = node;
                node = temp;
                /* continue to case 3 */
            }
            /* case 3 */
            obj_rbtree_black(parent);
            obj_rbtree_red(gparent);
            obj_rbtree_rotate_left(gparent, root);
        }
    }
    /* color the root black */
    obj_rbtree_black(*root);
}

/* balance the tree after delete */
static void obj_rbtree_rebalance_for_delete(obj_rbtree_node_t *temp, obj_rbtree_node_t **root) {
    obj_rbtree_node_t *other;
    while (temp != *root && obj_rbtree_is_black(temp)) {
        if (temp == temp->parent->left) {
            /* cousin node */
            other = temp->parent->right;
            if (obj_rbtree_is_red(other)) {
                obj_rbtree_black(other);
                obj_rbtree_red(temp->parent);
                obj_rbtree_rotate_left(temp->parent, root);
                other = temp->parent->right;
            }
            if (obj_rbtree_is_black(other->left) && obj_rbtree_is_black(other->right)) {
                obj_rbtree_red(other);
                temp = temp->parent;
            } else {
                if (obj_rbtree_is_black(other->right)) {
                    obj_rbtree_black(other->left);
                    obj_rbtree_red(other);
                    obj_rbtree_rotate_right(other, root);
                    other = temp->parent->right;
                }
                obj_rbtree_copy_color(other, temp->parent);
                obj_rbtree_black(temp->parent);
                obj_rbtree_black(other->right);
                obj_rbtree_rotate_left(temp->parent, root);
                /* terminate */
                temp = *root;
            }
        } else {
            /* symmetric operations */
            other = temp->parent->left;
            if (obj_rbtree_is_red(other)) {
                obj_rbtree_black(other);
                obj_rbtree_red(temp->parent);
                obj_rbtree_rotate_right(temp->parent, root);
                other = temp->parent->left;
            }
            if (obj_rbtree_is_black(other->left) && obj_rbtree_is_black(other->right)) {
                obj_rbtree_red(other);
                temp = temp->parent;
            } else {
                if (obj_rbtree_is_black(other->left)) {
                    obj_rbtree_black(other->right);
                    obj_rbtree_red(other);
                    obj_rbtree_rotate_left(other, root);
                    other = temp->parent->left;
                }
                obj_rbtree_copy_color(other, temp->parent);
                obj_rbtree_black(temp->parent);
                obj_rbtree_black(other->left);
                obj_rbtree_rotate_right(temp->parent, root);
                /* terminate */
                temp = *root;
            }
        }
    }
    /* black the node */
    obj_rbtree_black(temp);
}

/* insert */
static obj_bool_t obj_rbtree_insert_node(obj_rbtree_t *tree, obj_rbtree_node_t *node) {
    obj_rbtree_node_t *temp;
    if (tree->root == &s_sentinel) {
        /* empty tree */
        node->parent = NULL;
        node->left = &s_sentinel;
        node->right = &s_sentinel;
        /* color black */
        obj_rbtree_black(node);
        tree->root = node;
        tree->node_cnt++;
        return true;
    }
    /* insert the node to the tree */
    if (!tree->methods->insert(tree, node)) {
        return false;
    }
    /* rebalance the tree */
    obj_rbtree_rebalance_for_insert(node, &tree->root);
    tree->node_cnt++;
    return true;
}

/* insert unique*/
static obj_bool_t obj_rbtree_insert_node_unique(obj_rbtree_t *tree, obj_rbtree_node_t *node) {
    obj_rbtree_node_t **p;
    obj_rbtree_node_t *temp = tree->root;
    int compare;
    /* can't be sentinel node */
    obj_assert(temp != &s_sentinel);
    while (true) {
        compare = tree->methods->key_compare(node->key, temp->key);
        /* duplicate key. return false */
        if (compare == 0) {
            return false;
        }
        p = (compare < 0) ? &temp->left : &temp->right;
        if (*p == &s_sentinel) {
            break;
        }
        temp = *p;
    }
    *p = node;
    node->parent = temp;
    node->left = &s_sentinel;
    node->right = &s_sentinel;
    /* color red */
    obj_rbtree_red(node);
    return true;
}

/* insert value can't be duplicated */
obj_bool_t obj_rbtree_insert(obj_rbtree_t *tree, const void *key, const void *value) {
    obj_rbtree_node_t *node = obj_rbtree_create_node(key, value);
    if (node == NULL) {
        return false;
    }
    return obj_rbtree_insert_node(tree, node);
}

/* delete*/
obj_bool_t obj_rbtree_delete(obj_rbtree_t *tree, const void *key) {
    obj_rbtree_node_t *node;
    node = obj_rbtree_find_node(tree, key);
    if (node == NULL) {
        return false;
    }
    obj_rbtree_delete_node(tree, node);
    return true;
}

static obj_rbtree_node_t *obj_rbtree_find_node(obj_rbtree_t *tree, const void *key) {
    obj_rbtree_node_t *node = tree->root;
    int compare;
    while ((compare = tree->methods->key_compare(key, node->key)) != 0) {
        if (compare < 0) {
            if (node->left != &s_sentinel) {
                node = node->left;
            } else {
                return NULL;
            }
        } else {
            if (node->right != &s_sentinel) {
                node = node->right;
            } else {
                return NULL;
            }
        }
    }
    return node;
}

/* delete node */
static void obj_rbtree_delete_node(obj_rbtree_t *tree, obj_rbtree_node_t *node) {
    obj_rbtree_node_t *temp, *subst;
    obj_rbtree_node_t **root = &tree->root;
    obj_bool_t red;
    if (node->left == &s_sentinel) {
        temp = node->right;
        subst = node;
    } else if (node->right == &s_sentinel) {
        temp = node->left;
        subst = node;
    } else {
        subst = obj_rbtree_leftmost(node->right);
        temp = subst->right;
    }
    if (subst == *root) {
        *root = temp;
        obj_rbtree_black(temp);
        /* TODO free key/value of node */
        return;
    }
    red = obj_rbtree_is_red(subst);
    if (subst == subst->parent->left) {
        subst->parent->left = temp;
    } else {
        subst->parent->right = temp;
    }
    if (subst == node) {
        temp->parent = subst->parent;
    } else {
        if (subst->parent == node) {
            temp->parent = subst;
        } else {
            temp->parent = subst->parent;
        }
        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        obj_rbtree_copy_color(subst, node);
        if (node == *root) {
            *root = subst;
        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;
            } else {
                node->parent->right = subst;
            }
        }
        if (subst->left != &s_sentinel) {
            subst->left->parent = subst;
        }
        if (subst->right != &s_sentinel) {
            subst->right->parent = subst;
        }
    }
    /* TODO free key/value of node */
    if (red) {
        return;
    }
    obj_rbtree_rebalance_for_delete(temp, root);
}

/* for debug */
static void obj_rbtree_default_dump(obj_rbtree_t *tree) {
    obj_assert(tree);
    if (tree->root == &s_sentinel) {
        printf("empty tree\n");
        return;
    }
    obj_rbtree_pretty_print(tree->root);
    /*
    printf("total node num %d\n", tree->node_cnt);
    printf("********** preorder **********\n");
    obj_rbtree_default_preorder_traverse(tree->root);
    printf("********** midorder **********\n");
    obj_rbtree_default_midorder_traverse(tree->root);
    printf("********** postorder **********\n");
    obj_rbtree_default_postorder_traverse(tree->root);
    */
}

/* pretty print */
static void obj_rbtree_pretty_print(obj_rbtree_node_t *node) {
    obj_rbtree_pretty_print_helper(node, 0);
}

/* helper function */
static void obj_rbtree_pretty_print_helper(obj_rbtree_node_t *node, int skip) {
    int i;
    for (i = 0; i < skip; i++) {
        printf("-");
    }
    if (node == &s_sentinel) {
        printf("<nil>\n");
        return;
    }
    printf("%s\n", (char *)node->key);
    obj_rbtree_pretty_print_helper(node->left, skip + obj_strlen((char *)node->key));
    obj_rbtree_pretty_print_helper(node->right, skip + obj_strlen((char *)node->key));
}

static void obj_rbtree_default_preorder_traverse(obj_rbtree_node_t *node) {
    if (node == &s_sentinel) {
        return;
    }
    printf("[key]: %s, [value]: %s [color]: %d\n", (char *)node->key, (char *)node->value, node->color);
    obj_rbtree_default_preorder_traverse(node->left);
    obj_rbtree_default_preorder_traverse(node->right);
}

static void obj_rbtree_default_midorder_traverse(obj_rbtree_node_t *node) {
    if (node == &s_sentinel) {
        return;
    }
    obj_rbtree_default_midorder_traverse(node->left);
    printf("[key]: %s, [value]: %s [color]: %d\n", (char *)node->key, (char *)node->value, node->color);
    obj_rbtree_default_midorder_traverse(node->right);
}

static void obj_rbtree_default_postorder_traverse(obj_rbtree_node_t *node) {
    if (node == &s_sentinel) {
        return;
    }
    obj_rbtree_default_postorder_traverse(node->left);
    obj_rbtree_default_postorder_traverse(node->right);
    printf("[key]: %s, [value]: %s [color]: %d\n", (char *)node->key, (char *)node->value, node->color);
}

