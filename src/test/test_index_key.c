#include "obj_core.h"

int main() {
    obj_global_mem_context_init();
    /* utf-8 string */
    /*
    obj_bson_t *bson1 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_UTF8("value1"));
    obj_bson_t *bson2 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_UTF8("value2"));
    obj_index_key_order_t o1 = 1;
    obj_index_key_order_t o2 = 0;
    obj_index_key_t *index_key1 = obj_index_key_create(bson1);
    obj_index_key_t *index_key2 = obj_index_key_create(bson2);
    int res1 = obj_index_key_compare(index_key1, index_key2, o1);
    printf("res1 = %d\n", res1);
    int res2 = obj_index_key_compare(index_key1, index_key2, o2);
    printf("res2 = %d\n", res2);
    obj_index_key_dump(index_key1);
    */
    /* double */
    /*
    obj_bson_t *bson3 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_DOUBLE(1.27));
    obj_bson_t *bson4 = OBJ_BSON_BCON_NEW("key1", OBJ_BSON_BCON_DOUBLE(1.28));
    obj_index_key_order_t o3 = 1;
    obj_index_key_order_t o4 = 0;
    obj_index_key_t *index_key3 = obj_index_key_create(bson3);
    obj_index_key_t *index_key4 = obj_index_key_create(bson4);
    int res3 = obj_index_key_compare(index_key3, index_key4, o3);
    printf("res3 = %d\n", res3);
    int res4 = obj_index_key_compare(index_key3, index_key4, o4);
    printf("res4 = %d\n", res4);
    */
    /* complex */
    obj_uint8_t bin5[5] = {4, 5, 6, 7, 8};
    obj_bson_t *bson5 = OBJ_BSON_BCON_NEW(
        "key1", OBJ_BSON_BCON_DOUBLE(59.6),
        "key2", OBJ_BSON_BCON_UTF8("abcdef"),
        "key3", OBJ_BSON_BCON_BINARY(bin5, 5), 
        "key4", OBJ_BSON_BCON_INT32(1000),
        "key5", OBJ_BSON_BCON_INT64(10000000000)
        );
    obj_uint8_t bin6[6] = {4, 5, 7, 8, 9, 10};
    obj_bson_t *bson6 = OBJ_BSON_BCON_NEW(
        "key1", OBJ_BSON_BCON_DOUBLE(59.6),
        "key2", OBJ_BSON_BCON_UTF8("abcdef"),
        "key3", OBJ_BSON_BCON_BINARY(bin6, 6), 
        "key4", OBJ_BSON_BCON_INT32(1000),
        "key5", OBJ_BSON_BCON_INT64(10000000000)
        );
    obj_bson_print(bson5);
    /*
    obj_bool_t print_res = obj_bson_visit_print_visit(bson5);
    printf("\n");
    printf("print_res = %d\n", print_res);
    */
    obj_v1_index_key_t *index_key5 = obj_v1_index_key_create(bson5);
    obj_v1_index_key_t *index_key6 = obj_v1_index_key_create(bson6);
    obj_index_key_order_t o5 = (1 | 2);
    obj_index_key_order_t o6 = (1 | 4);
    obj_v1_index_key_dump(index_key5);
    obj_v1_index_key_dump(index_key6);
    int res5 = obj_v1_index_key_compare(index_key5, index_key6, o5);
    int res6 = obj_v1_index_key_compare(index_key5, index_key6, o6);
    printf("res5 = %d\n", res5);
    printf("res6 = %d\n", res6);
    
    return 0;
}