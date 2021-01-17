#include "obj_core.h"

/* benchmark */

typedef struct bench_settings_s bench_settings_t;
typedef struct bench_client_s bench_client_t;
typedef struct bench_thread_s bench_thread_t;

/* benchmark settings */
struct bench_settings_s {
    const char *host_ip;                /* host ip */
    int host_port;                      /* host port */
    int num_clients;                    /* number of parallel connections */
    int num_requests;                   /* total number of requests */
    obj_bool_t keep_alive;              /* keep alive */ 
    int num_threads;                    /* number of threads */
    int pipeline;                       /* pipeline number of requests per-request */
};

/* benchmark client */
struct bench_client_s {

};

/* benchmark thread */
struct bench_thread_s {

};

static bench_settings_t bench_settings;

/* print usage */
static void bench_usage() {
    printf("usage: benchmark [-a <host>] [-p <port>] [-c <clients>] [-n <requests>] [-k <keepalive>] [-t <threads>] [-P <numreq>]\n\n");
    printf("-a, --hostname      <hostname>       server hostname (default 127.0.0.1)\n"
           "-p, --port          <port>           server port (default 6666)\n"
           "-c, --clients       <clients>        number of parallel connections (default 50)\n"
           "-n, --requests      <requests>       total number of requests (default 10000)\n"
           "-k, --keepalive     <keepalive>      1=keep alive 0=reconnect (default 1)\n"
           "-t, --threads       <threads>        number of threads (default 1)\n"
           "-P, --pipeline      <numreq>         pipeline <numreq> requests (default 1)\n" 
           "-h, --help                           print help message\n"      
           );
}   

/* init settings */
static void bench_settings_init() {
    bench_settings.host_ip = "127.0.0.1";
    bench_settings.host_port = 6666;
    bench_settings.num_clients = 50;
    bench_settings.num_requests = 10000;
    bench_settings.keep_alive = true;
    bench_settings.num_threads = 1;
    bench_settings.pipeline = 1;
}

int main(int argc, char **argv) {
    int c;
    obj_bool_t keepalive;
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    /* init default settings */
    bench_settings_init();
    /* process arguments */
    char *shortopts = 
        "a"         /* host */
        "p"         /* port */
        "c"         /* number of parallel connections */
        "n"         /* total number of requests */
        "k"         /* keep alive */
        "t"         /* number of threads */
        "P"         /* pipeline requests */
        "h"         /* help */
        ;
#ifdef HAVE_GETOPT_LONG
    const struct option longopts[] = {
        {"hostname", required_argument, 0, 'a'},
        {"port", required_argument, 0, 'p'},
        {"clients", required_argument, 0, 'c'},
        {"requests", required_argument, 0, 'n'},
        {"keepalive", required_argument, 0, 'k'},
        {"threads", required_argument, 0, 't'},
        {"pipeline", required_argument, 0, 'P'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int optindex;
    while ((c = getopt_long(argc, argv, shortopts, longopts, &optindex)) != -1) {
#else
    while ((c = getopt(argc, argv, shortopts)) != -1) {
#endif
    switch (c) {
        case 'a':
            bench_settings.host_ip = optarg;
            break;
        case 'p':
            bench_settings.host_port = atoi(optarg);
            break;
        case 'c':
            bench_settings.num_clients = atoi(optarg);
            if (bench_settings.num_clients <= 0) {
                fprintf(stderr, "number of clients must be greater than 0\n");
                exit(1);
            }
            break;
        case 'n':
            bench_settings.num_requests = atoi(optarg);
            if (bench_settings.num_requests <= 0) {
                fprintf(stderr, "total number of requests must be greater than 0\n");
                exit(1);
            }
            break;
        case 'k':
            keepalive = atoi(optarg) > 0 ? true : false;
            break;
        case 't':
            bench_settings.num_threads = atoi(optarg);
            if (bench_settings.num_threads <= 0) {
                fprintf(stderr, "number of threads must be greater than 0\n");
                exit(1);
            }
            break;
        case 'P':
            bench_settings.pipeline = atoi(optarg);
            if (bench_settings.pipeline <= 0) {
                fprintf(stderr, "number of pipeline requests must be greater than 0\n");
            }
            break;
        case 'h':
            bench_usage();
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }

    return 0;
}