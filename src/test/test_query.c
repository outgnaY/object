#include "obj_core.h"


int main() {
    obj_global_mem_context_init();
    /* ********** query request test ********** */
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
    
    /* ********** standard query test ********** */

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
    return 0;
}