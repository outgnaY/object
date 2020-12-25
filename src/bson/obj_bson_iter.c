#include "bson/obj_bson.h"
#include "bson/obj_bson_iter.h"

/* must validate bson before iter!!! */

obj_bool_t obj_bson_iter_init(obj_bson_iter_t *iter, obj_bson_t *bson) {
    obj_assert(iter);
    obj_assert(bson);
    if (bson->len < 5) {
        return false;
    }
    
    return true;
}


/*
double obj_bson_iter_double(obj_bson_iter_t *iter) {
    obj_assert(iter);
    if 
}
*/

/* check if has next */
obj_bool_t obj_bson_iter_has_next(obj_bson_iter_t *iter) {

}

void obj_bson_iter_next(obj_bson_iter_t *iter) {

}



