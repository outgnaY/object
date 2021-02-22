#ifndef OBJ_EMBEDDED_LIST_H
#define OBJ_EMBEDDED_LIST_H


#define OBJ_EMBEDDED_LIST_BASE_NODE_T(TYPE)\
struct {\
    int count;      /* count of nodes in list */\
    TYPE *start;\
    TYPE *end;\
}\

#define OBJ_EMBEDDED_LIST_NODE_T(TYPE)\
struct {\
    TYPE *prev;\
    TYPE *next;\
}\

#define OBJ_EMBEDDED_LIST_INIT(BASE)\
{\
    (BASE).count = 0;\
    (BASE).start = NULL;\
    (BASE).end = NULL;\
}\

#define OBJ_EMBEDDED_LIST_ADD_FIRST(NAME, BASE, N)\
{\
    ((BASE).count)++;\
	((N)->NAME).next = (BASE).start;\
	((N)->NAME).prev = NULL;\
	if ((BASE).start != NULL) {\
		(((BASE).start)->NAME).prev = (N);\
	}\
	(BASE).start = (N);\
	if ((BASE).end == NULL) {\
		(BASE).end = (N);\
	}\
}\

#define OBJ_EMBEDDED_LIST_ADD_LAST(NAME, BASE, N)\
{\
    ((BASE).count)++;\
    ((N)->NAME).prev = (BASE).end;\
	((N)->NAME).next = NULL;\
	if ((BASE).end != NULL) {\
		(((BASE).end)->NAME).next = (N);\
	}\
	(BASE).end = (N);\
	if ((BASE).start == NULL) {\
		(BASE).start = (N);\
	}\
}\

#define OBJ_EMBEDDED_LIST_INSERT_AFTER(NAME, BASE, NODE1, NODE2)\
{\
	((BASE).count)++;\
	((NODE2)->NAME).prev = (NODE1);\
	((NODE2)->NAME).next = ((NODE1)->NAME).next;\
	if (((NODE1)->NAME).next != NULL) {\
		((((NODE1)->NAME).next)->NAME).prev = (NODE2);\
	}\
	((NODE1)->NAME).next = (NODE2);\
	if ((BASE).end == (NODE1)) {\
		(BASE).end = (NODE2);\
	}\
}\

#define OBJ_EMBEDDED_LIST_REMOVE(NAME, BASE, N)\
{\
    obj_assert((BASE).count > 0);\
    ((BASE).count)--;\
    if (((N)->NAME).next != NULL) {\
        ((((N)->NAME).next)->NAME).prev = ((N)->NAME).prev;\
    } else {\
        (BASE).end = ((N)->NAME).prev;\
    }\
    if (((N)->NAME).prev != NULL) {\
        ((((N)->NAME).prev)->NAME).next = ((N)->NAME).next;\
    } else {\
        (BASE).start = ((N)->NAME).next;\
    }\
}\

#define OBJ_EMBEDDED_LIST_GET_NEXT(NAME, N) (((N)->NAME).next)

#define OBJ_EMBEDDED_LIST_GET_PREV(NAME, N) (((N)->NAME).prev)

#define OBJ_EMBEDDED_LIST_GET_LEN(BASE) (BASE).count

#define OBJ_EMBEDDED_LIST_GET_FIRST(BASE) (BASE).start

#define OBJ_EMBEDDED_LIST_GET_LAST(BASE) (BASE).end


#endif  /* OBJ_EMBEDDED_LIST_H */