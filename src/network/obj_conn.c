#include "obj_core.h"

static obj_conn_queue_item_t *obj_conn_queue_item_freelist;
static pthread_mutex_t obj_conn_queue_item_freelist_lock;
static pthread_mutex_t obj_conn_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile obj_bool_t obj_conn_allow_new_conns = true;     /* allow new connection flag */
static struct event obj_conn_maxconnsevent;

/* globals */
int obj_conn_max_fds;                       /* maximum fds */
obj_conn_t **obj_conn_conns;                /* connection array */
obj_conn_t *obj_conn_listen_conn = NULL;

static void obj_conn_queue_push(obj_conn_queue_t *cq, obj_conn_queue_item_t *item);
static obj_conn_queue_item_t *obj_conn_queue_new_item();
static void obj_conn_reset_cmd_handler(obj_conn_t *c);
static obj_conn_read_result_t obj_conn_read(obj_conn_t *c);
static obj_bool_t obj_conn_has_pending_reply(obj_conn_t *c);
static obj_bool_t obj_conn_add_reply_to_buffer(obj_conn_t *c, obj_msg_reply_t *reply);
static obj_bool_t obj_conn_add_reply_to_list(obj_conn_t *c, obj_msg_reply_t *reply);
static obj_conn_write_result_t obj_conn_write(obj_conn_t *c);
static void obj_drive_machine(obj_conn_t *c);
static void obj_conn_maxconns_handler(const evutil_socket_t fd, const short which, void *arg);
static void obj_conn_cleanup(obj_conn_t *c);
static void obj_conn_free(obj_conn_t *c);



void obj_conn_queue_item_freelist_init() {
    obj_conn_queue_item_freelist = NULL;
    pthread_mutex_init(&obj_conn_queue_item_freelist_lock, NULL);
}

/* initializes a connection queue */
void obj_conn_queue_init(obj_conn_queue_t *cq) {
    pthread_mutex_init(&cq->lock, NULL);
    cq->head = NULL;
    cq->tail = NULL;
}

/* pop an item in a connection queue */
obj_conn_queue_item_t *obj_conn_queue_pop(obj_conn_queue_t *cq) {
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
        item = (obj_conn_queue_item_t *)obj_alloc(sizeof(obj_conn_queue_item_t) * OBJ_CONN_QUEUE_ITEMS_PER_ALLOC);
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
void obj_conn_queue_free_item(obj_conn_queue_item_t *item) {
    pthread_mutex_lock(&obj_conn_queue_item_freelist_lock);
    item->next = obj_conn_queue_item_freelist;
    obj_conn_queue_item_freelist = item;
    pthread_mutex_unlock(&obj_conn_queue_item_freelist_lock);
}


/* ---------- connection related ---------- */

/* reset */
static void obj_conn_reset_cmd_handler(obj_conn_t *c) {
    if (obj_buffer_readable_bytes(c->inbuf) > 0) {
        /* printf("reset_cmd_handler: parse_cmd\n"); */
        obj_conn_set_state(c, OBJ_CONN_PARSE_CMD);
    } else if (obj_conn_has_pending_reply(c)) {
        /* if reply list is not empty, try to send */
        /* printf("reset_cmd_handler: conn_write\n"); */
        obj_conn_set_state(c, OBJ_CONN_WRITE);
    } else {
        /* nothing to do now, just waiting */
        /* printf("reset_cmd_handler: conn_waiting\n"); */
        obj_conn_set_state(c, OBJ_CONN_WAITING);
    }
}


/* read from network */
static obj_conn_read_result_t obj_conn_read(obj_conn_t *c) {
    obj_conn_read_result_t ret = OBJ_CONN_READ_NO_DATA_RECEIVED;
    int n;
    int saved_errno;
    obj_bool_t res;
    /* read */
    while (true) {
        /* try to read into input buffer */
        obj_buffer_read_fd(c->inbuf, c->sfd, &saved_errno, &n);
        if (n > 0) {
            ret = OBJ_CONN_READ_DATA_RECEIVED;
            break;
        }
        /* an error occurred, or connection closed by client */
        if (n == 0) {
            return OBJ_CONN_READ_ERROR;
        }
        if (n < 0) {
            if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
                /* exit */
                break;
            }
            return OBJ_CONN_READ_ERROR;
        }
    }
    return ret;
}

static obj_bool_t obj_conn_has_pending_reply(obj_conn_t *c) {
    return obj_buffer_readable_bytes(c->outbuf) > 0 || obj_list_length(c->reply_list) > 0;
}

/* add reply to buffer */
static obj_bool_t obj_conn_add_reply_to_buffer(obj_conn_t *c, obj_msg_reply_t *reply) {
    /* printf("add reply to buffer, len: %d\n", reply->header.len); */
    int available = 0;
    obj_int32_t len;
    int i;
    /* if reply list is not empty, we can't add anything more to buffer */
    if (!obj_list_is_empty(c->reply_list)) {
        return false;
    }
    available = obj_buffer_writable_bytes(c->outbuf);
    len = reply->header.len;
    if (len > available) {
        return false;
    }
    /* append reply */
    /* header */
    obj_buffer_append_int32(c->outbuf, reply->header.len);
    obj_buffer_append_int32(c->outbuf, reply->header.opCode);
    /* response_flags */
    obj_buffer_append_int32(c->outbuf, reply->response_flags);
    /* cursor_id */
    obj_buffer_append_int64(c->outbuf, reply->cursor_id);
    /* start_from */
    obj_buffer_append_int32(c->outbuf, reply->start_from);
    /* num_return */
    obj_buffer_append_int32(c->outbuf, reply->num_return);
    /* objects */
    for (i = 0; i < reply->num_return; i++) {
        obj_buffer_append_bson(c->outbuf, reply->objects[i]);
    }
    
    return true;
}

/* add reply to list */
static obj_bool_t obj_conn_add_reply_to_list(obj_conn_t *c, obj_msg_reply_t *reply) {
    /* printf("add reply to list, len:%d\n", reply->header.len); */
    obj_list_node_t *tail_node = obj_list_get_tail(c->reply_list);
    obj_conn_reply_block_t *tail = tail_node ? (obj_conn_reply_block_t *)obj_list_node_value(tail_node) : NULL;
    obj_buffer_t *buf;
    obj_bool_t res = false;
    int i;
    int size = reply->header.len;
    if (tail) {
        if (obj_buffer_writable_bytes(tail->buf) < size) {
            goto new_block;
        }
        goto add;
    } 
new_block:
    tail = obj_alloc(sizeof(obj_conn_reply_block_t));
    if (tail == NULL) {
        goto clean;
    }
    if (size < OBJ_BUFFER_INIT_SIZE) {
        size = OBJ_BUFFER_INIT_SIZE;
    }
    buf = obj_buffer_create_with_size(size);
    if (buf == NULL) {
        goto clean;
    }
    tail->buf = buf;
    /* link to last */
    obj_list_add_node_tail(c->reply_list, tail);
add:
    /* have enough space, add reply */
    /* header */
    obj_buffer_append_int32(tail->buf, reply->header.len);
    obj_buffer_append_int32(tail->buf, reply->header.opCode);
    /* response_flags */
    obj_buffer_append_int32(tail->buf, reply->response_flags);
    /* cursor_id */
    obj_buffer_append_int64(tail->buf, reply->cursor_id);
    /* start_from */
    obj_buffer_append_int32(tail->buf, reply->start_from);
    /* num_return */
    obj_buffer_append_int32(tail->buf, reply->num_return);
    /* objects */
    for (i = 0; i < reply->num_return; i++) {
        obj_buffer_append_bson(tail->buf, reply->objects[i]);
    }
    return true;
clean:
    if (tail != NULL) {
        obj_free(tail);
    }
    if (buf != NULL) {
        obj_buffer_destroy(buf);
    }
    return false;
}

/* add reply */
obj_bool_t obj_conn_add_reply(obj_conn_t *c, obj_msg_reply_t *reply) {
    if (!obj_conn_add_reply_to_buffer(c, reply)) {
        return obj_conn_add_reply_to_list(c, reply);
    }
    return true;
}

/* send data */
static obj_conn_write_result_t obj_conn_write(obj_conn_t *c) {
    obj_conn_write_result_t ret = OBJ_CONN_WRITE_INCOMPLETE;
    int nwrite = 0;
    int saved_errno;
    int total_write = 0;
    obj_conn_reply_block_t *block;
    while (obj_conn_has_pending_reply(c)) {
        /* check output buffer and reply list */
        if (obj_buffer_readable_bytes(c->outbuf) > 0) {
            nwrite = obj_buffer_write_fd(c->outbuf, c->sfd, &saved_errno);
            if (nwrite <= 0) {
                break;
            }
            total_write += nwrite;
        } else {
            block = (obj_conn_reply_block_t *)obj_list_node_value(obj_list_get_head(c->reply_list));
            /* safe check */
            if (obj_buffer_readable_bytes(block->buf) == 0) {
                obj_list_del_node(c->reply_list, obj_list_get_head(c->reply_list));
                continue;
            }
            nwrite = obj_buffer_write_fd(block->buf, c->sfd, &saved_errno);
            if (nwrite <= 0) {
                break;
            }
            total_write += nwrite;
            /* check if we sent all data in the buf */
            if (obj_buffer_readable_bytes(block->buf) == 0) {
                obj_list_del_node(c->reply_list, obj_list_get_head(c->reply_list));
            }
        }
        /* avoid starving other connections */
        if (total_write > OBJ_CONN_WRITE_MAX_BYTES_PER_EVENT) {
            break;
        }
    }
    if (nwrite >= 0) {
        if (!obj_conn_has_pending_reply(c)) {
            return OBJ_CONN_WRITE_COMPLETE;
        } else {
            return OBJ_CONN_WRITE_INCOMPLETE;
        }
    }
    if (nwrite < 0 && (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK)) {
        if (!obj_conn_update_event(c, EV_WRITE | EV_PERSIST)) {
            if (obj_settings.verbose > 0) {
                fprintf(stderr, "can't update event\n");
            }
            /* obj_conn_set_state(c, OBJ_CONN_CLOSING); */
            return OBJ_CONN_WRITE_HARD_ERROR;
        }
        return OBJ_CONN_WRITE_SOFT_ERROR;
    }
    /* other errors */
    if (obj_settings.verbose > 0) {
        perror("failed to write, and not due to blocking");
    }
    /* obj_conn_set_state(c, OBJ_CONN_CLOSING); */
    return OBJ_CONN_WRITE_HARD_ERROR;
}

/* drive the state machine */
static void obj_drive_machine(obj_conn_t *c) {
    obj_bool_t stop = false;
    int sfd;
    socklen_t addrlen;
    struct sockaddr_in addr;
    obj_bool_t reject = false;
    int nreqs = obj_settings.max_reqs_per_event;
    obj_conn_read_result_t read_res;
    obj_conn_write_result_t write_res;
    while (!stop) {
        switch (c->state) {
            case OBJ_CONN_LISTENING:
                addrlen = sizeof(addr);
                bzero(&addr, addrlen);
                sfd = accept(c->sfd, (struct sockaddr *)&addr, &addrlen);
                if (sfd == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    stop = true;
                    } else if (errno == EMFILE) {
                        if (obj_settings.verbose > 0) {
                            fprintf(stderr, "too many open connections\n");
                        }
                        obj_conn_accept_new_conns(false);
                        stop = true;
                    } else {
                        perror("accept");
                        stop = true;
                    }
                    break;
                }
                if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK) < 0) {
                    perror("setting O_NONBLOCK");
                    close(sfd);
                    break;
                }
                reject = false;
                if (reject) {

                } else {
                    obj_conn_dispatch_conn_new(sfd, OBJ_CONN_NEW_CMD, EV_READ | EV_PERSIST);
                }
                stop = true;
                break;
            case OBJ_CONN_WAITING:
                if (!obj_conn_update_event(c, EV_READ | EV_PERSIST)) {
                    if (obj_settings.verbose > 0) {
                        fprintf(stderr, "can't update event\n");
                    }
                    /* close the connection */
                    obj_conn_set_state(c, OBJ_CONN_CLOSING);
                    break;
                }
                obj_conn_set_state(c, OBJ_CONN_READ);
                stop = true;
                break;
            case OBJ_CONN_NEW_CMD:
                --nreqs;
                if (nreqs >= 0) {
                    obj_conn_reset_cmd_handler(c);
                } else if (obj_conn_has_pending_reply(c)) {
                    /* can not process any new requests, try to send some data */
                    obj_conn_set_state(c, OBJ_CONN_WRITE);
                } else {
                    /* do nothing */
                    if (obj_buffer_readable_bytes(c->inbuf) > 0) {
                        if (!obj_conn_update_event(c, EV_WRITE | EV_PERSIST)) {
                            if (obj_settings.verbose > 0) {
                                fprintf(stderr, "can't update event\n");
                            }
                            obj_conn_set_state(c, OBJ_CONN_CLOSING);
                            break;
                        }
                    }
                    stop = true;
                }
                break;
            case OBJ_CONN_PARSE_CMD:
                /* try to read command */
                if (!obj_proto_read_command(c)) {
                    /* printf("we need more data\n"); */
                    /* need more data */
                    if (obj_conn_has_pending_reply(c)) {
                        obj_conn_set_state(c, OBJ_CONN_WRITE);
                    } else {
                        obj_conn_set_state(c, OBJ_CONN_WAITING);
                    }
                }
                break;
            case OBJ_CONN_READ:
                read_res = obj_conn_read(c);
                /* printf("read res = %d\n", read_res); */
                switch (read_res) {
                    case OBJ_CONN_READ_NO_DATA_RECEIVED:
                        obj_conn_set_state(c, OBJ_CONN_WAITING);
                        break;
                    case OBJ_CONN_READ_DATA_RECEIVED:
                        obj_conn_set_state(c, OBJ_CONN_PARSE_CMD);
                        break;
                    case OBJ_CONN_READ_ERROR:
                        obj_conn_set_state(c, OBJ_CONN_CLOSING);
                        break;
                    case OBJ_CONN_READ_MEMORY_ERROR:
                        /* already set close_after_write */
                        if (obj_conn_has_pending_reply(c)) {
                            /* try to send something, otherwise close the connection */
                            obj_conn_set_state(c, OBJ_CONN_WRITE);
                        } else {
                            obj_conn_set_state(c, OBJ_CONN_CLOSING);
                        }
                        break;
                }
                break;
            case OBJ_CONN_WRITE:
                write_res = obj_conn_write(c);
                /* printf("write res = %d\n", write_res); */
                switch (write_res) {
                    case OBJ_CONN_WRITE_COMPLETE:
                        obj_conn_set_state(c, OBJ_CONN_NEW_CMD);
                        if (c->close_after_write) {
                            obj_conn_set_state(c, OBJ_CONN_CLOSING);
                        }
                        break;
                    case OBJ_CONN_WRITE_INCOMPLETE:
                        break;
                    case OBJ_CONN_WRITE_SOFT_ERROR:
                        stop = true;
                        break;
                    case OBJ_CONN_WRITE_HARD_ERROR:
                        obj_conn_set_state(c, OBJ_CONN_CLOSING);
                        break;
                }
                break;
            case OBJ_CONN_CLOSING:
                obj_conn_close(c);
                stop = true;
                break;
            case OBJ_CONN_CLOSED:
                abort();
                break;
            default:
                /* unexpected */
                fprintf(stderr, "unexpected\n");
                obj_conn_close(c);
                stop = true;
                break;
        }
    }
    return;
}

static void obj_conn_maxconns_handler(const evutil_socket_t fd, const short which, void *arg) {
    struct timeval t = {.tv_sec = 0, .tv_usec = 10000};
    if (fd == -42 || !obj_conn_allow_new_conns) {
        /* reschedule in 10ms if we need to keep polling */
        evtimer_set(&obj_conn_maxconnsevent, obj_conn_maxconns_handler, 0);
        event_base_set(obj_main_base, &obj_conn_maxconnsevent);
        evtimer_add(&obj_conn_maxconnsevent, &t);
    } else {
        evtimer_del(&obj_conn_maxconnsevent);
        obj_conn_accept_new_conns(true);
    }
}

/* release resources hold by the connection */
static void obj_conn_cleanup(obj_conn_t *c) {
    obj_assert(c != NULL);
    obj_list_node_t *node;
    /* free input buffer */
    if (c->inbuf != NULL) {
        obj_buffer_destroy(c->inbuf);
    }
    /* free output buffer */
    if (c->outbuf != NULL) {
        obj_buffer_destroy(c->outbuf);
    }
    /* free reply list */
    if (c->reply_list != NULL) {
        while (!obj_list_is_empty(c->reply_list)) {
            node = obj_list_get_head(c->reply_list);
            if (node->value != NULL) {
                obj_buffer_destroy((obj_buffer_t *)node->value);
            }
            obj_list_del_node(c->reply_list, node);
        }
    }
    obj_list_destroy(c->reply_list);
}

/* free a connection */
static void obj_conn_free(obj_conn_t *c) {
    if (c) {
        obj_conn_conns[c->sfd] = NULL;
        obj_free(c);
    }
}

/* initialize the connection array */
void obj_conn_conns_init() {
    int next_fd = dup(1);
    if (next_fd < 0) {
        fprintf(stderr, "failed to duplicate file descriptor\n");
        exit(1);
    }
    /* extra room for unexpected open fds */
    int headroom = 10;              
    struct rlimit rl;
    obj_conn_max_fds = obj_settings.maxconns + headroom + next_fd;
    /* get the actual highest fd if possible */
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        obj_conn_max_fds = rl.rlim_max;
    } else {
        fprintf(stderr, "failed to query maximum file descriptor\n");
    }
    close(next_fd);
    if ((obj_conn_conns = obj_alloc(obj_conn_max_fds * sizeof(obj_conn_t *))) == NULL) {
        fprintf(stderr, "failed to allocate connection structures\n");
        exit(1);
    }
}

obj_conn_context_t *obj_conn_context_create() {
    obj_conn_context_t *context = obj_alloc(sizeof(obj_conn_context_t));
    if (context == NULL) {
        return NULL;
    }
    context->locker = obj_locker_create();
    if (context->locker == NULL) {
        obj_free(context);
        return NULL;
    }
    return context;
}

void obj_conn_context_destroy(obj_conn_context_t *context) {
    obj_locker_destroy(context->locker);
    obj_free(context);
}

obj_conn_t *obj_conn_new(const int sfd, obj_conn_state_t init_state, const short event_flags, struct event_base *base) {
    obj_conn_t *c = NULL;
    obj_buffer_t *inbuf = NULL;
    obj_buffer_t *outbuf = NULL;
    obj_list_t *reply_list = NULL;
    obj_conn_context_t *context = NULL;
    obj_assert(sfd >= 0 && sfd < obj_conn_max_fds);
    c = obj_conn_conns[sfd];
    if (c == NULL) {
        if (!(c = (obj_conn_t *)obj_alloc(sizeof(obj_conn_t)))) {
            fprintf(stderr, "failed to allocation memory for connection\n");
            return NULL;
        }
        c->sfd = sfd;
        obj_conn_conns[sfd] = c;
    }
    c->state = init_state;
    /* set for idle kicker */
    c->last_cmd_time = g_rel_current_time;
    c->close_after_write = false;
    inbuf = obj_buffer_create();
    if (inbuf == NULL) {
        fprintf(stderr, "failed to allocate memory for input buffer\n");
        goto clean;
    }
    c->inbuf = inbuf;
    outbuf = obj_buffer_create();
    if (outbuf == NULL) {
        fprintf(stderr, "failed to allocate memory for output buffer\n");
        goto clean;
    }
    c->outbuf = outbuf;
    reply_list = obj_list_create();
    if (reply_list == NULL) {
        fprintf(stderr, "failed to create reply list\n");
        goto clean;
    }
    c->reply_list = reply_list;
    context = obj_conn_context_create();
    if (context == NULL) {
        fprintf(stderr, "failed to create connection context\n");
        goto clean;
    }
    event_set(&c->event, sfd, event_flags, obj_conn_event_handler, (void *)c);
    event_base_set(base, &c->event);
    c->event_flags = event_flags;
    if (event_add(&c->event, 0) == -1) {
        goto clean;
    }
success:
    return c;
clean:
    if (c != NULL) {
        obj_free(c);
    }
    if (inbuf != NULL) {
        obj_buffer_destroy(inbuf);
    }
    if (outbuf != NULL) {
        obj_buffer_destroy(outbuf);
    }
    if (reply_list != NULL) {
        obj_list_destroy(reply_list);
    }
    if (context != NULL) {
        obj_conn_context_destroy(context);
    }
    return NULL;
}


void obj_conn_dispatch_conn_new(int sfd, obj_conn_state_t init_state, int event_flags) {
    obj_conn_queue_item_t *item = obj_conn_queue_new_item();
    char buf[1];
    if (item == NULL) {
        close(sfd);
        fprintf(stderr, "failed to allocate memory for connection object\n");
        return;
    }
    /* round robin */
    int tid = (obj_thread_last_thread + 1) % obj_settings.num_threads;
    obj_thread_libevent_thread_t *thread = obj_thread_threads + tid;
    obj_thread_last_thread = tid;
    /* init item */
    item->sfd = sfd;
    item->init_state = init_state;
    item->event_flags = event_flags;

    obj_conn_queue_push(thread->new_conn_queue, item);
    buf[0] = 'c';
    /* nofify worker thread */
    if (write(thread->notify_send_fd, buf, 1) != 1) {
        perror("writing to worker thread notify pipe error");
    }
}

void obj_conn_accept_new_conns(const obj_bool_t do_accept) {
    pthread_mutex_lock(&obj_conn_lock);
    obj_conn_do_accept_new_conns(do_accept);
    pthread_mutex_unlock(&obj_conn_lock);
}

void obj_conn_do_accept_new_conns(const obj_bool_t do_accept) {
    obj_conn_t *next;
    for (next = obj_conn_listen_conn; next; next = next->next) {
        if (do_accept) {
            obj_conn_update_event(next, EV_READ | EV_PERSIST);
            if (listen(next->sfd, obj_settings.backlog) != 0) {
                perror("listen");
            }
        } else {
            obj_conn_update_event(next, 0);
            if (listen(next->sfd, 0) != 0) {
                perror("listen");
            }
        }
    }
    if (do_accept) {

    } else {
        obj_conn_allow_new_conns = false;
        obj_conn_maxconns_handler(-42, 0, 0);
    }
}

obj_bool_t obj_conn_update_event(obj_conn_t *c, const int new_flags) {
    obj_assert(c != NULL);
    struct event_base *base = c->event.ev_base;
    if (c->event_flags == new_flags) {
        return true;
    }
    if (event_del(&c->event) == -1) {
        return false;
    }
    event_set(&c->event, c->sfd, new_flags, obj_conn_event_handler, (void *)c);
    event_base_set(base, &c->event);
    c->event_flags = new_flags;
    if (event_add(&c->event, 0) == -1) {
        return false;
    }
    return true;
}

void obj_conn_event_handler(const evutil_socket_t fd, const short which, void *arg) {
    obj_conn_t *c;
    c = (obj_conn_t *)arg;
    obj_assert(c != NULL);
    c->which = which;
    if (fd != c->sfd) {
        if (obj_settings.verbose > 0) {
            fprintf(stderr, "event fd doesn't match conn fd\n");
        }
        obj_conn_close(c);
        return;
    }
    obj_drive_machine(c);
    return;
}

/* close idle connection */
void obj_conn_close_idle(obj_conn_t *c) {
    if (obj_settings.idle_timeout > 0 && (g_rel_current_time - c->last_cmd_time) > obj_settings.idle_timeout) {
        /* a connection timeout */
        if (c->state != OBJ_CONN_NEW_CMD && c->state != OBJ_CONN_READ) {
            if (obj_settings.verbose > 1) {
                fprintf(stderr, "fd %d wants to timeout, but isn't in correct state\n", c->sfd);
            }
            return;
        }
        if (obj_settings.verbose > 1) {
            fprintf(stderr, "closing idle fd %d\n", c->sfd);
        }
        obj_conn_set_state(c, OBJ_CONN_CLOSING);
        /* drive the state machine */
        obj_drive_machine(c);
    }
    
}

void obj_conn_close(obj_conn_t *c) {
    obj_assert(c != NULL);
    /* delete the event, the socket and the conn */
    event_del(&c->event);
    if (obj_settings.verbose > 1) {
        fprintf(stderr, "<%d connection closed\n", c->sfd);
    }
    /* TODO force release of read buffer */
    obj_conn_cleanup(c);
    obj_conn_set_state(c, OBJ_CONN_CLOSED);
    close(c->sfd);
    pthread_mutex_lock(&obj_conn_lock);
    obj_conn_allow_new_conns = true;
    pthread_mutex_unlock(&obj_conn_lock);
}

void obj_conn_close_all() {
    int i;
    for (i = 0; i < obj_conn_max_fds; i++) {
        if (obj_conn_conns[i] && obj_conn_conns[i]->state != OBJ_CONN_CLOSED) {
            obj_conn_close(obj_conn_conns[i]);
        }
    }
}

void obj_conn_set_state(obj_conn_t *c, obj_conn_state_t state) {
    obj_assert(c != NULL);
    if (state != c->state) {
        c->state = state;
    }
}