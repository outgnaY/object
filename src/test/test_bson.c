#include "obj_core.h"

int main() {
    obj_global_mem_context_init();
    obj_bson_t *bson1 = OBJ_BSON_BCON_NEW("key", OBJ_BSON_BCON_UTF8("value"));
    obj_bson_value_t *value1 = obj_bson_get_path(bson1, "key");
    obj_assert(value1->type == OBJ_BSON_TYPE_UTF8);
    printf("%s\n", value1->value.v_utf8.str);
    
    obj_bson_t *bson2 = OBJ_BSON_BCON_NEW(
        "key1", OBJ_BSON_BCON_UTF8("value1"),
        "key2", "[", OBJ_BSON_BCON_UTF8("value2"), OBJ_BSON_BCON_UTF8("value3"), "]"
    );
    obj_bson_value_t *value2 = obj_bson_get_path(bson2, "key2[0]");
    printf("%d %s\n", value2->type, value2->value.v_utf8.str);

    obj_bson_t *bson3 = OBJ_BSON_BCON_NEW(
        "key1", OBJ_BSON_BCON_UTF8("value1"),
        "key2", "{",
            "key3", OBJ_BSON_BCON_INT32(3),
        "}"
    );
    obj_bson_value_t *value3 = obj_bson_get_path(bson3, "key2.key3");
    printf("%d %d\n", value3->type, value3->value.v_int32);
    return 0;
}