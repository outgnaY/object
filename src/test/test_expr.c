#include "obj_core.h"

int main() {
    obj_global_mem_context_init();
    /* ********** correct query format ********** */
    /* simple query1 */
    /* {"a": 45} */
    obj_bson_t *query1 = obj_bson_init();
    obj_bson_append_int32(query1, "a", 1, 45);
    obj_status_with_t status1 = obj_expr_parse(query1);
    obj_assert(obj_status_isok(&status1));
    printf("**********query 1:**********\n");
    obj_expr_dump((obj_expr_base_t *)status1.data);
    /* simple query2 */
    /* {"x1": 5, "x2": 89} */
    obj_bson_t *query2 = obj_bson_init();
    obj_bson_append_int32(query2, "x1", 2, 5);
    obj_bson_append_int64(query2, "x2", 2, 89);
    obj_status_with_t status2 = obj_expr_parse(query2);
    obj_assert(obj_status_isok(&status2));
    printf("**********query 2:**********\n");
    obj_expr_dump((obj_expr_base_t *)status2.data);
    /* simple query3 */
    /* {"x": {"$lt": 8}} */
    obj_bson_t *query3 = obj_bson_init();
    obj_bson_t *cond3 = obj_bson_init();
    obj_bson_append_int32(cond3, "$lt", 3, 8);
    obj_bson_append_object(query3, "x", 1, cond3);
    obj_status_with_t status3 = obj_expr_parse(query3);
    obj_assert(obj_status_isok(&status3));
    printf("**********query 3:**********\n");
    obj_expr_dump((obj_expr_base_t *)status3.data);
    /* simple query4 */
    /* {"x": {"$lt": 8, "$gt": 5}} */
    obj_bson_t *query4 = obj_bson_init();
    obj_bson_t *cond4 = obj_bson_init();
    obj_bson_append_int32(cond4, "$lt", 3, 8);
    obj_bson_append_int32(cond4, "$gt", 3, 5);
    obj_bson_append_object(query4, "x", 1, cond4);
    obj_status_with_t status4 = obj_expr_parse(query4);
    obj_assert(obj_status_isok(&status4));
    printf("**********query 4:**********\n");
    obj_expr_dump((obj_expr_base_t *)status4.data);
    /* simple query5 */
    /* {"x": {"$neq": 9}} */
    obj_bson_t *query5 = obj_bson_init();
    obj_bson_t *cond5 = obj_bson_init();
    obj_bson_append_int32(cond5, "$neq", 4, 9);
    obj_bson_append_object(query5, "x", 1, cond5);
    obj_status_with_t status5 = obj_expr_parse(query5);
    obj_assert(obj_status_isok(&status5));
    printf("**********query 5 **********\n");
    obj_expr_dump((obj_expr_base_t *)status5.data);
    /* simple query6 */
    /* {"x": {"$not": {"$lte": 5, "$gt": 4}}} */
    obj_bson_t *query6 = obj_bson_init();
    obj_bson_t *cond6 = obj_bson_init();
    obj_bson_append_int32(cond6, "$lte", 4, 5);
    obj_bson_append_int32(cond6, "$gt", 3, 4);
    obj_bson_t *not6 = obj_bson_init();
    obj_bson_append_object(not6, "$not", 4, cond6);
    obj_bson_append_object(query6, "x", 1, not6);
    obj_status_with_t status6 = obj_expr_parse(query6);
    obj_assert(obj_status_isok(&status6));
    printf("**********query 6 **********\n");
    obj_expr_dump((obj_expr_base_t *)status6.data);
    /* complicated query7 */
    /* {"$and": [{"x": 4}, {"y": 5}]} */
    obj_bson_t *query7 = obj_bson_init();
    obj_bson_t *arr7 = obj_bson_init();
    obj_bson_t *cond71 = obj_bson_init();
    obj_bson_t *cond72 = obj_bson_init();
    obj_bson_append_int32(cond71, "x", 1, 4);
    obj_bson_append_int32(cond72, "y", 1, 5);
    obj_bson_append_object(arr7, "0", 1, cond71);
    obj_bson_append_object(arr7, "1", 1, cond72);
    obj_bson_append_array(query7, "$and", 4, arr7);
    obj_status_with_t status7 = obj_expr_parse(query7);
    obj_assert(obj_status_isok(&status7));
    printf("**********query 7 **********\n");
    obj_expr_dump((obj_expr_base_t *)status7.data);
    /* complicated query8 */
    /* {"$or": [{"x": 4}, {"y": 5}]} */
    obj_bson_t *query8 = obj_bson_init();
    obj_bson_t *arr8 = obj_bson_init();
    obj_bson_t *cond81 = obj_bson_init();
    obj_bson_t *cond82 = obj_bson_init();
    obj_bson_append_int32(cond81, "x", 1, 4);
    obj_bson_append_int32(cond82, "y", 1, 5);
    obj_bson_append_object(arr8, "0", 1, cond81);
    obj_bson_append_object(arr8, "1", 1, cond82);
    obj_bson_append_array(query8, "$or", 3, arr8);
    obj_status_with_t status8 = obj_expr_parse(query8);
    obj_assert(obj_status_isok(&status8));
    printf("**********query 8 **********\n");
    obj_expr_dump((obj_expr_base_t *)status8.data);
    /* complicated query9 */
    /* {"$and": [{"x": 4}, {"$or": [{"y": 5}, {"z": {"$lte": 7}}]}]} */
    obj_bson_t *query9 = obj_bson_init();
    obj_bson_t *andcond91 = obj_bson_init();
    obj_bson_t *andcond92 = obj_bson_init();
    obj_bson_t *andarr = obj_bson_init();
    obj_bson_t *orarr = obj_bson_init();
    obj_bson_t *orcond91 = obj_bson_init();
    obj_bson_t *orcond92 = obj_bson_init();
    obj_bson_t *orcondlte = obj_bson_init();
    obj_bson_append_int32(andcond91, "x", 1, 4);
    obj_bson_append_int32(orcond91, "y", 1, 5);
    obj_bson_append_int32(orcondlte, "$lte", 4, 7);
    obj_bson_append_object(orcond92, "z", 1, orcondlte);
    obj_bson_append_object(orarr, "0", 1, orcond91);
    obj_bson_append_object(orarr, "1", 1, orcond92);
    obj_bson_append_array(andcond92, "$or", 3, orarr);
    obj_bson_append_object(andarr, "0", 1, andcond91);
    obj_bson_append_object(andarr, "1", 1, andcond92);
    obj_bson_append_array(query9, "$and", 4, andarr);
    obj_status_with_t status9 = obj_expr_parse(query9);
    obj_assert(obj_status_isok(&status9));
    printf("**********query 9 **********\n");
    obj_expr_dump((obj_expr_base_t *)status9.data);
    /* **********wrong format query ********** */
    /* wrong format query10 */
    /* {"x": {"$lt": true}} */
    obj_bson_t *query10 = obj_bson_init();
    obj_bson_t *cond10 = obj_bson_init();
    obj_bson_append_bool(cond10, "$lt", 3, true);
    obj_bson_append_object(query10, "x", 1, cond10);
    obj_status_with_t status10 = obj_expr_parse(query10);
    obj_assert(!obj_status_isok(&status10));
    printf("**********query 10:**********\n");
    /* wrong format query11 */
    /* {"x": {"$not": 5}} */
    obj_bson_t *query11 = obj_bson_init();
    obj_bson_t *not11 = obj_bson_init();
    obj_bson_append_int32(not11, "$not", 4, 5);
    obj_bson_append_object(query11, "x", 1, not11);
    obj_status_with_t status11 = obj_expr_parse(query11);
    obj_assert(!obj_status_isok(&status11));
    printf("**********query 11:**********\n");
    printf("********** all tests passed! **********\n");
    return 0;
}