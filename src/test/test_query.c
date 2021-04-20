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
    /*
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{",
            "$and", "[",
                "{", "x", OBJ_BSON_BCON_INT32(4), "}",
                "{", "y", OBJ_BSON_BCON_INT32(8), "}",
            "]",
        "}"
    );
    */
    /*
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{",
            "$and", "[",
                "{", "x", "{", "$lte", OBJ_BSON_BCON_INT32(8), "}", "}",
                "{", "x", "{", "$gte", OBJ_BSON_BCON_INT32(5), "}", "}",
            "]",
        "}"
    );
    */
    obj_bson_t *cmd = OBJ_BSON_BCON_NEW(
        "find", OBJ_BSON_BCON_UTF8("db.coll"),
        "filter", "{",
            "$or", "[",
                "{", 
                    "$and", "[",
                        "{", "x", "{", "$lte", OBJ_BSON_BCON_INT32(8), "}", "}",
                        "{", "x", "{", "$gte", OBJ_BSON_BCON_INT32(2), "}", "}",
                        "{", "y", "{", "$gte", OBJ_BSON_BCON_INT32(5), "}", "}",
                    "]",
                "}",
                "{",
                    "z", "{", "$gte", OBJ_BSON_BCON_INT32(5), "}",
                "}",
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
    obj_bson_t *prototype = OBJ_BSON_BCON_NEW(
        "x", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_INT32),
        "y", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_INT32),
        "z", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_INT32)
    );
    obj_collection_catalog_entry_t *collection = obj_collection_catalog_entry_create(prototype);
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
    obj_bson_t *kp4 = OBJ_BSON_BCON_NEW(
        "z", OBJ_BSON_BCON_INT32(1)
    );
    obj_index_catalog_entry_t entry1 = {2, kp1, obj_skiplist_create(0), 0};
    obj_index_catalog_entry_t entry2 = {1, kp2, obj_skiplist_create(0), 1};
    obj_index_catalog_entry_t entry3 = {1, kp3, obj_skiplist_create(0), 2};
    obj_index_catalog_entry_t entry4 = {1, kp4, obj_skiplist_create(0), 3};
    obj_array_push_back(&collection->indexes, &entry1);
    obj_array_push_back(&collection->indexes, &entry2);
    obj_array_push_back(&collection->indexes, &entry3);
    obj_array_push_back(&collection->indexes, &entry4);
    obj_record_store_t *record_store = collection->record_store;
    /* init data */
    int i;
    for (i = 0; i < 100; i++) {
        obj_bson_t *key1 = OBJ_BSON_BCON_NEW(
            "x", OBJ_BSON_BCON_INT32(i),
            "y", OBJ_BSON_BCON_INT32(i * 2)
        );
        obj_bson_t *key2 = OBJ_BSON_BCON_NEW(
            "x", OBJ_BSON_BCON_INT32(i)
        );
        obj_bson_t *key3 = OBJ_BSON_BCON_NEW(
            "y", OBJ_BSON_BCON_INT32(i * 2)
        );
        obj_bson_t *key4 = OBJ_BSON_BCON_NEW(
            "z", OBJ_BSON_BCON_INT32(i * 3)
        );
        obj_bson_t *data = OBJ_BSON_BCON_NEW(
            "x", OBJ_BSON_BCON_INT32(i),
            "y", OBJ_BSON_BCON_INT32(i * 2),
            "z", OBJ_BSON_BCON_INT32(i * 3)
        );
        obj_record_t *record = obj_alloc(sizeof(obj_record_t));
        record->bson = data;
        obj_record_store_add_record(record_store, record);
        obj_skiplist_insert(entry1.skiplist, key1, record);
        obj_skiplist_insert(entry2.skiplist, key2, record);
        obj_skiplist_insert(entry3.skiplist, key3, record);
        obj_skiplist_insert(entry4.skiplist, key4, record);
    }
    /*
    obj_skiplist_dump(entry1.skiplist);
    obj_skiplist_dump(entry2.skiplist);
    obj_skiplist_dump(entry3.skiplist);
    */

    gettimeofday(&start, NULL);
    obj_status_with_t status_with_qr1 = obj_query_parse_from_find_cmd(cmd);
    obj_status_with_t status_with_sq1 = obj_query_standardize((obj_query_request_t *)status_with_qr1.data);
    obj_standard_query_t *sq = (obj_standard_query_t *)status_with_sq1.data;
    obj_expr_dump(sq->root);
    
    obj_query_plan_executor_t *executor = obj_get_query_plan_executor_find(collection, sq);

    obj_query_plan_executor_exec_state_t state = OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_ADVANCED;
    obj_record_t *out;
    int cnt = 0;
    /*
    while ((state = obj_query_plan_executor_get_next(executor, &out)) == OBJ_QUERY_PLAN_EXECUTOR_EXEC_STATE_ADVANCED) {
        cnt++;
    }
    printf("cnt = %d\n", cnt);
    */
    gettimeofday(&end, NULL);
    time_interval(start, end);

    return 0;
}