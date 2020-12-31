#include "obj_core.h"

static obj_conn_queue_item_t *obj_conn_queue_item_freelist;
static pthread_mutex_t obj_conn_queue_item_freelist_lock;


/* initializes a connection queue */
static void obj_conn_queue_init(obj_conn_queue_t *cq) {
    pthread_mutex_init(&cq->lock, NULL);
    cq->head = NULL;
    cq->tail = NULL;
}

/* pop an item in a connection queue */
static obj_conn_queue_item_t *obj_conn_queue_pop(obj_conn_queue_t *cq) {
    obj_conn_queue_item_t *item;
    pthread_mutex_lock(&cq->lock);
    item = cq->head;
    if (item != NULL) {
        cq->head = item->next;
        if (cq->head == NULL) {
            cq->tail = NULL;
        }
    }
    pthread_mutex_unlock(&cq->lock);
    return item;
}

/* push an item to a connection queue */
static void obj_conn_queue_push(obj_conn_queue_t *cq, obj_conn_queue_item_t *item) {
    item->next = NULL;
    pthread_mutex_lock(&cq->lock);
    if (cq->tail == NULL) {
        cq->head = item;
    } else {
        cq->tail->next = item;
    }
    cq->tail = item;
    pthread_mutex_unlock(&cq->lock);
}

/* return a new connection queue item */
static obj_conn_queue_item_t *obj_conn_queue_new_item() {
    obj_conn_queue_item_t *item = NULL;
    pthread_mutex_lock(&obj_conn_queue_item_freelist_lock);
    if (obj_conn_queue_item_freelist) {
        item = obj_conn_queue_item_freelist;
        obj_conn_queue_item_freelist = item->next;
    }
    pthread_mutex_unlock(&obj_conn_queue_item_freelist_lock);
    if (item == NULL) {
        int i;
        item = obj_alloc(sizeof(obj_conn_queue_item_t) * OBJ_CONN_QUEUE_ITEMS_PER_ALLOC);
        if (item == NULL) {
            return NULL;
        }
        /* link all the new allocated items except the first one */
        for (i = 2; i < OBJ_CONN_QUEUE_ITEMS_PER_ALLOC; i++) {
            item[i - 1].next = &item[i];
        }
        pthread_mutex_lock(&obj_conn_queue_item_freelist_lock);
        item[OBJ_CONN_QUEUE_ITEMS_PER_ALLOC - 1].next = obj_conn_queue_item_freelist;
        obj_conn_queue_item_freelist = &item[1];
        pthread_mutex_unlock(&obj_conn_queue_item_freelist_lock);
    }
    return item;
}

/* free a connection queue item */
static void obj_conn_queue_free_item(obj_conn_queue_item_t *item) {
    pthread_mutex_lock(&obj_conn_queue_item_freelist_lock);
    item->next = obj_conn_queue_item_freelist;
    obj_conn_queue_item_freelist = item;
    pthread_mutex_unlock(&obj_conn_queue_item_freelist_lock);
}


