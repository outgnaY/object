#include "obj_core.h"

void time_interval(struct timeval t1, struct timeval t2) {
    int us = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
    printf("total time: %ds %dus\n", us / 1000000, us - (us / 1000000) * 1000000);
}

int main() {
    struct timeval start;
    struct timeval end;
    int sockfd;
    struct sockaddr_in addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6666);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "connection failed\n");
    }
    obj_uint8_t recv_buf[1024];
    obj_uint8_t send_buf[1024];
    int n;
    obj_bson_t *create_database_cmd = OBJ_BSON_BCON_NEW(
        "create_database", "db1"
    );
    obj_bson_t *list_databases_cmd = OBJ_BSON_BCON_NEW(
        "list_databases", ""
    );
    obj_bson_t *delete_database_cmd = OBJ_BSON_BCON_NEW(
        "delete_database", "db1"
    );
    obj_bson_t *create_collection_cmd = OBJ_BSON_BCON_NEW(
        "create_collection", "db1.coll1",
        "prototype", "{", "a", OBJ_BSON_BCON_INT32(OBJ_BSON_TYPE_DOUBLE), "}"
    );
    obj_bson_t *list_collections_cmd = OBJ_BSON_BCON_NEW(
        "list_collections", "db1"
    );
    obj_bson_t *delete_collection_cmd = OBJ_BSON_BCON_NEW(
        "delete_collection", "db1.coll1"
    );
    obj_bson_t *create_index_cmd = OBJ_BSON_BCON_NEW(
        "create_index", "db1.coll1",
        "name", "index1",
        "pattern", "{", "a", OBJ_BSON_BCON_INT32(1), "}"
    );
    obj_bson_t *delete_index_cmd = OBJ_BSON_BCON_NEW(
        "delete_index", "db1.coll1",
        "name", "index1"
    );
    obj_bson_t *insert_cmd = OBJ_BSON_BCON_NEW(
        "insert", "db1.coll1"
    );
    
    


    /* create database db1 */
    obj_memcpy(send_buf, create_database_cmd->data, create_database_cmd->len);
    /* gettimeofday(&start, NULL); */
    write(sockfd, send_buf, create_database_cmd->len);
    n = read(sockfd, recv_buf, 1024);
    /* gettimeofday(&end, NULL); */
    /* time_interval(start, end); */
    obj_bson_t *reply1 = obj_bson_create_with_data(recv_buf, n);
    obj_bson_visit_print_visit(reply1);

    /* list databases */
    obj_memcpy(send_buf, list_databases_cmd->data, list_databases_cmd->len);
    write(sockfd, send_buf, list_databases_cmd->len);
    n = read(sockfd, recv_buf, 1024);
    obj_bson_t *reply2 = obj_bson_create_with_data(recv_buf, n);
    obj_bson_visit_print_visit(reply2);

    /* create collection */
    obj_memcpy(send_buf, create_collection_cmd->data, create_collection_cmd->len);
    write(sockfd, send_buf, create_collection_cmd->len);
    n = read(sockfd, recv_buf, 1024);
    obj_bson_t *reply3 = obj_bson_create_with_data(recv_buf, n);
    obj_bson_visit_print_visit(reply3);

    /* list collections */
    obj_memcpy(send_buf, list_collections_cmd->data, list_collections_cmd->len);
    write(sockfd, send_buf, list_collections_cmd->len);
    n = read(sockfd, recv_buf, 1024);
    obj_bson_t *reply4 = obj_bson_create_with_data(recv_buf, n);
    obj_bson_visit_print_visit(reply4);

    /* create index */
    obj_memcpy(send_buf, create_index_cmd->data, create_index_cmd->len);
    write(sockfd, send_buf, create_index_cmd->len);
    n = read(sockfd, recv_buf, 1024);
    obj_bson_t *reply5 = obj_bson_create_with_data(recv_buf, n);
    obj_bson_visit_print_visit(reply5);

    /* delete index */
    obj_memcpy(send_buf, delete_index_cmd->data, delete_index_cmd->len);
    write(sockfd, send_buf, delete_index_cmd->len);
    n = read(sockfd, recv_buf, 1024);
    obj_bson_t *reply6 = obj_bson_create_with_data(recv_buf, n);
    obj_bson_visit_print_visit(reply6);

    /* delete collection */
    obj_memcpy(send_buf, delete_collection_cmd->data, delete_collection_cmd->len);
    write(sockfd, send_buf, delete_collection_cmd->len);
    n = read(sockfd, recv_buf, 1024);
    obj_bson_t *reply7 = obj_bson_create_with_data(recv_buf, n);
    obj_bson_visit_print_visit(reply7);

    /* delete databases */
    obj_memcpy(send_buf, delete_database_cmd->data, delete_database_cmd->len);
    write(sockfd, send_buf, delete_database_cmd->len);
    n = read(sockfd, recv_buf, 1024);
    obj_bson_t *reply8 = obj_bson_create_with_data(recv_buf, n);
    obj_bson_visit_print_visit(reply8);

    
    return 0;
}

/* a simple cilent */
/*
int main() {
    obj_global_mem_context_init();
    int sockfd;
    struct sockaddr_in addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6666);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "connection failed\n");
    }
    obj_uint8_t recv_buf[1024];
    obj_uint8_t send_buf[1024];
    obj_int32_t len = sizeof(obj_msg_header_t);
    obj_int32_t op = obj_int32_to_le(OBJ_MSG_OP_DELETE);
    obj_int32_t flags = obj_int32_to_le(1);
    obj_bson_t *selector;
    const char *collection_name = "test_collection\0";
    selector = obj_bson_create();
    obj_bson_append_utf8(selector, "key1", 4, "value1", 6);
    len += obj_strlen(collection_name) + 1;
    len += sizeof(obj_int32_t);
    len += selector->len;
    len = obj_int32_to_le(len);
    int i;
    int nread = 0;
    int curr_read = 0;
    obj_int32_t expect = 0;
    int total_read = 0;
    int avail = 1024;
    obj_bool_t flag = false;
    int curr = 0;
    obj_memcpy(send_buf + curr, &len, sizeof(obj_int32_t));
    curr += sizeof(obj_int32_t);
    obj_memcpy(send_buf + curr, &op, sizeof(obj_int32_t));
    curr += sizeof(obj_int32_t);
    obj_memcpy(send_buf + curr, collection_name, obj_strlen(collection_name) + 1);
    curr += obj_strlen(collection_name) + 1;
    obj_memcpy(send_buf + curr, &flags, sizeof(obj_int32_t));
    curr += sizeof(obj_int32_t);
    obj_memcpy(send_buf + curr, selector->data, selector->len);
    curr += selector->len;
    struct timeval start;
    struct timeval end;
    printf("begin\n");
    gettimeofday(&start, NULL);
    for (i = 0; i < 50000; i++) {
        
        write(sockfd, send_buf, curr);
        nread = 0;
        curr_read = 0;
        expect = 0;
        flag = false;
        
        while (true) {
            curr_read = read(sockfd, recv_buf, avail);
            if (curr_read > 0) {
                nread += curr_read;
                if (!flag) {
                    if (nread < sizeof(obj_int32_t)) {
                        continue;
                    } else {
                        flag = true;
                        expect = (obj_int32_t)(*((obj_int32_t *)(recv_buf)));
                        expect = obj_int32_from_le(expect);
                        if (nread == expect) {
                            break;
                        } else {
                            continue;
                        }
                    }
                } else {
                    if (nread == expect) {
                        break;
                    } else {
                        continue;
                    }
                }
            } else if (curr_read < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                }
                fprintf(stderr, "read error\n");
                exit(1);
            } else { 
                fprintf(stderr, "connection closed\n");
                exit(1);
            }
        }
        total_read += nread;
        
    }
    gettimeofday(&end, NULL);
    printf("time: %ld %ld\n", end.tv_sec - start.tv_sec, end.tv_usec - start.tv_usec);
    close(sockfd);
    return 0;
}
*/