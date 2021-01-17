#include "obj_core.h"

/* a simple cilent */
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
    /* send message */
    /* delete message */
    obj_int32_t len = sizeof(obj_msg_header_t);
    obj_int32_t op = obj_int32_to_le(OBJ_MSG_OP_DELETE);
    obj_int32_t flags = obj_int32_to_le(1);
    obj_bson_t *selector;
    const char *collection_name = "test_collection\0";
    selector = obj_bson_init();
    obj_bson_append_utf8(selector, "key1", 4, "value1", 6);
    /* obj_bson_append_utf8(selector, "key2", 4, "value2", 6); */
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
        /* printf("%d\n", i); */
        /* send */
        write(sockfd, send_buf, curr);
        nread = 0;
        curr_read = 0;
        expect = 0;
        flag = false;
        /* receive */
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