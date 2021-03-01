#include "obj_core.h"

static obj_bool_t obj_msg_parse_update(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header);
static obj_bool_t obj_msg_parse_insert(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header);
static obj_bool_t obj_msg_parse_query(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header);
static obj_bool_t obj_msg_parse_delete(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header);
static obj_msg_operation_t obj_proto_get_operation(obj_conn_t *c);
static obj_bool_t obj_process_command(obj_conn_t *c, obj_msg_header_t *header);


/* command table */
static obj_bool_t (*obj_msg_parser[6])(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header) = {
    NULL,
    obj_msg_parse_update,
    obj_msg_parse_insert,
    obj_msg_parse_query,
    obj_msg_parse_delete,
    NULL
};

/* parse update message */
static obj_bool_t obj_msg_parse_update(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header) {
    /* printf("parse update\n"); */
    obj_msg_update_t *update_msg = NULL;
    /* safe check */
    int curr_len = sizeof(obj_msg_header_t);
    char *collection_name = NULL;
    int name_len;
    obj_int32_t flags;
    obj_int32_t selector_len;
    obj_int32_t update_len;
    obj_bson_t *selector = NULL;
    obj_bson_t *update = NULL;
    if (curr_len > len) {
        return false;
    }
    obj_buffer_v_retrieve(c->inbuf, sizeof(obj_msg_header_t));
    update_msg = obj_alloc(sizeof(obj_msg_update_t));
    if (update_msg == NULL) {
        return false;
    }
    /* set header */
    update_msg->header.len = len;
    update_msg->header.opCode = OBJ_MSG_OP_UPDATE;
    /* collection name */
    collection_name = obj_buffer_v_read_string_unsafe(c->inbuf, &name_len);
    if (collection_name == NULL) {
        goto clean;
    }
    curr_len += name_len + 1;
    if (curr_len > len) {
        goto clean;
    }
    update_msg->collection_name = collection_name;
    update_msg->name_len = name_len;
    /* flags */
    curr_len += sizeof(obj_int32_t);
    if (curr_len > len) {
        goto clean;
    }
    flags = obj_buffer_v_read_int32_unsafe(c->inbuf);
    update_msg->flags = flags;
    /* selector */
    if (curr_len + sizeof(obj_int32_t) > len) {
        goto clean;
    }
    selector_len = obj_buffer_v_peek_int32_unsafe(c->inbuf);
    curr_len += selector_len;
    if (curr_len > len) {
        goto clean;
    }
    selector = obj_buffer_v_read_bson_unsafe(c->inbuf, selector_len);
    if (selector == NULL) {
        goto clean;
    }
    update_msg->selector = selector;
    /* update */
    if (curr_len + sizeof(obj_int32_t) > len) {
        goto clean;
    }
    update_len = obj_buffer_v_peek_int32_unsafe(c->inbuf);
    curr_len += update_len;
    if (curr_len > len) {
        goto clean;
    }
    update = obj_buffer_v_read_bson_unsafe(c->inbuf, update_len);
    if (update == NULL) {
        goto clean;
    }
    update_msg->update = update;
    /* check length */
    if (curr_len != len) {
        goto clean;
    }
    /* all check passed */
success:
    *header = (obj_msg_header_t *)update_msg;
    return true;
    /* something wrong */
clean:
    if (update_msg != NULL) {
        obj_free(update_msg);
    }
    if (selector != NULL) {
        obj_bson_destroy(selector);
    }
    if (update != NULL) {
        obj_bson_destroy(update);
    }
    return false;
}

/* parse insert message */
static obj_bool_t obj_msg_parse_insert(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header) {
    /* printf("parse insert\n"); */
    obj_msg_insert_t *insert_msg = NULL;
    /* safe check */
    int curr_len = sizeof(obj_msg_header_t);
    char *collection_name = NULL;
    int name_len;
    obj_int32_t flags;
    obj_int32_t num;
    obj_int32_t object_len;
    obj_bson_t *object;
    obj_bson_t **objects;
    int i;

    if (curr_len > len) {
        return false;
    }
    obj_buffer_v_retrieve(c->inbuf, sizeof(obj_msg_header_t));
    insert_msg = obj_alloc(sizeof(obj_msg_insert_t));
    if (insert_msg == NULL) {
        return false;
    }
    /* set header */
    insert_msg->header.len = len;
    insert_msg->header.opCode = OBJ_MSG_OP_INSERT;
    /* collection name */
    collection_name = obj_buffer_v_read_string_unsafe(c->inbuf, &name_len);
    if (collection_name == NULL) {
        goto clean;
    }
    curr_len += name_len + 1;
    if (curr_len > len) {
        goto clean;
    }
    insert_msg->collection_name = collection_name;
    insert_msg->name_len = name_len;
    /* flags */
    curr_len += sizeof(obj_int32_t);
    if (curr_len > len) {
        goto clean;
    }
    flags = obj_buffer_v_read_int32_unsafe(c->inbuf);
    insert_msg->flags = flags;
    /* num */
    curr_len += sizeof(obj_int32_t);
    if (curr_len > len) {
        goto clean;
    }
    num = obj_buffer_v_read_int32_unsafe(c->inbuf);
    insert_msg->num = num;
    objects = obj_alloc(num * sizeof(obj_bson_t *));
    if (objects == NULL) {
        goto clean;
    }
    for (i = 0; i < num; i++) {
        objects[i] = NULL;
    }
    /* objects */
    for (i = 0; i < num; i++) {
        if (curr_len + sizeof(obj_int32_t) > len) {
            goto clean;
        }
        object_len = obj_buffer_v_peek_int32_unsafe(c->inbuf);
        curr_len += object_len;
        if (curr_len > len) {
            goto clean;
        }
        object = obj_buffer_v_read_bson_unsafe(c->inbuf, object_len);
        if (object == NULL) {
            goto clean;
        }
        objects[i] = object;
    }
    insert_msg->objects = objects;
    /* check length */
    if (curr_len != len) {
        goto clean;
    }
    /* all check passed */
success:
    *header = (obj_msg_header_t *)insert_msg;
    return true;
clean:
    if (insert_msg != NULL) {
        obj_free(insert_msg);
    }
    if (objects != NULL) {
        for (i = 0; i < num; i++) {
            if (objects[i] != NULL) {
                obj_bson_destroy(objects[i]);
            } else {
                break;
            }
        }
        obj_free(objects);
    }
    return false;
} 

/* parse query message */
static obj_bool_t obj_msg_parse_query(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header) {
    /* printf("parse query\n"); */
    obj_msg_query_t *query_msg = NULL;
    /* safe check */
    int curr_len = sizeof(obj_msg_header_t);
    char *collection_name = NULL;
    int name_len;
    obj_int32_t flags;
    obj_int32_t num_return;
    obj_int32_t query_len;
    obj_int32_t return_fields_len;
    obj_bson_t *query;
    obj_bson_t *return_fields;
    if (curr_len > len) {
        return false;
    }
    obj_buffer_v_retrieve(c->inbuf, sizeof(obj_msg_header_t));
    query_msg = obj_alloc(sizeof(obj_msg_query_t));
    if (query_msg == NULL) {
        return false;
    }
    /* set header */
    query_msg->header.len = len;
    query_msg->header.opCode = OBJ_MSG_OP_QUERY;
    /* collection name */
    collection_name = obj_buffer_v_read_string_unsafe(c->inbuf, &name_len);
    if (collection_name == NULL) {
        goto clean;
    }
    curr_len += name_len + 1;
    if (curr_len > len) {
        goto clean;
    }
    query_msg->collection_name = collection_name;
    query_msg->name_len = name_len;
    /* flags */
    curr_len += sizeof(obj_int32_t);
    if (curr_len > len) {
        goto clean;
    }
    flags = obj_buffer_v_read_int32_unsafe(c->inbuf);
    query_msg->flags = flags;
    /* num_return */
    curr_len += sizeof(obj_int32_t);
    if (curr_len > len) {
        goto clean;
    }
    num_return = obj_buffer_v_read_int32_unsafe(c->inbuf);
    query_msg->num_return = num_return;
    /* query */
    if (curr_len + sizeof(obj_int32_t) > len) {
        goto clean;
    }
    query_len = obj_buffer_v_peek_int32_unsafe(c->inbuf);
    curr_len += query_len;
    if (curr_len > len) {
        goto clean;
    }
    query = obj_buffer_v_read_bson_unsafe(c->inbuf, query_len);
    if (query == NULL) {
        goto clean;
    }
    query_msg->query = query;
    /* return fields */
    if (curr_len + sizeof(obj_int32_t) > len) {
        goto clean;
    }
    return_fields_len = obj_buffer_v_peek_int32_unsafe(c->inbuf);
    curr_len += return_fields_len;
    if (curr_len > len) {
        goto clean;
    }
    return_fields = obj_buffer_v_read_bson_unsafe(c->inbuf, return_fields_len);
    if (return_fields == NULL) {
        goto clean;
    }
    query_msg->num_return = num_return;
    /* check length */
    if (curr_len != len) {
        goto clean;
    }
    /* all check passed */
success:
    *header = (obj_msg_header_t *)query_msg;
    return true;
clean:
    if (query_msg != NULL) {
        obj_free(query_msg);
    }
    if (query != NULL) {
        obj_bson_destroy(query);
    }
    if (return_fields != NULL) {
        obj_bson_destroy(return_fields);
    }
    return false;
}

/* parse delete message */
static obj_bool_t obj_msg_parse_delete(obj_conn_t *c, obj_int32_t len, obj_msg_header_t **header) {
    /* printf("parse delete\n"); */
    /* printf("read_index %d, write_index %d, v_read_index %d\n", c->inbuf->read_index, c->inbuf->write_index, c->inbuf->v_read_index); */
    obj_msg_delete_t *delete_msg = NULL;
    /* safe check */
    int curr_len = sizeof(obj_msg_header_t);
    char *collection_name = NULL;
    int name_len;
    obj_int32_t flags;
    obj_int32_t selector_len;
    obj_bson_t *selector = NULL;

    if (curr_len > len) {
        return false;
    }
    obj_buffer_v_retrieve(c->inbuf, sizeof(obj_msg_header_t));
    delete_msg = obj_alloc(sizeof(obj_msg_delete_t));
    if (delete_msg == NULL) {
        return false;
    }
    /* set header */
    delete_msg->header.len = len;
    delete_msg->header.opCode = OBJ_MSG_OP_DELETE;
    /* collection name */
    collection_name = obj_buffer_v_read_string_unsafe(c->inbuf, &name_len);
    if (collection_name == NULL) {
        goto clean;
    }
    curr_len += name_len + 1;
    if (curr_len > len) {
        goto clean;
    }
    delete_msg->collection_name = collection_name;
    delete_msg->name_len = name_len;
    /* flags */
    curr_len += sizeof(obj_int32_t);
    if (curr_len > len) {
        goto clean;
    }
    flags = obj_buffer_v_read_int32_unsafe(c->inbuf);
    delete_msg->flags = flags;
    /* selector */
    if (curr_len + sizeof(obj_int32_t) > len) {
        goto clean;
    }
    selector_len = obj_buffer_v_peek_int32_unsafe(c->inbuf);
    curr_len += selector_len;
    if (curr_len > len) {
        goto clean;
    }
    selector = obj_buffer_v_read_bson_unsafe(c->inbuf, selector_len);
    if (selector == NULL) {
        goto clean;
    }
    delete_msg->selector = selector;
    /* check length */
    if (curr_len != len) {
        goto clean;
    }
    /* all check passed */
success:
    *header = (obj_msg_header_t *)delete_msg;
    return true;
    /* something wrong */
clean:
    if (delete_msg != NULL) {
        obj_free(delete_msg);
    }
    if (selector != NULL) {
        obj_bson_destroy(selector);
    }
    return false;
}

/* get operation type of current command */
static obj_msg_operation_t obj_proto_get_operation(obj_conn_t *c) {
    obj_int32_t op;
    op = obj_buffer_v_read_int32_unsafe(c->inbuf);
    return op;
}


/* for test */
static obj_bool_t obj_process_command(obj_conn_t *c, obj_msg_header_t *header) {
    /* printf("process command\n"); */
    obj_msg_reply_t *reply_msg;
    obj_bool_t res = false;
    obj_int32_t len = sizeof(obj_msg_header_t) + sizeof(obj_int32_t) + sizeof(obj_int64_t) + sizeof(obj_int32_t) + sizeof(obj_int32_t);
    obj_bson_t **objects;
    obj_bson_t *object;
    reply_msg = obj_alloc(sizeof(obj_msg_reply_t));
    if (reply_msg == NULL) {
        goto clean;
    }
    objects = obj_alloc(sizeof(obj_bson_t *));
    if (objects == NULL) {
        goto clean;
    }
    /* send message back */
    switch (header->opCode) {
        case OBJ_MSG_OP_UPDATE:
            object = obj_bson_init();
            if (object == NULL) {
                goto clean;
            }
            res = obj_bson_append_utf8(object, "type", 4, "update", 6);
            break;
        case OBJ_MSG_OP_INSERT:
            object = obj_bson_init();
            if (object == NULL) {
                goto clean;
            }
            res = obj_bson_append_utf8(object, "type", 4, "insert", 6);
            break;
        case OBJ_MSG_OP_QUERY:
            object = obj_bson_init();
            if (object == NULL) {
                goto clean;
            }
            res = obj_bson_append_utf8(object, "type", 4, "query", 5);
            break;
        case OBJ_MSG_OP_DELETE:
            object = obj_bson_init();
            if (object == NULL) {
                goto clean;
            }
            res = obj_bson_append_utf8(object, "type", 4, "delete", 6);
            break;
        default:
            obj_assert(false);
            break;
    }
    if (!res) {
        goto clean;
    }
    objects[0] = object;
    len += object->len;
    /* printf("reply msg len: %d\n", len); */
    len = obj_int32_to_le(len);
    reply_msg->header.len = len;
    reply_msg->header.opCode = header->opCode;
    reply_msg->response_flags = 0;
    reply_msg->cursor_id = 0;
    reply_msg->start_from = 0;
    reply_msg->num_return = 1;
    reply_msg->objects = objects;
    /* add reply */
    if (!obj_conn_add_reply(c, reply_msg)) {
        res = false;
        goto clean;
    }
    /* obj_buffer_dump(c->outbuf); */
    obj_conn_set_state(c, OBJ_CONN_WRITE);
clean:
    /* release resources */
    if (reply_msg != NULL) {
        obj_free(reply_msg);
    }
    if (objects != NULL) {
        if (objects[0] != NULL) {
            obj_bson_destroy(objects[0]);
        }
        obj_free(objects);
    }
    return res;
}



/* try to read command */
obj_bool_t obj_proto_read_command(obj_conn_t *c) {
    /* printf("read command\n"); */
    obj_int32_t len;
    /* obj_msg_operation_t op; */
    obj_msg_header_t peek_header;
    obj_msg_header_t *header;
    obj_bool_t parse_res;
    if (obj_buffer_can_read_int32(c->inbuf)) {
        /* length */
        /* len = obj_buffer_v_read_int32_unsafe(c->inbuf); */
        len = obj_buffer_v_peek_int32_unsafe(c->inbuf);
        /* check length */
        if (len > OBJ_MSG_MAX_LEN || len < sizeof(obj_msg_header_t)) {
            /* close the connection */
            obj_conn_set_state(c, OBJ_CONN_CLOSING);
            /* force close */
            return true;
        }
        /* haven't read enough data yet */
        if (obj_buffer_readable_bytes(c->inbuf) < len) {
            return false;
        }
        /* have read enough data */
        peek_header = obj_buffer_v_peek_msg_header_unsafe(c->inbuf);
        /* sanity check */
        if (peek_header.opCode <= OBJ_MSG_OP_REPLY || peek_header.opCode >= OBJ_MSG_OP_MAX) {
            obj_conn_set_state(c, OBJ_CONN_CLOSING);
            return true;
        }
        /* printf("message len: %d, op: %d\n", len, peek_header.opCode); */
        parse_res = obj_msg_parser[peek_header.opCode](c, len, &header);
        /* move read index */
        if (!parse_res) {
            obj_conn_set_state(c, OBJ_CONN_CLOSING);
            return true;
        }
        obj_buffer_retrieve(c->inbuf, len);
        obj_buffer_v_reset(c->inbuf);
        /* printf("read_index: %d, v_read_index: %d\n", c->inbuf->read_index, c->inbuf->v_read_index); */
        /* test process command */
        obj_process_command(c, header);
        /* set last command time of the connection */
        c->last_cmd_time = g_rel_current_time;
        return true;
    } else {
        return false;
    }
}