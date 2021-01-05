#include "obj_core.h"

/* sds tests */
int main() {
    obj_global_mem_context_init();
    obj_sds x = obj_sds_new("foo"), y;
    obj_assert(obj_sds_len(x) == 3 && obj_memcmp(x, "foo\0", 4) == 0);
    obj_sds_free(x);
    x = obj_sds_newlen("foo", 2);
    obj_assert(obj_sds_len(x) == 2 && obj_memcmp(x, "fo\0", 3) == 0);
    x = obj_sds_cat(x, "bar");
    obj_assert(obj_sds_len(x) == 5 && obj_memcmp(x, "fobar\0", 6) == 0);
    x = obj_sds_cpy(x, "a");
    obj_assert(obj_sds_len(x) == 1 && obj_memcmp(x, "a\0", 2) == 0);
    x = obj_sds_cpy(x, "xyzxxxxxxxxxxyyyyyyyyyyzzzzzzzzzz");
    obj_assert(obj_sds_len(x) == 33 && obj_memcmp(x, "xyzxxxxxxxxxxyyyyyyyyyyzzzzzzzzzz\0", 33) == 0);
    obj_sds_free(x);
    /* format */
    x = obj_sds_new("--");
    x = obj_sds_catfmt(x, "%u,%U--", __UINT32_MAX__, __UINT64_MAX__);
    /* obj_sds_dump(x); */
    obj_assert(obj_sds_len(x) == 35 && obj_memcmp(x, "--4294967295,18446744073709551615--\0", 36) == 0);
    obj_sds_free(x);
    /* range */
    x = obj_sds_new("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    obj_sds_range(x, 0, 51);
    obj_assert(obj_memcmp(x, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 52) == 0);
    obj_sds_range(x, 1, 25);
    obj_assert(obj_memcmp(x, "bcdefghijklmnopqrstuvwxyz", 25) == 0);
    obj_sds_range(x, -2, -1);
    obj_assert(obj_memcmp(x, "yz", 2) == 0);
    obj_sds_range(x, -1, -2);
    obj_assert(obj_memcmp(x, "\0", 1) == 0);
    obj_sds_free(x);
    /* cmp */
    x = obj_sds_new("aar");
    y = obj_sds_new("bar");
    obj_assert(obj_sds_cmp(x, y) < 0);
    obj_sds_free(x);
    obj_sds_free(y);
    x = obj_sds_new("aar");
    y = obj_sds_new("aar");
    obj_assert(obj_sds_cmp(x, y) == 0);
    obj_sds_free(x);
    obj_sds_free(y);
    /* join */

    printf("ALL TESTS FINISHED.\n");
    return 0;
}