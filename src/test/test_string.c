#include "obj_core.h"


int main() {
    obj_bool_t res;
    obj_print_utf8_string("abcd", 4);
    res = obj_validate_utf8_string("abcd", 4, false);
    printf("res = %d\n", res);

    /* printf("%d\n", sizeof("中文测试")); */
    obj_print_utf8_string("中文测试", sizeof("中文测试") - 1);
    res = obj_validate_utf8_string("中文测试", sizeof("中文测试") - 1, false);
    printf("res = %d\n", res);
    
    return 0;
}