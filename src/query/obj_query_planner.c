#include "obj_core.h"

/* query planner */

static obj_uint64_t obj_stringdata_set_hash_func(const void *key);
static int obj_stringdata_set_key_compare(const void *key1, const void *key2);
static void *obj_stringdata_set_key_get(void *data);
static void obj_stringdata_set_key_set(void *data, void *key);

static void obj_query_planner_dump_relevant_indexes(obj_array_t *relevant_indexes);

/* stringdata set methods */
static obj_prealloc_map_methods_t stringdata_set_methods = {
    obj_stringdata_set_hash_func,
    obj_stringdata_set_key_compare,
    NULL,
    NULL,
    obj_stringdata_set_key_get,
    NULL,
    obj_stringdata_set_key_set,
    NULL,
    NULL,
    NULL
};

static obj_uint64_t obj_stringdata_set_hash_func(const void *key) {
    obj_stringdata_t *stringdata = (obj_stringdata_t *)key;
    return obj_set_hash_function(stringdata->data, stringdata->size);
}

static int obj_stringdata_set_key_compare(const void *key1, const void *key2) {
    obj_stringdata_t *stringdata1 = (obj_stringdata_t *)key1;
    obj_stringdata_t *stringdata2 = (obj_stringdata_t *)key2;
    return obj_stringdata_compare(stringdata1, stringdata2);
}

static void *obj_stringdata_set_key_get(void *data) {
    return (obj_stringdata_t *)data;
}

static void obj_stringdata_set_key_set(void *data, void *key) {
    obj_memcpy(data, key, sizeof(obj_stringdata_t));
}

/* build collection scan plan */
obj_query_plan_tree_base_node_t *obj_query_planner_build_collection_scan() {
    
}


/* 
 * generate query plan for a query 
 */
obj_status_with_t obj_query_planner_plan(obj_standard_query_t *sq, obj_array_t *indexes) {
    printf("begin planning...\n");
    /* output query plan trees */
    obj_array_t *out = NULL;
    out = obj_array_create(sizeof(obj_query_plan_tree_base_node_t *));
    if (out == NULL) {
        return obj_status_with_create(NULL, "out of memory", OBJ_CODE_QUERY_PLAN_NOMEM);
    }
    /*
     * 1. find relevant indexes    
     */
    /* get fields */
    obj_set_t fields;
    if (!obj_set_init(&fields, &stringdata_set_methods, sizeof(obj_stringdata_t))) {
        return obj_status_with_create(NULL, "out of memory", OBJ_CODE_QUERY_PLAN_NOMEM);
    }
    obj_query_index_get_fields(sq->root, &fields);
    /* find relevant indexes */
    obj_array_t relevant_indexes;
    if (!obj_array_init(&relevant_indexes, sizeof(obj_bson_t *))) {
        return obj_status_with_create(NULL, "out of memory", OBJ_CODE_QUERY_PLAN_NOMEM);
    }
    obj_query_index_find_relevant_indexes(&fields, indexes, &relevant_indexes);
    /* obj_query_planner_dump_relevant_indexes(&relevant_indexes); */
    /*
     * 2. if we have any relevant indexes, try to create indexed plans
     */
    if (relevant_indexes.size > 0) {
        /* set relevant tag for expression tree */
        obj_query_index_rate_indexes(sq->root, &relevant_indexes);

    }
    /*
     * check for sort
     */

    /*
     * check for projection 
     */

    /*
     * add collection scan node if we need to
     */
    if (out->size == 0) {

        
    }

    return obj_status_with_create(out, "", OBJ_CODE_OK);
}

/* for debug */
static void obj_query_planner_dump_relevant_indexes(obj_array_t *relevant_indexes) {
    int i;
    obj_bson_t *kp = NULL;
    for (i = 0; i < relevant_indexes->size; i++) {
        kp = (obj_bson_t *)obj_array_get_index_value(relevant_indexes, i, uintptr_t);
        printf("index %d:\n", i);
        obj_bson_visit_print_visit(kp);
        printf("\n");
    }
}