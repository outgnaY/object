#include "obj_core.h"

static const char *keys[] = {"9", "8", "7", "6", "5", "4", "3", "2", "1"};
static const char *values[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};


int main() {
    obj_global_mem_context_init();
    int i;
    obj_rbtree_t *tree = obj_rbtree_default_init();
    obj_bool_t res;
    for (i = 0; i < OBJ_NELEM(keys); i++) {
        res = obj_rbtree_insert(tree, keys[i], values[i]);
    }
    tree->methods->dump(tree);
    return 0;
}