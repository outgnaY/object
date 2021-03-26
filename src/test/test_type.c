#include "obj_core.h"

int main() {
    obj_global_mem_context_init();
    /* simple */
    obj_bson_t *proto1 = OBJ_BSON_BCON_NEW(
        "key1", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_DOUBLE)
    );
    obj_assert(obj_check_type_define(proto1));
    obj_bson_t *proto1_data1 = OBJ_BSON_BCON_NEW(
        "key1", OBJ_BSON_BCON_DOUBLE(6.7)
    );
    printf("proto1_data1 %d\n", obj_check_type(proto1, proto1_data1));
    obj_bson_t *proto1_data2 = OBJ_BSON_BCON_NEW(
        "key1", OBJ_BSON_BCON_INT32(8)
    );
    printf("proto1_data2 %d\n", obj_check_type(proto1, proto1_data2));
    obj_bson_t *proto1_data3 = OBJ_BSON_BCON_NEW(
        "key1", "{", "a", OBJ_BSON_BCON_UTF8("abc"), "}"
    );
    printf("proto1_data3 %d\n", obj_check_type(proto1, proto1_data3));

    /* simple object */
    obj_bson_t *proto2 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_DOUBLE),
            "b", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_UTF8),
        "}"
    );
    obj_assert(obj_check_type_define(proto2));
    obj_bson_t *proto2_data1 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_DOUBLE(6.6),
            "b", OBJ_BSON_BCON_UTF8("ABC"),
        "}"
    );
    printf("proto2_data1 %d\n", obj_check_type(proto2, proto2_data1));
    obj_bson_t *proto2_data2 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_DOUBLE(6.6),
            "b", OBJ_BSON_BCON_INT32(8),
        "}"
    );
    printf("proto2_data2 %d\n", obj_check_type(proto2, proto2_data2));
    obj_bson_t *proto2_data3 = OBJ_BSON_BCON_NEW(
        "key1", OBJ_BSON_BCON_DOUBLE(6.6)
    );
    printf("proto2_data3 %d\n", obj_check_type(proto2, proto2_data3));

    /* multilevel nested object */
    obj_bson_t *proto3 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_DOUBLE),
            "b", "{",
                "c", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_INT32),
                "d", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_UTF8),
            "}",
        "}"
    );
    obj_assert(obj_check_type_define(proto3));
    obj_bson_t *proto3_data1 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_DOUBLE(6.9),
            "b", "{",
                "c", OBJ_BSON_BCON_INT32(8),
                "d", OBJ_BSON_BCON_UTF8("abcdef"),
            "}",
        "}"
    );
    printf("proto3_data1 %d\n", obj_check_type(proto3, proto3_data1));
    obj_bson_t *proto3_data2 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_DOUBLE(6.9),
            "b1", "{",
                "c", OBJ_BSON_BCON_INT32(8),
                "d", OBJ_BSON_BCON_UTF8("abcdef"),
            "}",
        "}"
    );
    printf("proto3_data2 %d\n", obj_check_type(proto3, proto3_data2));

    /* with simple array */
    obj_bson_t *proto4 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_DOUBLE),
            "b", "{",
                "c", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_INT32),
                "d", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_UTF8),
            "}",
            "c", "[", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_DOUBLE), "]",
        "}"
    );
    obj_assert(obj_check_type_define(proto4));
    obj_bson_t *proto4_data1 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_DOUBLE(1.62),
            "b", "{",
                "c", OBJ_BSON_BCON_INT32(8),
                "d", OBJ_BSON_BCON_UTF8("CDE"),
            "}",
            "c", "[", OBJ_BSON_BCON_DOUBLE(3.6), OBJ_BSON_BCON_DOUBLE(8.6), "]",
        "}"
    );

    printf("proto4_data1 %d\n", obj_check_type(proto4, proto4_data1));
    obj_bson_t *proto4_data2 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_DOUBLE(1.62),
            "b", "{",
                "c", OBJ_BSON_BCON_INT32(8),
                "d", OBJ_BSON_BCON_UTF8("CDE"),
            "}",
            "c", "[", OBJ_BSON_BCON_DOUBLE(3.6), OBJ_BSON_BCON_INT32(8), "]",
        "}"
    );
    printf("proto4_data2 %d\n", obj_check_type(proto4, proto4_data2));
    obj_bson_t *proto4_data3 = OBJ_BSON_BCON_NEW(
        "key1", "{",
            "a", OBJ_BSON_BCON_DOUBLE(6.9),
            "b1", "{",
                "c", OBJ_BSON_BCON_INT32(8),
                "d", OBJ_BSON_BCON_UTF8("abcdef"),
            "}",
        "}"
    );
    printf("proto4_data3 %d\n", obj_check_type(proto4, proto4_data3));
    /* with complicated array */
    obj_bson_t *proto5 = OBJ_BSON_BCON_NEW(
        "key1", "[", 
            "{", "a", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_UTF8), "}",
        "]"
    );
    obj_assert(obj_check_type_define(proto5));
    obj_bson_t *proto5_data1 = OBJ_BSON_BCON_NEW(
        "key1", "[", 
            "{", "a", OBJ_BSON_BCON_UTF8("a.bc"), "}",
            "{", "a", OBJ_BSON_BCON_UTF8("a.bcd"), "}",
            "{", "a", OBJ_BSON_BCON_UTF8("a.bcde"), "}",
        "]"
    );
    printf("proto5_data1 %d\n", obj_check_type(proto5, proto5_data1));
    obj_bson_t *proto5_data2 = OBJ_BSON_BCON_NEW(
        "key1", "[", 
            "{", "a", OBJ_BSON_BCON_INT32(5), "}",
        "]"
    );
    printf("proto5_data2 %d\n", obj_check_type(proto5, proto5_data2));
    obj_bson_t *proto5_data3 = OBJ_BSON_BCON_NEW(
        "key1", "[", 
            "{", "a", OBJ_BSON_BCON_UTF8("a.bc"), "}",
            "{", "a", OBJ_BSON_BCON_INT32(5), "}",
        "]"
    );
    printf("proto5_data3 %d\n", obj_check_type(proto5, proto5_data3));
    return 0;
}