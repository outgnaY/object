#include "obj_core.h"


void time_interval(struct timeval t1, struct timeval t2) {
    int us = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
    printf("total time: %ds %dus\n", us / 1000000, us - (us / 1000000) * 1000000);
}

int main() {
    /* obj_global_mem_context_init(); */
    /* query planner test */
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL); 
    
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{",
            "$or", "[",
                "{", "x", OBJ_BSON_BCON_INT32(4), "}",
                "{", "y", OBJ_BSON_BCON_INT32(5), "}",
            "]",
        "}"
    );
    
    /*
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{", "z", OBJ_BSON_BCON_INT32(1), "}"
    );
    */
    /*
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{",
            "$and", "[",
                "{", "x", OBJ_BSON_BCON_INT32(4), "}",
                "{", 
                    "$or", "[",
                        "{", "y", OBJ_BSON_BCON_INT32(5), "}",
                    "]",
                "}",
                "{", "z", OBJ_BSON_BCON_INT32(5), "}",
            "]",
        "}"
    );
    */
    obj_status_with_t status_with_qr1 = obj_query_parse_from_find_cmd(cmd);
    obj_status_with_t status_with_sq1 = obj_query_standardize((obj_query_request_t *)status_with_qr1.data);
    obj_standard_query_t *sq = (obj_standard_query_t *)status_with_sq1.data;
    obj_expr_dump(sq->root);
    obj_bson_t *kp1 = OBJ_BSON_BCON_NEW(
        "x", OBJ_BSON_BCON_INT32(1),
        "y", OBJ_BSON_BCON_INT32(-1)
    );
    obj_bson_t *kp2 = OBJ_BSON_BCON_NEW(
        "x", OBJ_BSON_BCON_INT32(1)
    );
    obj_bson_t *kp3 = OBJ_BSON_BCON_NEW(
        "y", OBJ_BSON_BCON_INT32(1)
    );
    /*
    obj_array_t indexes;
    obj_array_init(&indexes, sizeof(obj_query_index_entry_t));
    obj_query_index_entry_t entry1 = {2, kp1};
    obj_query_index_entry_t entry2 = {1, kp2};
    obj_query_index_entry_t entry3 = {1, kp3};
    
    obj_array_push_back(&indexes, &entry1);
    obj_array_push_back(&indexes, &entry2);
    obj_array_push_back(&indexes, &entry3);
    
    obj_status_with_t status_with_plan = obj_query_planner_plan(sq, &indexes);
    obj_query_plan_tree_base_node_t *plan_tree = (obj_query_plan_tree_base_node_t *)status_with_plan.data;
    obj_query_plan_tree_dump(plan_tree, 0);
    gettimeofday(&end, NULL);
    time_interval(start, end);
    */
    /*
    obj_bson_t *data = OBJ_BSON_BCON_NEW(
        "x", OBJ_BSON_BCON_INT32(8),
        "y", OBJ_BSON_BCON_INT32(5),
        "z", OBJ_BSON_BCON_UTF8("aaa")
    );
    */
    obj_collection_catalog_entry_t *collection = obj_collection_catalog_entry_create();
    obj_record_store_t *record_store = collection->record_store;
    int i;
    for (i = 0; i < 100000; i++) {
        obj_bson_t *data = OBJ_BSON_BCON_NEW(
            "x", OBJ_BSON_BCON_INT32(8),
            "y", OBJ_BSON_BCON_INT32(6),
            "z", OBJ_BSON_BCON_UTF8("aaa")
        );
        obj_record_store_add(record_store, data);
    }
    obj_array_t indexes;
    obj_array_init(&indexes, sizeof(obj_query_index_entry_t));
    obj_status_with_t status_with_plan = obj_query_planner_plan(sq, &indexes);
    obj_query_plan_tree_base_node_t *plan_tree = (obj_query_plan_tree_base_node_t *)status_with_plan.data;
    ((obj_query_plan_tree_collection_scan_node_t *)plan_tree)->collection = collection;
    obj_query_plan_tree_dump(plan_tree, 0);
    obj_exec_working_set_t *ws = obj_exec_working_set_create();
    obj_exec_tree_base_node_t *exec_tree = obj_query_plan_tree_build_exec_tree(plan_tree, sq, ws);
    obj_exec_working_set_id_t id;
    obj_record_t *record = NULL;
    obj_exec_tree_exec_state_t state;
    while ((state = exec_tree->methods->work(exec_tree, &id)) != OBJ_EXEC_TREE_STATE_EOF) {
        if (state == OBJ_EXEC_TREE_STATE_ADVANCED) {
            printf("found\n"); 
            obj_exec_working_set_member_t *member = obj_exec_working_set_get(ws, id);
        }

    }
    gettimeofday(&end, NULL);
    time_interval(start, end);
    return 0;
}