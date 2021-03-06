#include "util/obj_math.h"

obj_size_t obj_next_power_of_two(obj_size_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}


int obj_sgn(int i) {
    if (i == 0) {
        return 0;
    }
    return (i > 0 ? 1 : -1);
}