#include "obj_core.h"

int main() {
    obj_global_mem_context_init();
    int i;
    /* validate test */
    obj_global_error_code_t code;
    /* empty bson */
    /*
    obj_uint8_t empty[5] = {'\x05', '\x00', '\x00', '\x00', '\x00'};
    code = obj_bson_validate(empty, 5);
    printf("result: %d\n", code);
    */
    /* single k-v pair */
    /*
    obj_uint8_t simple_1[22] = {'\x16', '\x00', '\x00', '\x00', 
    '\x02', 
    'h', 'e', 'l', 'l', 'o', '\x00',
    '\x06', '\x00', '\x00', '\x00', 
    'w', 'o', 'r', 'l', 'd', '\x00', 
    '\x00'};
    code = obj_bson_validate(simple_1, 22);
    printf("result: %d\n", code);
    */
    /* multiple k-v pair */
    /*
    obj_uint8_t simple_2[39] = {'\x27', '\x00', '\x00', '\x00',
    '\x02',
    'h', 'e', 'l', 'l', 'o', '\x00',
    '\x06', '\x00', '\x00', '\x00', 
    'w', 'o', 'r', 'l', 'd', '\x00', 
    '\x02',
    'h', 'e', 'l', 'l', 'o', '\x00',
    '\x06', '\x00', '\x00', '\x00', 
    'w', 'o', 'r', 'l', 'd', '\x00', 
    '\x00'
    };
    code = obj_bson_validate(simple_2, 39);
    printf("result: %d\n", code);
    */
    /* {"BSON": ["awesome", 5.05, 1986]} */
    /*
    obj_uint8_t simple_3[49] = {'\x31', '\x00', '\x00', '\x00', 
    '\x04', 
    'B', 'S', 'O', 'N', '\x00',
    '\x26', '\x00', '\x00', '\x00',
    '\x02', '\x30', '\x00', '\x08', '\x00', '\x00', '\x00', 'a', 'w', 'e', 's', 'o', 'm', 'e', '\x00',
    '\x01', '\x31', '\x00', '\x33', '\x33', '\x33', '\x33', '\x33', '\x33', '\x14', '\x40',
    '\x10', '\x32', '\x00', '\xc2', '\x07', '\x00', '\x00',
    '\x00',
    '\x00'};
    code = obj_bson_validate(simple_3, 49);
    printf("result: %d\n", code);
    */

    /* simple case */
    /*
    obj_bson_t *bson1 = obj_bson_init();
    for (i = 0; i < 10; i++) {
        obj_bson_append_utf8(bson1, "hello", 5, "world", 5);
        obj_bson_print(bson1);
        printf("\n");
    }
    obj_bson_destroy(bson1);
    */
    /* nesting case */
    /*
    obj_bson_t *bson2 = obj_bson_init();
    obj_bson_t *array1 = obj_bson_init();
    obj_bson_append_utf8(array1, "0", 1, "awesome", 7);
    obj_bson_append_double(array1, "1", 1, 5.05);
    obj_bson_append_int32(array1, "2", 1, 1986);
    obj_bson_append_array(bson2, "BSON", 4, array1);
    obj_bson_print(bson2);
    obj_bson_destroy(bson2);
    obj_bson_destroy(array1);
    */
    /* test visitor, simple case */
    obj_bson_t *bson3 = obj_bson_init();
    for (i = 0; i < 6; i++) {
        obj_bson_append_utf8(bson3, "hello", 5, "world", 5);
    }
    obj_bool_t res = obj_bson_visit_print_visit(bson3);
    printf("\n");
    printf("res = %d\n", res);
    obj_bson_destroy(bson3);
    /* test visitor, nesting case */
    obj_bson_t *bson4 = obj_bson_init();
    obj_bson_t *array2 = obj_bson_init();
    obj_bson_append_utf8(array2, "0", 1, "awesome", 7);
    obj_bson_append_double(array2, "1", 1, 5.05);
    obj_bson_append_int32(array2, "2", 1, 1986);
    obj_bson_append_array(bson4, "BSON", 4, array2);
    res = obj_bson_visit_print_visit(bson4);
    printf("\n");
    printf("res = %d\n", res);
    obj_bson_destroy(bson4);
    obj_bson_destroy(array2);
    /* test validator, simple case */
    obj_bson_t *bson5 = obj_bson_init();
    for (i = 0; i < 6; i++) {
        obj_bson_append_utf8(bson5, "hello", 5, "world", 5);
    }
    obj_bson_print(bson5);
    res = obj_bson_visit_validate_visit(bson5, OBJ_BSON_VALIDATE_FLAG_NONE);
    printf("validate bson5 res: %d\n", res);
    bson5->data[0] = 0x6c;
    res = obj_bson_visit_validate_visit(bson5, OBJ_BSON_VALIDATE_FLAG_NONE);
    printf("validate bson5 res after modify: %d\n", res);
    bson5->data[0] = 0x6b;
    bson5->data[4] = 0x01;
    res = obj_bson_visit_validate_visit(bson5, OBJ_BSON_VALIDATE_FLAG_NONE);
    printf("validate bson5 res after modify: %d\n", res);
    obj_bson_destroy(bson5);
    /* test validator, nesting case */
    obj_bson_t *bson6 = obj_bson_init();
    obj_bson_t *array3 = obj_bson_init();
    obj_bson_append_utf8(array3, "0", 1, "awesome", 7);
    obj_bson_append_double(array3, "1", 1, 5.05);
    obj_bson_append_int32(array3, "2", 1, 1986);
    obj_bson_append_array(bson6, "BSON", 4, array3);
    res = obj_bson_visit_validate_visit(bson6, OBJ_BSON_VALIDATE_FLAG_NONE);
    printf("validate bson6 res: %d\n", res);
    bson6->data[0]++;
    bson6->len++;
    res = obj_bson_visit_validate_visit(bson6, OBJ_BSON_VALIDATE_FLAG_NONE);
    printf("validate bson6 res: %d\n", res);

    obj_bson_destroy(bson6);
    obj_bson_destroy(array3);
    return 0;
}