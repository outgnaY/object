#ifndef OBJ_QUERY_H
#define OBJ_QUERY_H

#include "obj_core.h"

/* process query request */

typedef struct obj_query_request_s obj_query_request_t;


struct obj_query_request_s {
    /* filter */
    obj_bson_t filter;
    /* projection */
    obj_bson_t projection;
    /* sort */
    obj_bson_t sort;
    /* hint */
    /* obj_bson_t hint; */
    /* skip, -1 if not assign */
    int skip;
    /* limit, -1 if not assign */
    int limit;
    /* full name. e.x. db.coll */
    obj_namespace_string_t full_name;
};

obj_status_with_t obj_query_parse_from_find_cmd(obj_bson_t *cmd);
void obj_query_request_dump(obj_query_request_t *qr);

#endif  /* OBJ_QUERY_H */