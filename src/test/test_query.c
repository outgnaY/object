#include "obj_core.h"


int main() {
    obj_global_mem_context_init();
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
    /*
    obj_bson_visit_print_visit(cmd1);
    printf("\n");
    */
    obj_status_with_t status_with1 = obj_query_parse_from_find_cmd(cmd1);
    obj_assert(status_with1.code == OBJ_CODE_OK);
    obj_query_request_dump((obj_query_request_t *)status_with1.data);
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
    obj_status_with_t status_with2 = obj_query_parse_from_find_cmd(cmd2);
    obj_assert(status_with2.code == OBJ_CODE_OK);
    obj_query_request_dump((obj_query_request_t *)status_with2.data);
    return 0;
}