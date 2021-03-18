#include "obj_core.h"


int main() {
    obj_global_mem_context_init();
    /* interval test */
    /*
    printf("********** interval test case 1: **********\n");
    obj_bson_t *bson1 = OBJ_BSON_BCON_NEW(
        "", OBJ_BSON_BCON_INT32(5),
        "", OBJ_BSON_BCON_INT32(8)
    );
    obj_interval_t interval1;
    obj_interval_init(&interval1, bson1, false, true);
    obj_bson_t *bson2 = OBJ_BSON_BCON_NEW(
        "", OBJ_BSON_BCON_INT32(5),
        "", OBJ_BSON_BCON_INT32(8)
    );
    obj_interval_t interval2;
    obj_interval_init(&interval2, bson2, true, true);
    printf("compare: %d\n", obj_interval_compare(&interval1, &interval2));
    obj_interval_intersect(&interval1, &interval2, obj_interval_compare(&interval1, &interval2));
    obj_interval_dump(&interval1);
    */
    /* ordered interval list test */
    /* [[1, 2], [4, 5]] */
    /* [[2, 3], [5, 6]] */
    obj_interval_t interval1, interval2, interval3, interval4;
    obj_bson_t *bson1, *bson2, *bson3, *bson4;
    bson1 = OBJ_BSON_BCON_NEW(
        "", OBJ_BSON_BCON_INT32(1),
        "", OBJ_BSON_BCON_INT32(2)
    );
    obj_interval_init(&interval1, bson1, true, true);
    bson2 = OBJ_BSON_BCON_NEW(
        "", OBJ_BSON_BCON_INT32(4),
        "", OBJ_BSON_BCON_INT32(5)
    );
    obj_interval_init(&interval2, bson2, true, true);
    bson3 = OBJ_BSON_BCON_NEW(
        "", OBJ_BSON_BCON_INT32(2),
        "", OBJ_BSON_BCON_INT32(3)
    );
    obj_interval_init(&interval3, bson3, true, true);
    bson4 = OBJ_BSON_BCON_NEW(
        "", OBJ_BSON_BCON_INT32(5),
        "", OBJ_BSON_BCON_INT32(6)
    );
    obj_interval_init(&interval4, bson4, true, true);
    obj_ordered_interval_list_t oil1, oil2;
    obj_array_init(&oil1.intervals, sizeof(obj_interval_t));
    obj_array_init(&oil2.intervals, sizeof(obj_interval_t));
    obj_array_push_back(&oil1.intervals, &interval1);
    obj_array_push_back(&oil1.intervals, &interval2);
    obj_array_push_back(&oil2.intervals, &interval3);
    obj_array_push_back(&oil2.intervals, &interval4);
    /* obj_ordered_interval_list_intersect(&oil1, &oil2); */
    obj_ordered_interval_list_union(&oil1, &oil2);
    int i;
    for (i = 0; i < oil1.intervals.size; i++) {
        printf("+++ %d th interval: +++\n", i);
        obj_interval_t *temp_interval = (obj_interval_t *)obj_array_get_index(&oil1.intervals, i);
        obj_interval_dump(temp_interval);
    }
    return 0;
}