#ifndef OBJ_EXPR_H
#define OBJ_EXPR_H

#include "obj_core.h"

typedef enum obj_expr_type obj_expr_type_t;
typedef struct obj_expr_methods_s obj_expr_methods_t;
typedef enum obj_expr_tag_type obj_expr_tag_type_t;
typedef struct obj_expr_tag_s obj_expr_tag_t;
typedef struct obj_expr_relevant_tag_s obj_expr_relevant_tag_t;
typedef struct obj_expr_index_tag_s obj_expr_index_tag_t;
typedef struct obj_expr_base_expr_s obj_expr_base_expr_t;
typedef struct obj_expr_tree_expr_s obj_expr_tree_expr_t;
typedef struct obj_expr_compare_expr_s obj_expr_compare_expr_t;
typedef struct obj_expr_not_expr_s obj_expr_not_expr_t;

enum obj_expr_type {
    /* logical */
    OBJ_EXPR_TYPE_AND,
    OBJ_EXPR_TYPE_OR,
    /* compare */
    OBJ_EXPR_TYPE_EQ,
    OBJ_EXPR_TYPE_LTE,
    OBJ_EXPR_TYPE_LT,
    OBJ_EXPR_TYPE_GT,
    OBJ_EXPR_TYPE_GTE,
    /* negation */
    /*
    OBJ_EXPR_TYPE_NOT,
    OBJ_EXPR_TYPE_NOR,
    OBJ_EXPR_TYPE_NEQ
    */
};

struct obj_expr_methods_s {
    /* child num */
    int (*num_child)(obj_expr_base_expr_t *expr);
    /* get i-th child */
    obj_expr_base_expr_t *(*get_child)(obj_expr_base_expr_t *expr, int i);
    /* destroy */
    void (*destroy)(obj_expr_base_expr_t *expr);
    /* path */
    obj_stringdata_t (*get_path)(obj_expr_base_expr_t *expr);
};

enum obj_expr_tag_type {
    OBJ_EXPR_TAG_TYPE_RELEVANT,
    OBJ_EXPR_TAG_TYPE_INDEX
};

/* tag bind on the expr */
struct obj_expr_tag_s {
    obj_expr_tag_type_t type;
};

/* internal */
struct obj_expr_relevant_tag_s {
    obj_expr_tag_t base;
    obj_array_t first;
    obj_array_t not_first;
    obj_stringdata_t path;
};

/* index tag */
struct obj_expr_index_tag_s {
    obj_expr_tag_t base;
    int index;
    /* for compound index */
    int pos;
};

struct obj_expr_base_expr_s {
    obj_expr_type_t type;
    obj_expr_methods_t *methods;
    obj_expr_tag_t *tag;
};

/* AND OR NOR */
struct obj_expr_tree_expr_s {
    obj_expr_base_expr_t base;
    obj_array_t expr_list;
};

/* EQ LTE LT GT GTE */
struct obj_expr_compare_expr_s {
    obj_expr_base_expr_t base;
    /* char *path; */
    obj_stringdata_t path;
    obj_bson_value_t value;
};

/* NOT */
/*
struct obj_expr_not_expr_s {
    obj_expr_base_expr_t base;
    obj_expr_base_expr_t *expr;
};
*/

/* tags */
void obj_expr_sort_using_tags(obj_expr_base_expr_t *expr);
void obj_expr_tag_dump(obj_expr_tag_t *tag);
void obj_expr_set_tag(obj_expr_base_expr_t *expr, obj_expr_tag_t *tag);
void obj_expr_tag_destroy(obj_expr_tag_t *tag);
void obj_expr_reset_tag(obj_expr_base_expr_t *expr);
obj_expr_relevant_tag_t *obj_expr_relevant_tag_create();
obj_expr_index_tag_t *obj_expr_index_tag_create(int i);
obj_expr_index_tag_t *obj_expr_index_tag_compound_create(int i, int p);
/* not expression */
/*
obj_expr_base_expr_t *obj_expr_not_expr_create(obj_expr_base_expr_t *child);
*/
/* compare expression */
obj_expr_base_expr_t *obj_expr_compare_expr_create(const char *path, obj_expr_type_t type, const obj_bson_value_t *value);
/* tree expression */
obj_expr_base_expr_t *obj_expr_tree_expr_create(obj_expr_type_t type);
obj_bool_t obj_expr_tree_expr_add_child(obj_expr_base_expr_t *expr, obj_expr_base_expr_t *child);
/* debug */
void obj_expr_dump(obj_expr_base_expr_t *expr);

#endif  /* OBJ_EXPR_H */