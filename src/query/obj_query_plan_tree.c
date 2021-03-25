#include "obj_core.h"

static char *obj_query_plan_tree_node_type_str_map[] = {
    "AND",
    "OR",
    "COLLECTION_SCAN",
    "INDEX_SCAN",
    "PROJECTION",
    "SORT",
    "SKIP",
    "LIMIT"
};


static void obj_query_plan_tree_init_base(obj_query_plan_tree_base_node_t *base_node, obj_query_plan_tree_node_methods_t *methods);
/* and node */
static obj_query_plan_tree_node_type_t obj_query_plan_tree_and_node_get_type();
static void obj_query_plan_tree_and_node_dump(obj_query_plan_tree_base_node_t *node);
/* or node */
static obj_query_plan_tree_node_type_t obj_query_plan_tree_or_node_get_type();
static void obj_query_plan_tree_or_node_dump(obj_query_plan_tree_base_node_t *node);
/* collection scan node */
static obj_query_plan_tree_node_type_t obj_query_plan_tree_collection_scan_node_get_type();
static void obj_query_plan_tree_collection_scan_node_dump(obj_query_plan_tree_base_node_t *node);
/* index scan node */
static obj_query_plan_tree_node_type_t obj_query_plan_tree_index_scan_node_get_type();
static void obj_query_plan_tree_index_scan_node_dump(obj_query_plan_tree_base_node_t *node);
/* projection node */
static obj_query_plan_tree_node_type_t obj_query_plan_tree_projection_node_get_type();
static void obj_query_plan_tree_projection_node_dump(obj_query_plan_tree_base_node_t *node);
/* sort node */
static obj_query_plan_tree_node_type_t obj_query_plan_tree_sort_node_get_type();
static void obj_query_plan_tree_sort_node_dump(obj_query_plan_tree_base_node_t *node);
/* skip node */
static obj_query_plan_tree_node_type_t obj_query_plan_tree_skip_node_get_type();
static void obj_query_plan_tree_skip_node_dump(obj_query_plan_tree_base_node_t *node);
/* limit node */
static obj_query_plan_tree_node_type_t obj_query_plan_tree_limit_node_get_type();
static void obj_query_plan_tree_limit_node_dump(obj_query_plan_tree_base_node_t *node);


/* build execution tree from query plan tree */
/* TODO collection */
/*
obj_exec_tree_base_node_t *obj_query_plan_tree_build_exec_tree(obj_query_plan_tree_base_node_t *root, obj_standard_query_t *sq, obj_exec_working_set_t *ws) {
    switch (root->methods->get_type()) {
        case OBJ_QUERY_PLAN_TREE_NODE_TYPE_AND: {
            obj_query_plan_tree_and_node_t *and_node = (obj_query_plan_tree_and_node_t *)root;
        }
        case OBJ_QUERY_PLAN_TREE_NODE_TYPE_OR: {
            obj_query_plan_tree_or_node_t *or_node = (obj_query_plan_tree_or_node_t *)root;
            int i;
            obj_query_plan_tree_base_node_t *child_node = NULL;
            for (i = 0; i < root->children.size; i++) {
                child_node = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&root->children, i, uintptr_t);
                obj_exec_tree
            }
        }
        case OBJ_QUERY_PLAN_TREE_NODE_TYPE_COLLECTION_SCAN: {
            obj_query_plan_tree_collection_scan_node_t *collection_scan_node = (obj_query_plan_tree_base_node_t *)root;
            
        }
        case OBJ_QUERY_PLAN_TREE_NODE_TYPE_INDEX_SCAN: {
            obj_query_plan_tree_index_scan_node_t *index_scan_node = (obj_query_plan_tree_index_scan_node_t *)root;
        }
        case OBJ_QUERY_PLAN_TREE_NODE_TYPE_PROJECTION: {
            obj_query_plan_tree_projection_node_t *projection_node = (obj_query_plan_tree_projection_node_t *)root;
        }
        case OBJ_QUERY_PLAN_TREE_NODE_TYPE_SORT: {
            obj_query_plan_tree_sort_node_t *sort_node = (obj_query_plan_tree_sort_node_t *)root;
            obj_query_plan_tree_base_node_t *child_node = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&root->children, 0, uintptr_t);
            obj_exec_tree_base_node_t *child_exec = obj_query_plan_tree_build_exec_tree(child_node, sq, ws);
            if (child_exec == NULL) {
                return NULL;
            }
            return (obj_exec_tree_base_node_t *)
        }
        case OBJ_QUERY_PLAN_TREE_NODE_TYPE_SKIP: {
            obj_query_plan_tree_skip_node_t *skip_node = (obj_query_plan_tree_skip_node_t *)root;
            obj_query_plan_tree_base_node_t *child_node = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&root->children, 0, uintptr_t);
            obj_exec_tree_base_node_t *child_exec = obj_query_plan_tree_build_exec_tree(child_node, sq, ws);
            if (child_exec == NULL) {
                return NULL;
            }
            return (obj_exec_tree_base_node_t *)obj_exec_tree_skip_node_create(ws, child_exec, skip_node->skip);
        }
        case OBJ_QUERY_PLAN_TREE_NODE_TYPE_LIMIT: {
            obj_query_plan_tree_limit_node_t *limit_node = (obj_query_plan_tree_limit_node_t *)root;
            obj_query_plan_tree_base_node_t *child_node = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&root->children, 0, uintptr_t);
            obj_exec_tree_base_node_t *child_exec = obj_query_plan_tree_build_exec_tree(child_node, sq, ws);
            if (child_exec == NULL) {
                return NULL;
            }
            return (obj_exec_tree_base_node_t *)obj_exec_tree_limit_node_create(ws, child_exec, limit_node->limit);
        }
        default:
            obj_assert(0);
    }
}
*/
/* count nodes */
int obj_query_plan_tree_count_nodes(obj_query_plan_tree_base_node_t *root) {
    int count = 1;
    int i;
    obj_query_plan_tree_base_node_t *child = NULL;
    for (i = 0; i < root->children.size; i++) {
        child = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&root->children, i, uintptr_t);
        count += obj_query_plan_tree_count_nodes(child);
    }
    return count;
}

/* destroy query plan tree */
void obj_query_plan_tree_destroy(obj_query_plan_tree_base_node_t *root) {
    int i;
    obj_query_plan_tree_base_node_t *child = NULL;
    /* destroy children */
    for (i = 0; i < root->children.size; i++) {
        child = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&root->children, i, uintptr_t);
        obj_query_plan_tree_destroy(child);
    }
    obj_array_destroy_static(&root->children);
    obj_free(root);
}

/* for debug */
void obj_query_plan_tree_dump(obj_query_plan_tree_base_node_t *root, int skip) {
    int i;
    /* output prefix " " */
    for (i = 0; i < skip; i++) {
        printf(" ");
    }
    root->methods->dump(root);
    obj_query_plan_tree_base_node_t *child = NULL;
    for (i = 0; i < root->children.size; i++) {
        child = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&root->children, i, uintptr_t);
        obj_query_plan_tree_dump(child, skip + 1);
    }
}

/* add child */
void obj_query_plan_tree_add_child(obj_query_plan_tree_base_node_t *root, obj_query_plan_tree_base_node_t *child) {
    obj_array_push_back(&root->children, &child);
}

/* add children */
void obj_query_plan_tree_add_children(obj_query_plan_tree_base_node_t *root, obj_array_t *children) {
    int i;
    obj_query_plan_tree_base_node_t *child;
    obj_array_reserve(&root->children, children->size + root->children.size);
    for (i = 0; i < children->size; i++) {
        child = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(children, i, uintptr_t);
        obj_array_push_back(&root->children, &child);
    }
}

/* init base structure */
static void obj_query_plan_tree_init_base(obj_query_plan_tree_base_node_t *base_node, obj_query_plan_tree_node_methods_t *methods) {
    obj_array_init(&base_node->children, sizeof(obj_query_plan_tree_base_node_t *));
    base_node->filter = NULL;
    base_node->methods = methods;
}

/* ********** and node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_and_node_methods = {
    obj_query_plan_tree_and_node_get_type,
    obj_query_plan_tree_and_node_dump
};

obj_query_plan_tree_and_node_t *obj_query_plan_tree_and_node_create() {
    obj_query_plan_tree_and_node_t *and_node = (obj_query_plan_tree_and_node_t *)obj_alloc(sizeof(obj_query_plan_tree_and_node_t));
    obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)and_node, &obj_query_plan_tree_and_node_methods);
    return and_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_and_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_AND;
}


static void obj_query_plan_tree_and_node_dump(obj_query_plan_tree_base_node_t *node) {
    obj_query_plan_tree_and_node_t *and_node = (obj_query_plan_tree_and_node_t *)node;
    printf("%s:\n", obj_query_plan_tree_node_type_str_map[node->methods->get_type()]);
}

/* ********** or node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_or_node_methods = {
    obj_query_plan_tree_or_node_get_type,
    obj_query_plan_tree_or_node_dump
};

obj_query_plan_tree_or_node_t *obj_query_plan_tree_or_node_create() {
    obj_query_plan_tree_or_node_t *or_node = (obj_query_plan_tree_or_node_t *)obj_alloc(sizeof(obj_query_plan_tree_or_node_t));
    obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)or_node, &obj_query_plan_tree_or_node_methods);
    return or_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_or_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_OR;
}


static void obj_query_plan_tree_or_node_dump(obj_query_plan_tree_base_node_t *node) {
    obj_query_plan_tree_or_node_t *or_node = (obj_query_plan_tree_or_node_t *)node;
    printf("%s:\n", obj_query_plan_tree_node_type_str_map[node->methods->get_type()]);
}

/* ********** collection scan node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_collection_scan_node_methods = {
    obj_query_plan_tree_collection_scan_node_get_type,
    obj_query_plan_tree_collection_scan_node_dump
};

obj_query_plan_tree_collection_scan_node_t *obj_query_plan_tree_collection_scan_node_create() {
    obj_query_plan_tree_collection_scan_node_t *collection_scan_node = (obj_query_plan_tree_collection_scan_node_t *)obj_alloc(sizeof(obj_query_plan_tree_collection_scan_node_t));
    obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)collection_scan_node, &obj_query_plan_tree_collection_scan_node_methods);
    return collection_scan_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_collection_scan_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_COLLECTION_SCAN;
}


static void obj_query_plan_tree_collection_scan_node_dump(obj_query_plan_tree_base_node_t *node) {
    obj_query_plan_tree_collection_scan_node_t *collection_scan_node = (obj_query_plan_tree_collection_scan_node_t *)node;
    printf("%s:\n", obj_query_plan_tree_node_type_str_map[node->methods->get_type()]);
}

/* ********** index scan node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_index_scan_node_methods = {
    obj_query_plan_tree_index_scan_node_get_type,
    obj_query_plan_tree_index_scan_node_dump
};

obj_query_plan_tree_index_scan_node_t *obj_query_plan_tree_index_scan_node_create(obj_query_index_entry_t *index_entry) {
    obj_query_plan_tree_index_scan_node_t *index_scan_node = (obj_query_plan_tree_index_scan_node_t *)obj_alloc(sizeof(obj_query_plan_tree_index_scan_node_t));
    obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)index_scan_node, &obj_query_plan_tree_index_scan_node_methods);
    index_scan_node->direction = 1;
    index_scan_node->index_entry = *index_entry;
    /* init index bounds else where */
    return index_scan_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_index_scan_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_INDEX_SCAN;
}


static void obj_query_plan_tree_index_scan_node_dump(obj_query_plan_tree_base_node_t *node) {
    obj_query_plan_tree_index_scan_node_t *index_scan_node = (obj_query_plan_tree_index_scan_node_t *)node;
    printf("%s:\n", obj_query_plan_tree_node_type_str_map[node->methods->get_type()]);
    obj_index_bounds_dump(&index_scan_node->bounds);
}

/* ********** projection node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_projection_node_methods = {
    obj_query_plan_tree_projection_node_get_type,
    obj_query_plan_tree_projection_node_dump
};

obj_query_plan_tree_projection_node_t *obj_query_plan_tree_projection_node_create() {
    obj_query_plan_tree_projection_node_t *projection_node = (obj_query_plan_tree_projection_node_t *)obj_alloc(sizeof(obj_query_plan_tree_projection_node_t));
    obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)projection_node, &obj_query_plan_tree_projection_node_methods);
    return projection_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_projection_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_PROJECTION;
}


static void obj_query_plan_tree_projection_node_dump(obj_query_plan_tree_base_node_t *node) {
    obj_query_plan_tree_projection_node_t *projection_node = (obj_query_plan_tree_projection_node_t *)node;
    printf("%s:\n", obj_query_plan_tree_node_type_str_map[node->methods->get_type()]);
}


/* ********* sort node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_sort_node_methods = {
    obj_query_plan_tree_sort_node_get_type,
    obj_query_plan_tree_sort_node_dump
};

obj_query_plan_tree_sort_node_t *obj_query_plan_tree_sort_node_create() {
    obj_query_plan_tree_sort_node_t *sort_node = (obj_query_plan_tree_sort_node_t *)obj_alloc(sizeof(obj_query_plan_tree_sort_node_t));
    obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)sort_node, &obj_query_plan_tree_sort_node_methods);
    return sort_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_sort_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_SORT;
}


static void obj_query_plan_tree_sort_node_dump(obj_query_plan_tree_base_node_t *node) {
    obj_query_plan_tree_sort_node_t *sort_node = (obj_query_plan_tree_sort_node_t *)node;
    printf("%s:\n", obj_query_plan_tree_node_type_str_map[node->methods->get_type()]);
}

/* ********** skip node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_skip_node_methods = {
    obj_query_plan_tree_skip_node_get_type,
    obj_query_plan_tree_skip_node_dump
};

obj_query_plan_tree_skip_node_t *obj_query_plan_tree_skip_node_create() {
    obj_query_plan_tree_skip_node_t *skip_node = (obj_query_plan_tree_skip_node_t *)obj_alloc(sizeof(obj_query_plan_tree_skip_node_t));
    obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)skip_node, &obj_query_plan_tree_skip_node_methods);
    return skip_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_skip_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_SKIP;
}


static void obj_query_plan_tree_skip_node_dump(obj_query_plan_tree_base_node_t *node) {
    obj_query_plan_tree_skip_node_t *skip_node = (obj_query_plan_tree_skip_node_t *)node;
    printf("%s:\n", obj_query_plan_tree_node_type_str_map[node->methods->get_type()]);
    printf("skip: %d\n", skip_node->skip);
}


/* ********** limit node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_limit_node_methods = {
    obj_query_plan_tree_limit_node_get_type,
    obj_query_plan_tree_limit_node_dump
};

obj_query_plan_tree_limit_node_t *obj_query_plan_tree_limit_node_create() {
    obj_query_plan_tree_limit_node_t *limit_node = (obj_query_plan_tree_limit_node_t *)obj_alloc(sizeof(obj_query_plan_tree_limit_node_t));
    obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)limit_node, &obj_query_plan_tree_limit_node_methods);
    return limit_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_limit_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_LIMIT;
}


static void obj_query_plan_tree_limit_node_dump(obj_query_plan_tree_base_node_t *node) {
    obj_query_plan_tree_limit_node_t *limit_node = (obj_query_plan_tree_limit_node_t *)node;
    printf("%s:\n", obj_query_plan_tree_node_type_str_map[node->methods->get_type()]);
    printf("limit = %d\n", limit_node->limit);
}
