#include "obj_core.h"


/* try to read command */
obj_bool_t obj_proto_read_command(obj_conn_t *c) {
    /* printf("read command\n"); */
    obj_int32_t len;
    obj_bson_t *command_bson = NULL;
    obj_bool_t parse_res;
    if (obj_buffer_can_read_int32(c->inbuf)) {
        /* length */
        /* len = obj_buffer_v_read_int32_unsafe(c->inbuf); */
        len = obj_buffer_v_peek_int32_unsafe(c->inbuf);
        /* check length */
        if (len > OBJ_MSG_MAX_LEN) {
            /* close the connection */
            obj_conn_set_state(c, OBJ_CONN_CLOSING);
            /* force close */
            return true;
        }
        /* haven't read enough data yet */
        if (obj_buffer_readable_bytes(c->inbuf) < len) {
            return false;
        }
        /* read bson */ 
        command_bson = obj_buffer_read_bson_unsafe(c->inbuf, len);
        if (command_bson == NULL) {
            /* close the connection */
            obj_conn_set_state(c, OBJ_CONN_CLOSING);
            /* force close */
            return true;
        }
        
        /* process command */
        obj_process_command(c, command_bson);
        /* set last command time of the connection */
        c->last_cmd_time = g_rel_current_time;
        return true;
    } else {
        return false;
    }
}