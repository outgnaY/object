#ifndef OBJ_QUERY_PLANNER_H
#define OBJ_QUERY_PLANNER_H

#include "obj_core.h"

/* query planner */


obj_status_with_t obj_query_planner_plan(obj_standard_query_t *sq, obj_array_t *indexes);


#endif  /* OBJ_QUERY_PLANNER_H */