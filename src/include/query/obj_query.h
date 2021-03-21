#ifndef OBJ_QUERY_H
#define OBJ_QUERY_H

#include "obj_core.h"

/* process query request */

typedef struct obj_query_request_s obj_query_request_t;
typedef struct obj_standard_query_s obj_standard_query_t;

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
    char *full_name;
};

/* standard query */
struct obj_standard_query_s {
    /* query request */
    obj_query_request_t *qr;
    /* filter expression */
    obj_expr_base_expr_t *root;
};

/* query request methods */
obj_status_with_t obj_query_parse_from_find_cmd(obj_bson_t *cmd);
void obj_query_request_dump(obj_query_request_t *qr);

/* standard query methods */
obj_status_with_t obj_query_standardize(obj_query_request_t *qr);
void obj_standard_query_init(obj_standard_query_t *sq, obj_query_request_t *qr, obj_expr_base_expr_t *root);
void obj_standard_query_dump(obj_standard_query_t *sq);


#endif  /* OBJ_QUERY_H */