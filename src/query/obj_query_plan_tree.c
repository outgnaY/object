#include "obj_core.h"

/* add children */
obj_bool_t obj_query_plan_tree_add_children(obj_query_plan_tree_base_node_t *root, obj_array_t *children) {
    int i;
    obj_query_plan_tree_base_node_t *child;
    if (!obj_array_reserve(&root->children, children->size + root->children.size)) {
        return false;
    }
    for (i = 0; i < children->size; i++) {
        child = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(&root->children, i, uintptr_t);
        obj_array_push_back(&root->children, &child);
    }
    return true;
}

/* init base structure */
obj_bool_t obj_query_plan_tree_init_base(obj_query_plan_tree_base_node_t *base_node, obj_query_plan_tree_node_methods_t *methods) {
    if (!obj_array_init(&base_node->children, sizeof(obj_query_plan_tree_base_node_t *))) {
        return false;
    }
    base_node->filter = NULL;
    base_node->methods = &methods;
    return true;
}

/* ********** and node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_and_node_methods = {
    obj_query_plan_tree_and_node_get_type
};

obj_query_plan_tree_and_node_t *obj_query_plan_tree_and_node_create() {
    obj_query_plan_tree_and_node_t *and_node = (obj_query_plan_tree_and_node_t *)obj_alloc(sizeof(obj_query_plan_tree_and_node_t));
    if (and_node == NULL) {
        return NULL;
    }
    if (!obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)and_node, &obj_query_plan_tree_and_node_methods)) {
        obj_free(and_node);
        return NULL;
    }
    return and_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_and_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_AND;
}

/* ********** or node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_or_node_methods = {
    obj_query_plan_tree_or_node_get_type
};

obj_query_plan_tree_or_node_t *obj_query_plan_tree_or_node_create() {
    obj_query_plan_tree_or_node_t *or_node = (obj_query_plan_tree_or_node_t *)obj_alloc(sizeof(obj_query_plan_tree_or_node_t));
    if (or_node == NULL) {
        return NULL;
    }
    if (!obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)or_node, &obj_query_plan_tree_or_node_methods)) {
        obj_free(or_node);
        return NULL;
    }
    return or_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_or_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_OR;
}

/* ********** collection scan node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_collection_scan_node_methods = {
    obj_query_plan_tree_collection_scan_node_get_type
};

obj_query_plan_tree_collection_scan_node_t *obj_query_plan_tree_collection_scan_node_create() {
    obj_query_plan_tree_collection_scan_node_t *collection_scan_node = (obj_query_plan_tree_collection_scan_node_t *)obj_alloc(sizeof(obj_query_plan_tree_collection_scan_node_t));
    if (collection_scan_node == NULL) {
        return NULL;
    }
    if (!obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)collection_scan_node, &obj_query_plan_tree_collection_scan_node_methods)) {
        obj_free(collection_scan_node);
        return NULL;
    }
    return collection_scan_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_collection_scan_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_COLLECTION_SCAN;
}

/* ********** index scan node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_index_scan_node_methods = {
    obj_query_plan_tree_index_scan_node_get_type
};

obj_query_plan_tree_index_scan_node_t *obj_query_plan_tree_index_scan_node_create(obj_query_index_entry_t *index_entry) {
    obj_query_plan_tree_index_scan_node_t *index_scan_node = (obj_query_plan_tree_index_scan_node_t *)obj_alloc(sizeof(obj_query_plan_tree_index_scan_node_t));
    if (index_scan_node == NULL) {
        return NULL;
    }
    if (!obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)index_scan_node, &obj_query_plan_tree_index_scan_node_methods)) {
        obj_free(index_scan_node);
        return NULL;
    }
    index_scan_node->direction = 1;
    index_scan_node->index_entry = *index_entry;
    /* TODO init index bounds */
    return index_scan_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_index_scan_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_INDEX_SCAN;
}

/* ********** projection node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_projection_node_methods = {
    obj_query_plan_tree_projection_node_get_type
};

obj_query_plan_tree_projection_node_t *obj_query_plan_tree_projection_node_create() {
    obj_query_plan_tree_projection_node_t *projection_node = (obj_query_plan_tree_projection_node_t *)obj_alloc(sizeof(obj_query_plan_tree_projection_node_t));
    if (projection_node == NULL) {
        return NULL;
    }
    if (!obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)projection_node, &obj_query_plan_tree_projection_node_methods)) {
        obj_free(projection_node);
        return NULL;
    }
    return projection_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_projection_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_PROJECTION;
}


/* ********* sort node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_sort_node_methods = {
    obj_query_plan_tree_sort_node_get_type
};

obj_query_plan_tree_sort_node_t *obj_query_plan_tree_sort_node_create() {
    obj_query_plan_tree_sort_node_t *sort_node = (obj_query_plan_tree_sort_node_t *)obj_alloc(sizeof(obj_query_plan_tree_sort_node_t));
    if (sort_node == NULL) {
        return NULL;
    }
    if (!obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)sort_node, &obj_query_plan_tree_sort_node_methods)) {
        obj_free(sort_node);
        return NULL;
    }
    return sort_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_sort_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_SORT;
}


/* ********** skip node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_skip_node_methods = {
    obj_query_plan_tree_skip_node_get_type
};

obj_query_plan_tree_skip_node_t *obj_query_plan_tree_skip_node_create() {
    obj_query_plan_tree_skip_node_t *skip_node = (obj_query_plan_tree_skip_node_t *)obj_alloc(sizeof(obj_query_plan_tree_skip_node_t));
    if (skip_node == NULL) {
        return NULL;
    }
    if (!obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)skip_node, &obj_query_plan_tree_skip_node_methods)) {
        obj_free(skip_node);
        return NULL;
    }
    return skip_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_skip_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_SKIP;
}


/* ********** limit node ********** */

static obj_query_plan_tree_node_methods_t obj_query_plan_tree_limit_node_methods = {
    obj_query_plan_tree_limit_node_get_type
};

obj_query_plan_tree_limit_node_t *obj_query_plan_tree_limit_node_create() {
    obj_query_plan_tree_limit_node_t *limit_node = (obj_query_plan_tree_limit_node_t *)obj_alloc(sizeof(obj_query_plan_tree_limit_node_t));
    if (limit_node == NULL) {
        return NULL;
    }
    if (!obj_query_plan_tree_init_base((obj_query_plan_tree_base_node_t *)limit_node, &obj_query_plan_tree_limit_node_methods)) {
        obj_free(limit_node);
        return NULL;
    }
    return limit_node;
}

static obj_query_plan_tree_node_type_t obj_query_plan_tree_limit_node_get_type() {
    return OBJ_QUERY_PLAN_TREE_NODE_TYPE_LIMIT;
}