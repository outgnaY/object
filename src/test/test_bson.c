#include "obj_core.h"

int main() {
    obj_global_mem_context_init();
    /*
    obj_bson_t *bson = obj_bson_init_heap();
    obj_bson_append_utf8(bson, "hello", 5, "world", 5);
    obj_bson_print(bson);
    */

    /* validate test */
    obj_global_error_code_t code;
    /* empty bson */
    /*
    obj_uint8_t empty[5] = {'\x05', '\x00', '\x00', '\x00', '\x00'};
    code = obj_bson_validate(empty, 5);
    printf("result: %d\n", code);
    */
    /* single k-v pair */
    obj_uint8_t simple_1[22] = {'\x16', '\x00', '\x00', '\x00', 
    '\x02', 
    'h', 'e', 'l', 'l', 'o', '\x00',
    '\x06', '\x00', '\x00', '\x00', 
    'w', 'o', 'r', 'l', 'd', '\x00', 
    '\x00'};
    code = obj_bson_validate(simple_1, 22);
    printf("result: %d\n", code);
    /* multiple k-v pair */
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
    obj_bson_t *bson = obj_bson_init();
    obj_bson_append_utf8(bson, "hello", 5, "world", 5);
    obj_bson_print(bson);
    printf("\n");
    return 0;
}