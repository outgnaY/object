#include "obj_core.h"

/* a simple cilent */
int main() {
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
    /* send message */
    /* 
    const char *str = "hello\n";
    write(sockfd, str, strlen(str));
    */
    close(sockfd);

    return 0;
}