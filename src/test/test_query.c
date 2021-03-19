#include "obj_core.h"


void time_interval(struct timeval t1, struct timeval t2) {
    int us = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
    printf("total time: %ds %dus\n", us / 1000000, us - (us / 1000000) * 1000000);
}

int main() {
    obj_global_mem_context_init();
    /* ********** query request test ********** */
    /*
    obj_bson_t *cmd1 = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "skip", OBJ_BSON_BCON_INT32(4),
        "limit", OBJ_BSON_BCON_INT32(6),
        "filter", "{",
            "a", OBJ_BSON_BCON_INT32(1),
            "a.c", OBJ_BSON_BCON_UTF8("abc"),
        "}"
    );

    printf("cmd1:\n");
    obj_status_with_t status_with_qr1 = obj_query_parse_from_find_cmd(cmd1);
    obj_assert(status_with_qr1.code == OBJ_CODE_OK);
    obj_query_request_dump((obj_query_request_t *)status_with_qr1.data);
    obj_bson_t *cmd2 = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db1.coll1"),
        "filter", "{",
            "a", OBJ_BSON_BCON_INT32(1),
            "b", OBJ_BSON_BCON_INT32(1),
        "}",
        "projection", "{",
            "a", OBJ_BSON_BCON_INT32(1),
        "}",
        "sort", "{",
            "a", OBJ_BSON_BCON_INT32(1),
        "}"
    );
    printf("cmd2:\n");
    obj_status_with_t status_with_qr2 = obj_query_parse_from_find_cmd(cmd2);
    obj_assert(status_with_qr2.code == OBJ_CODE_OK);
    obj_query_request_dump((obj_query_request_t *)status_with_qr2.data);
    */
    /* ********** standard query test ********** */
    /*
    obj_status_with_t status_with_sq1 = obj_query_standardize((obj_query_request_t *)status_with_qr2.data);
    obj_assert(status_with_sq1.code == OBJ_CODE_OK);
    obj_standard_query_dump((obj_standard_query_t *)status_with_sq1.data);

    obj_bson_t *cmd3 = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db1.coll1"),
        "filter", "{",
            "a", OBJ_BSON_BCON_INT32(1),
            "b", OBJ_BSON_BCON_INT32(1),
            "$and", "[",
                "{", "x", OBJ_BSON_BCON_INT32(4), "}",
                "{", "y", OBJ_BSON_BCON_INT32(5), "}",
            "]",
        "}"
    );
    obj_status_with_t status_with_qr3 = obj_query_parse_from_find_cmd(cmd3);
    obj_status_with_t status_with_sq2 = obj_query_standardize((obj_query_request_t *)status_with_qr3.data);
    obj_assert(status_with_sq2.code == OBJ_CODE_OK);
    obj_standard_query_dump((obj_standard_query_t *)status_with_sq2.data);

    obj_bson_t *cmd4 = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db1.coll1"),
        "filter", "{",
            "$and", "[",
                "{", "x", OBJ_BSON_BCON_INT32(4), "}",
                "{", "y", OBJ_BSON_BCON_INT32(5), "}",
                "{", "z", "{", "$lte", OBJ_BSON_BCON_INT32(7), "}", "}",
            "]",
        "}"
    );
    obj_status_with_t status_with_qr4 = obj_query_parse_from_find_cmd(cmd4);
    obj_status_with_t status_with_sq3 = obj_query_standardize((obj_query_request_t *)status_with_qr4.data);
    obj_assert(status_with_sq3.code == OBJ_CODE_OK);
    obj_standard_query_dump((obj_standard_query_t *)status_with_sq3.data);
    */

    /* ********** find index test ********** */
    /*
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{",
            "$and", "[",
                "{", "x", OBJ_BSON_BCON_INT32(4), "}",
                "{", "y", OBJ_BSON_BCON_INT32(5), "}",
            "]",
        "}"
    );
    obj_status_with_t status_with_qr1 = obj_query_parse_from_find_cmd(cmd);
    obj_status_with_t status_with_sq1 = obj_query_standardize((obj_query_request_t *)status_with_qr1.data);
    obj_bson_t *kp1 = OBJ_BSON_BCON_NEW(
        "x", OBJ_BSON_BCON_INT32(1),
        "y", OBJ_BSON_BCON_INT32(-1)
    );
    obj_bson_t *kp2 = OBJ_BSON_BCON_NEW(
        "x", OBJ_BSON_BCON_INT32(1)
    );
    obj_bson_t *kp3 = OBJ_BSON_BCON_NEW(
        "z", OBJ_BSON_BCON_INT32(1)
    );
    obj_array_t all_indexes;
    obj_array_init(&all_indexes, sizeof(obj_bson_t *));
    obj_array_push_back(&all_indexes, &kp1);
    obj_array_push_back(&all_indexes, &kp2);
    obj_array_push_back(&all_indexes, &kp3);
    obj_query_planner_plan((obj_standard_query_t *)status_with_sq1.data, &all_indexes);
    */
    /* build query plan tree test */
    /*
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{",
            "$and", "[",
                "{", "x", OBJ_BSON_BCON_INT32(4), "}",
                "{", "y", OBJ_BSON_BCON_INT32(5), "}",
            "]",
        "}"
    );
    obj_status_with_t status_with_qr1 = obj_query_parse_from_find_cmd(cmd);
    obj_status_with_t status_with_sq1 = obj_query_standardize((obj_query_request_t *)status_with_qr1.data);
    obj_standard_query_t *sq = (obj_standard_query_t *)status_with_sq1.data;
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
    obj_array_t indexes;
    obj_array_init(&indexes, sizeof(obj_query_index_entry_t));
    obj_query_index_entry_t entry1 = {2, kp1};
    obj_query_index_entry_t entry2 = {1, kp2};
    obj_query_index_entry_t entry3 = {1, kp3};
    obj_array_push_back(&indexes, &entry1);
    obj_array_push_back(&indexes, &entry2);
    obj_array_push_back(&indexes, &entry3);
    obj_query_plan_tree_base_node_t *plan_root = NULL;
    obj_expr_base_expr_t *root = sq->root;
    obj_expr_base_expr_t *left = root->methods->get_child(root, 0);
    left->tag = (obj_expr_tag_t *)obj_expr_index_tag_compound_create(0, 0);
    obj_expr_base_expr_t *right = root->methods->get_child(root, 1);
    right->tag = (obj_expr_tag_t *)obj_expr_index_tag_create(2);
    obj_expr_dump(root);
    plan_root = obj_query_index_build_indexed_data_access(root, &indexes);
    obj_query_plan_tree_dump(plan_root, 0);
    */
    /* {"$and": [{"x": 4}, {"z", {"$lte": 7}}]} */
    /*
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{",
            "$and", "[",
                "{", "x", OBJ_BSON_BCON_INT32(4), "}",
                "{", "y", "{", "$lte", OBJ_BSON_BCON_INT32(7), "}", "}",
            "]",
        "}"
    );
    obj_status_with_t status_with_qr1 = obj_query_parse_from_find_cmd(cmd);
    obj_status_with_t status_with_sq1 = obj_query_standardize((obj_query_request_t *)status_with_qr1.data);
    obj_standard_query_t *sq = (obj_standard_query_t *)status_with_sq1.data;
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
    obj_array_t indexes;
    obj_array_init(&indexes, sizeof(obj_query_index_entry_t));
    obj_query_index_entry_t entry1 = {2, kp1};
    obj_query_index_entry_t entry2 = {1, kp2};
    obj_query_index_entry_t entry3 = {1, kp3};
    obj_array_push_back(&indexes, &entry1);
    obj_array_push_back(&indexes, &entry2);
    obj_array_push_back(&indexes, &entry3);
    obj_query_plan_tree_base_node_t *plan_root = NULL;
    obj_expr_base_expr_t *root = sq->root;
    obj_expr_base_expr_t *left = root->methods->get_child(root, 0);
    left->tag = (obj_expr_tag_t *)obj_expr_index_tag_compound_create(0, 0);
    obj_expr_base_expr_t *right = root->methods->get_child(root, 1);
    right->tag = (obj_expr_tag_t *)obj_expr_index_tag_create(2);
    plan_root = obj_query_index_build_indexed_data_access(root, &indexes);
    obj_query_plan_tree_dump(plan_root, 0);
    gettimeofday(&end, NULL);
    time_interval(start, end);
    */
    return 0;
}