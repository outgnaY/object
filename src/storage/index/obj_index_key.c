#include "obj_core.h"

/* 
 * for key pattern {a: 1, b: -1} 
 * get(0) == 1; get(1) == -1
 */
int obj_index_key_order_get(obj_index_key_order_t key_order, int i) {
    return ((1 << i) & key_order) ? -1 : 1;
}

unsigned obj_index_key_order_descending(obj_index_key_order_t key_order, unsigned mask) {
    return key_order & mask;
}