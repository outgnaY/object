#ifndef OBJ_QUERY_PLAN_ITER_H
#define OBJ_QUERY_PLAN_ITER_H

#include "obj_core.h"


typedef enum obj_query_plan_iter_node_type obj_query_plan_iter_node_type_t;
typedef struct obj_expr_id_pair_s obj_expr_id_pair_t;
typedef struct obj_index_expr_list_pair_s obj_index_expr_list_pair_t;
typedef struct obj_query_plan_iter_s obj_query_plan_iter_t;
typedef union obj_query_plan_iter_base_node obj_query_plan_iter_base_node_t;
typedef struct obj_query_plan_iter_and_node_s obj_query_plan_iter_and_node_t;
typedef struct obj_query_plan_iter_or_node_s obj_query_plan_iter_or_node_t;
typedef struct obj_query_plan_iter_index_assignment_s obj_query_plan_iter_index_assignment_t;
typedef struct obj_query_plan_iter_and_iter_state_s obj_query_plan_iter_and_iter_state_t;


enum obj_query_plan_iter_node_type {
    OBJ_QUERY_PLAN_ITER_NODE_TYPE_AND,
    OBJ_QUERY_PLAN_ITER_NODE_TYPE_OR
};

/* expr->memo_id pair */
struct obj_expr_id_pair_s {
    obj_expr_base_expr_t *expr;
    int id;
};

struct obj_index_expr_list_pair_s {
    int index_id;
    obj_array_t expr_list;
};


/* enumerate possible query plans */
struct obj_query_plan_iter_s {
    /* indexes */
    obj_array_t *indexes;
    /* expression root */
    obj_expr_base_expr_t *root;
    /* if we have done enumeration */
    obj_bool_t done;
    /* expr->memo_id */
    obj_prealloc_map_t expr_to_id;
    /* memo_id->iter_node* */
    obj_array_t id_to_node;
};

/* tree to enumerate plan */
union obj_query_plan_iter_base_node {
    obj_query_plan_iter_node_type_t node_type;
    obj_query_plan_iter_and_node_t and_node;
    obj_query_plan_iter_or_node_t or_node;
};


/* corresponding to and expression node */
struct obj_query_plan_iter_and_node_s {
    int counter;
    /* [iter_state] */
    obj_array_t choices;
};

/* corresponding to or expression node */
struct obj_query_plan_iter_or_node_s {
    int counter;
    /* [id] */
    obj_array_t subnodes;
};

/* used by and */
struct obj_query_plan_iter_index_assignment_s {
    /* [expr*] */
    obj_array_t preds;
    /* [position] */
    obj_array_t positions;
    /* index */
    int index;
};

/* iterate through and */
struct obj_query_plan_iter_and_iter_state_s {
    /* [index_assignment] */
    obj_array_t assignments;
    /* [id] */
    obj_array_t subnodes
};


#endif  /* OBJ_QUERY_PLAN_ITER_H */