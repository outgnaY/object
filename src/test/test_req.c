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
    inet_pton(AF_INET, "192.168.137.130", &addr.sin_addr);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "connection failed\n");
    }
    obj_uint8_t recv_buf[1024];
    /* send message */
    /* delete message */
    obj_int32_t len = sizeof(obj_msg_header_t);
    obj_int32_t op = obj_int32_to_le(OBJ_MSG_OP_DELETE);
    obj_int32_t flags = obj_int32_to_le(1);
    obj_bson_t *selector;
    const char *collection_name = "test_collection\0";
    selector = obj_bson_init();
    obj_bson_append_utf8(selector, "key1", 4, "value1", 6);
    obj_bson_append_utf8(selector, "key2", 4, "value2", 6);
    len += obj_strlen(collection_name) + 1;
    len += sizeof(obj_int32_t);
    len += selector->len;
    len = obj_int32_to_le(len);
    /* header */
    write(sockfd, &len, sizeof(obj_int32_t));
    write(sockfd, &op, sizeof(obj_int32_t));
    /* collection name */
    write(sockfd, collection_name, obj_strlen(collection_name) + 1);
    /* flags */
    write(sockfd, &flags, sizeof(obj_int32_t));
    /* selector */
    write(sockfd, selector->data, selector->len);
    /* receive data */
    int nread = 0;
    int total_read = 0;
    int avail = 1024;
    int i;
    while (true) {
        printf("read\n");
        nread = read(sockfd, recv_buf + total_read, avail);
        if (nread < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            fprintf(stderr, "read error\n");
            exit(1);
        }
        if (nread == 0) {
            fprintf(stderr, "connection closed\n");
            exit(1);
        }
        total_read += nread;
        avail -= nread;
        if (avail <= 0) {
            break;
        }
    }
    for (i = 0; i < total_read; i++) {
        printf("%02x", recv_buf[i]);
    }
    printf("\n");
    close(sockfd);
    return 0;
}