#include "obj_core.h"


int main() {
    obj_global_mem_context_init();
    obj_stringdata_t stringdata1;
    stringdata1 = obj_stringdata_create("db1.coll1");
    obj_stringdata_t prefix1, prefix2;
    prefix1 = obj_stringdata_create("db");
    prefix2 = obj_stringdata_create("db1.coll111");
    obj_bool_t res = obj_stringdata_startwith(&stringdata1, &prefix1);
    printf("%d\n", res);
    res = obj_stringdata_startwith(&stringdata1, &prefix2);
    printf("%d\n", res);
    return 0;
}