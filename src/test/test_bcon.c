#include "obj_core.h"


int main() {
    obj_global_mem_context_init();
    /* simple bcon1 */
    obj_bson_t *bcon1 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_UTF8("value"));
    obj_bson_visit_print_visit(bcon1);
    printf("\n");
    /* simple bcon2 */
    obj_bson_t *bcon2 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_INT32(156));
    obj_bson_visit_print_visit(bcon2);
    printf("\n");
    /* simple bcon3 */
    obj_bson_t *bcon3 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_NULL);
    obj_bson_visit_print_visit(bcon3);
    printf("\n");
    /* simple bcon4 */
    obj_uint8_t bin[5] = {4, 5, 6, 7, 8};
    obj_bson_t *bcon4 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_BINARY(bin, 5));
    obj_bson_visit_print_visit(bcon4);
    printf("\n");
    /* simple bcon5 */
    obj_bson_t *bcon5 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_DOUBLE(1.26));
    obj_bson_visit_print_visit(bcon5);
    printf("\n");
    /* simple bcon6 */
    obj_bson_t *bcon6 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_INT64(100000000000000));
    obj_bson_visit_print_visit(bcon6);
    printf("\n");
    /* simple bcon7 */
    obj_bson_t *bcon7 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_BOOL(true));
    obj_bson_visit_print_visit(bcon7);
    printf("\n");
    /* complicated bcon8 */
    obj_bson_t *bcon8 = OBJ_BSON_BCON_NEW("key1", "{", "key2", OBJ_BSON_BCON_DOUBLE(1.26), "}");
    obj_bson_visit_print_visit(bcon8);
    printf("\n");
    /* complicated bson9 */
    obj_bson_t *bcon9 = OBJ_BSON_BCON_NEW("key1", "[", OBJ_BSON_BCON_DOUBLE(1.26), "]");
    obj_bson_visit_print_visit(bcon9);
    printf("\n");
    /* complicated bson10 */
    obj_bson_t *bcon10 = OBJ_BSON_BCON_NEW("key1", "[", OBJ_BSON_BCON_DOUBLE(1.26), "{", "key2", OBJ_BSON_BCON_BOOL(false), "}", "]");
    obj_bson_visit_print_visit(bcon10);
    printf("\n");
    /* complicated bson11 */
    obj_bson_t *bcon11 = OBJ_BSON_BCON_NEW (
        "name", "{",
        "first", OBJ_BSON_BCON_UTF8 ("Grace"),
        "last", OBJ_BSON_BCON_UTF8 ("Hopper"),
        "}",
        "languages", "[",
        OBJ_BSON_BCON_UTF8 ("MATH-MATIC"),
        OBJ_BSON_BCON_UTF8 ("FLOW-MATIC"),
        OBJ_BSON_BCON_UTF8 ("COBOL"),
        "]",
        "degrees", "[",
        "{", "degree", OBJ_BSON_BCON_UTF8 ("BA"), "school", OBJ_BSON_BCON_UTF8 ("Vassar"), "}",
        "{", "degree", OBJ_BSON_BCON_UTF8 ("PhD"), "school", OBJ_BSON_BCON_UTF8 ("Yale"), "}",
        "]");
    obj_bson_visit_print_visit(bcon11);
    printf("\n");
    return 0;
}