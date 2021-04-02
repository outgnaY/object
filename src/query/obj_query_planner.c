#include "obj_core.h"

/* query planner */

static obj_uint64_t obj_string_set_hash_func(void *key);
static int obj_string_set_key_compare(void *key1, void *key2);
static void *obj_string_set_key_get(void *data);
static void obj_string_set_key_set(void *data, void *key);

static obj_query_plan_tree_base_node_t *obj_query_plan_tree_add_sort(obj_query_plan_tree_base_node_t *root, obj_bson_t *sort);
static obj_query_plan_tree_base_node_t *obj_query_plan_tree_add_skip(obj_query_plan_tree_base_node_t *root, int skip);
static obj_query_plan_tree_base_node_t *obj_query_plan_tree_add_limit(obj_query_plan_tree_base_node_t *root, int limit);
static obj_query_plan_tree_base_node_t *obj_query_plan_analyze_data_access(obj_query_request_t *qr, obj_query_plan_tree_base_node_t *root);
static obj_query_plan_tree_base_node_t *obj_query_plan_build_collection_scan(obj_standard_query_t *sq);
static void obj_query_planner_clean_unused_plans(obj_array_t *plans, int winner_index);
static obj_query_plan_tree_base_node_t *obj_query_planner_pick_best_plan(obj_array_t *plans, int *winner_index);
static void obj_query_planner_dump_relevant_indexes(obj_array_t *relevant_indexes);

/* string set methods */
static obj_prealloc_map_methods_t string_set_methods = {
    obj_string_set_hash_func,
    obj_string_set_key_compare,
    NULL,
    NULL,
    obj_string_set_key_get,
    NULL,
    obj_string_set_key_set,
    NULL,
    NULL,
    NULL
};

static obj_uint64_t obj_string_set_hash_func(void *key) {
    char *str = *(char **)key;
    return obj_set_hash_function(str, obj_strlen(str));
}

static int obj_string_set_key_compare(void *key1, void *key2) {
    char *str1 = *(char **)key1;
    char *str2 = *(char **)key2;
    return obj_strcmp(str1, str2);
}

static void *obj_string_set_key_get(void *data) {
    return data;
}

static void obj_string_set_key_set(void *data, void *key) {
    obj_memcpy(data, key, sizeof(char *));
}


/* add sort */
static inline obj_query_plan_tree_base_node_t *obj_query_plan_tree_add_sort(obj_query_plan_tree_base_node_t *root, obj_bson_t *sort) {
    obj_query_plan_tree_sort_node_t *sort_node = obj_query_plan_tree_sort_node_create();
    sort_node->pattern = sort;
    obj_query_plan_tree_add_child((obj_query_plan_tree_base_node_t *)sort_node, root);
    return (obj_query_plan_tree_base_node_t *)sort_node;
}

/* add skip */
static inline obj_query_plan_tree_base_node_t *obj_query_plan_tree_add_skip(obj_query_plan_tree_base_node_t *root, int skip) {
    obj_query_plan_tree_skip_node_t *skip_node = obj_query_plan_tree_skip_node_create();
    skip_node->skip = skip;
    obj_query_plan_tree_add_child((obj_query_plan_tree_base_node_t *)skip_node, root);
    return (obj_query_plan_tree_base_node_t *)skip_node;
}

/* add limit */
static inline obj_query_plan_tree_base_node_t *obj_query_plan_tree_add_limit(obj_query_plan_tree_base_node_t *root, int limit) {
    obj_query_plan_tree_limit_node_t *limit_node = obj_query_plan_tree_limit_node_create();
    limit_node->limit = limit;
    obj_query_plan_tree_add_child((obj_query_plan_tree_base_node_t *)limit_node, root);
    return (obj_query_plan_tree_base_node_t *)limit_node;
}

/* handle sort/skip/limit */
static obj_query_plan_tree_base_node_t *obj_query_plan_analyze_data_access(obj_query_request_t *qr, obj_query_plan_tree_base_node_t *root) {
    /* sort->skip->limit */
    if (!obj_bson_is_empty(&qr->sort)) {
        root = obj_query_plan_tree_add_sort(root, &qr->sort);
    }
    if (qr->skip != -1) {
        root = obj_query_plan_tree_add_skip(root, qr->skip);
    }
    if (qr->limit != -1) {
        root = obj_query_plan_tree_add_limit(root, qr->limit);
    }
    return root;
}

/* build collection scan node */
static obj_query_plan_tree_base_node_t *obj_query_plan_build_collection_scan(obj_standard_query_t *sq) {
    obj_query_plan_tree_collection_scan_node_t *collection_scan_node = obj_query_plan_tree_collection_scan_node_create();
    obj_query_plan_tree_base_node_t *root = NULL;
    collection_scan_node->base.filter = sq->root;
    /* TODO find collection */
    /* collection_scan_node->collection */
    root = obj_query_plan_analyze_data_access(sq->qr, (obj_query_plan_tree_base_node_t *)collection_scan_node);
    return root;
}


/* 
 * Entrance: generate query plan for a query 
 */
obj_status_with_t obj_query_planner_plan(obj_standard_query_t *sq, obj_array_t *indexes) {
    printf("begin planning...\n");
    /* possible query plans */
    obj_array_t plans;
    obj_array_init(&plans, sizeof(obj_query_plan_tree_base_node_t *));
    /*
     * 1. find relevant indexes    
     */
    /* get fields */
    obj_set_t fields;
    obj_set_init(&fields, &string_set_methods, sizeof(char *));
    obj_query_index_get_fields(sq->root, &fields);
    /* find relevant indexes */
    obj_array_t relevant_indexes;
    obj_array_init(&relevant_indexes, sizeof(obj_index_catalog_entry_t));
    obj_query_index_find_relevant_indexes(&fields, indexes, &relevant_indexes);
    /* obj_query_planner_dump_relevant_indexes(&relevant_indexes); */
    /*
     * 2. if we have any relevant indexes, try to create indexed plans
     */
    if (relevant_indexes.size > 0) {
        /* set relevant tag for expression tree */
        obj_query_index_rate_indexes(sq->root, &relevant_indexes);
        obj_query_plan_iter_t pi;
        obj_query_plan_iter_init(&pi, &relevant_indexes, sq->root);
        obj_expr_base_expr_t *tag_tree = NULL;
        while ((tag_tree = obj_query_plan_iter_get_next(&pi)) != NULL) {
            /* sort by index tag */
            obj_expr_sort_using_tags(tag_tree);
            obj_query_plan_tree_base_node_t *plan_root = NULL;
            plan_root = obj_query_index_build_indexed_data_access(tag_tree, &relevant_indexes);
            if (plan_root == NULL) {
                continue;
            }
            plan_root = obj_query_plan_analyze_data_access(sq->qr, plan_root);
            /*
            printf("********** plan tree: **********\n");
            obj_query_plan_tree_dump(plan_root, 0);
            */
            /* add to plans */
            obj_array_push_back(&plans, &plan_root);
            
        }
    }

    /* don't leave tags on query tree */
    obj_expr_reset_tag(sq->root);

    /*
     * make collection scan node if we need to
     */
    if (plans.size == 0) {
        obj_query_plan_tree_base_node_t *root = NULL;
        root = obj_query_plan_build_collection_scan(sq);
        obj_array_push_back(&plans, &root);
    }
    int winner_index;
    obj_query_plan_tree_base_node_t *winner_plan = obj_query_planner_pick_best_plan(&plans, &winner_index);
    obj_query_planner_clean_unused_plans(&plans, winner_index);
    obj_array_destroy_static(&plans);
    return obj_status_with_create(winner_plan, "", OBJ_CODE_OK);
}


static void obj_query_planner_clean_unused_plans(obj_array_t *plans, int winner_index) {
    int i;
    obj_query_plan_tree_base_node_t *plan = NULL;
    for (i = 0; i < plans->size; i++) {
        if (i != winner_index) {
            /* clean */
            plan = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(plans, i, uintptr_t);
            obj_query_plan_tree_destroy(plan);
        }
    }
}

/* pick a plan */
static obj_query_plan_tree_base_node_t *obj_query_planner_pick_best_plan(obj_array_t *plans, int *winner_index) {
    obj_assert(plans->size > 0);
    obj_query_plan_tree_base_node_t *current = NULL;
    int current_node_num;
    int i;
    current = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(plans, 0, uintptr_t);
    if (plans->size == 1) {
        *winner_index = 0;
        return current;
    }
    int min_node_num = obj_query_plan_tree_count_nodes(current);
    int min_node_index = 0;
    obj_query_plan_tree_base_node_t *winner = NULL;
    for (i = 1; i < plans->size; i++) {
        current = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(plans, 0, uintptr_t);
        current_node_num = obj_query_plan_tree_count_nodes(current);
        if (current_node_num < min_node_num) {
            min_node_index = i;
        }
    }
    winner = (obj_query_plan_tree_base_node_t *)obj_array_get_index_value(plans, min_node_index, uintptr_t);
    *winner_index = min_node_index;
    return winner;
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

