#include "obj_core.h"

/* globals */
obj_settings_t obj_settings;                                /* settings */
volatile obj_rel_time_t g_rel_current_time;                 /* seconds since process started */
time_t obj_process_started;                                 /* when the process was started */
struct event_base *obj_main_base;                           /* main thread event base */

static obj_stop_reason_t obj_stop_mainloop = OBJ_NOT_STOP;  /* mainloop stop reason */
static volatile sig_atomic_t obj_sighup;                    /* a HUP signal received but not yet handled */
static struct event obj_clockevent;

static void obj_settings_init();
static void obj_sig_handler(const int sig);
static void obj_sighup_handler();
static void obj_clock_handler(const evutil_socket_t fd, const short which, void *arg);
static void obj_version();
static void obj_usage();
static int obj_new_socket(struct addrinfo *ai);
static obj_bool_t obj_server_socket(int port);
static obj_bool_t obj_server_sockets(int port);

/* init settings */
static void obj_settings_init() {
    obj_settings.num_threads = 4;
    obj_settings.maxconns = 1024;
    /* disabled by default */
    obj_settings.idle_timeout = 0;
    obj_settings.backlog = 1024;
    obj_settings.maxconns_fast = true;
    obj_settings.max_reqs_per_event = 20;
    obj_settings.port = 6666;
    obj_settings.verbose = 2;
}

static void obj_sig_handler(const int sig) {
    obj_stop_mainloop = OBJ_EXIT_NORMALLY;
    fprintf(stderr, "signal %s\n", strsignal(sig));
}

static void obj_sighup_handler() {
    obj_sighup = 1;
    fprintf(stderr, "catch SIGHUP\n");
}

static void obj_clock_handler(const evutil_socket_t fd, const short which, void *arg) {
    struct timeval t = {.tv_sec = 1, .tv_usec = 0};
    static obj_bool_t initialized = false;
    if (initialized) {
        evtimer_del(&obj_clockevent);
    } else {
        initialized = true;
    }
    /* check HUP signal */
    if (obj_sighup) {
        obj_sighup = 0;
    }
    evtimer_set(&obj_clockevent, obj_clock_handler, 0);
    event_base_set(obj_main_base, &obj_clockevent);
    evtimer_add(&obj_clockevent, &t);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    g_rel_current_time = (obj_rel_time_t)(tv.tv_sec - obj_process_started);
}

static void obj_version() {
    printf("in-memory cache service. version %.1f\n", OBJ_VERSION);
}

static void obj_usage() {
    obj_version();
    printf("usage:\n");
    printf("-d, --daemon                run as a daemon\n"
           "-h, --help                  print help message and exit\n"
           "-r, --enable-coredumps      enable coredump\n");
    printf("-R, --max-reqs-per-event    maximum number of requests per event, default: %d\n", obj_settings.max_reqs_per_event);
    printf("-u, --user=<user>           assume identity of <username> (only when run as root)\n"
           "-v  --version               print version message and exit\n"
           );
}

static int obj_new_socket(struct addrinfo *ai) {
    int sfd;
    int flags;
    if ((sfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
        return -1;
    }
    if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 || fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("setting O_NONBLOCK");
        close(sfd);
        return -1;
    }
    return sfd;
}

/* listen on port */
static obj_bool_t obj_server_socket(int port) {
    int sfd;
    struct linger ling = {0, 0};
    struct addrinfo *ai;
    struct addrinfo *next;
    struct addrinfo hints = {.ai_flags = AI_PASSIVE, .ai_family = AF_UNSPEC};
    char port_buf[NI_MAXSERV];
    int error;
    int success = 0;
    int flags = 1;
    hints.ai_socktype = SOCK_STREAM;
    if (port == -1) {
        port = 0;
    }
    snprintf(port_buf, sizeof(port_buf), "%d", port);
    error = getaddrinfo(NULL, port_buf, &hints, &ai);
    if (error != 0) {
        if (error != EAI_SYSTEM) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        } else {
            perror("getaddrinfo");
        }
        return false;
    }
    for (next = ai; next; next = next->ai_next) {
        obj_conn_t *listen_conn_add;
        if ((sfd = obj_new_socket(next)) == -1) {
            if (errno == EMFILE) {
                perror("server socket");
                exit(EX_OSERR);
            }
            continue;
        }
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
        error = setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
        if (error != 0) {
            perror("setsockopt");
        }
        error = setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
        if (error != 0) {
            perror("setsockopt");
        }
        error = setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
        if (error != 0) {
            perror("setsockopt");
        }
        /* bind */
        if (bind(sfd, next->ai_addr, next->ai_addrlen) == -1) {
            if (errno != EADDRINUSE) {
                perror("bind");
                close(sfd);
                freeaddrinfo(ai);
                return false;
            }
            close(sfd);
            continue;
        } else {
            success++;
            if (listen(sfd, obj_settings.backlog) == -1) {
                perror("listen");
                close(sfd);
                freeaddrinfo(ai);
                return false;
            }
        }
        /* printf("main thread new connection, sfd = %d\n", sfd); */
        if (!(listen_conn_add = obj_conn_new(sfd, OBJ_CONN_LISTENING, EV_READ | EV_PERSIST, obj_main_base))) {
            fprintf(stderr, "failed to create listening connecton\n");
            exit(EXIT_FAILURE);
        }
        listen_conn_add->next = obj_conn_listen_conn;
        obj_conn_listen_conn = listen_conn_add;
        break;       // for debug
    }
    freeaddrinfo(ai);
    return success > 0;
}

static obj_bool_t obj_server_sockets(int port) {
    return obj_server_socket(port);
}


int main(int argc, char **argv) {
    obj_bool_t do_daemonize = false;
    obj_bool_t enable_core = false;
    char *username = NULL;
    struct passwd *pw;
    struct rlimit rlim;
    int c;
    int retval = EXIT_SUCCESS;
    /* handle signals */
    signal(SIGINT, obj_sig_handler);
    signal(SIGTERM, obj_sig_handler);
    signal(SIGHUP, obj_sighup_handler);
    /* init settings */
    obj_settings_init();
    /* process arguments */
    char *shortopts = 
        "d"     /* daemon mode */
        "h"     /* help */
        "r"     /* enable core dump */
        "R:"    /* max requests per event */
        "u:"    /* user identity to run as */
        "v"     /* version */
        ;
#ifdef HAVE_GETOPT_LONG
    const struct option longopts[] = {
        {"daemon", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"enable-coredumps", no_argument, 0, 'r'},
        {"max-reqs_per_event", required_argument, 0, 'R'},
        {"user", required_argument, 0, 'u'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    int optindex;
    while ((c = getopt_long(argc, argv, shortopts, longopts, &optindex)) != -1) {
#else
    while ((c = getopt(argc, argv, shortopts)) != -1) {
#endif
        switch (c) {
            case 'd':
                do_daemonize = true;
                break;
            case 'h':
                obj_usage();
                exit(EXIT_SUCCESS);
            case 'r':
                enable_core = true;
                break;
            case 'R':
                obj_settings.max_reqs_per_event = atoi(optarg);
                if (obj_settings.max_reqs_per_event == 0) {
                    fprintf(stderr, "max number of requests per event must be greater than 0\n");
                    exit(1);
                }
                break;
            case 'u':
                username = optarg;
                break;
            case 'v':
                obj_version();
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "invalid argument\n");
                obj_usage();
                exit(1);
        }
    }
    if (enable_core) {
        struct rlimit rlim_new;
        /* first try raising to infinity; if fails, try bringing the soft limit to the hard */
        if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
            rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
            if (setrlimit(RLIMIT_CORE, &rlim_new) != 0) {
                /* failed. try raising just to the old max */
                rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
                setrlimit(RLIMIT_CORE, &rlim_new);
            }
        }
        /* getrlimit again */
        if ((getrlimit(RLIMIT_CORE, &rlim) != 0) || rlim.rlim_cur == 0) {
            fprintf(stderr, "failed to ensure corefile creation\n");
            exit(EX_OSERR);
        }
    }
    /* lose root privileges if we have */
    if (getuid() == 0 || geteuid() == 0) {
        if (username == 0 || *username == '\0') {
            fprintf(stderr, "can't run as root without specify username\n");
            exit(EX_USAGE);
        }
        if ((pw = getpwnam(username)) == 0) {
            fprintf(stderr, "can't find the user %s to switch to\n", username);
            exit(EX_NOUSER);
        }
        if (setgroups(0, NULL) < 0) {
            int should_exit = errno != EPERM;
            fprintf(stderr, "failed to drop supplementary groups: %s\n", strerror(errno));
            if (should_exit) {
                exit(EX_OSERR);
            }
        }
        if (setgid(pw->pw_gid) < 0 || setuid(pw->pw_uid) < 0) {
            fprintf(stderr, "failed to assume identity of user %s\n", username);
            exit(EX_OSERR);
        }
    }
    if (do_daemonize) {
        /* ignore SIGHUP if run as a daemon */
        if (signal(SIGHUP, SIG_IGN)) {
            perror("failed to ignore SIGHUP");
        }
        if (!obj_daemonize(enable_core)) {
            fprintf(stderr, "failed to daemonize\n");
            exit(EXIT_FAILURE);
        }
    }
#if defined(LIBEVENT_VERSION_NUMBER) && LIBEVENT_VERSION_NUMBER >= 0x2000101
    struct event_config *ev_config;
    ev_config = event_config_new();
    event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);
    obj_main_base = event_base_new_with_config(ev_config);
    event_config_free(ev_config);
#else
    obj_main_base = event_init();
#endif
    /* init memory allocator */
    obj_global_mem_context_init();
    /* init connections structure */
    obj_conn_conns_init();
    /* set process start time */
    obj_process_started = time(0);
    /* ignore SIGPIPE signals */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        fprintf(stderr, "failed to ignore SIGPIPE\n");
        exit(EX_OSERR);
    }
    /* init libevent threads */
    obj_thread_init(obj_settings.num_threads, NULL);
    /* init timeout checking thread */
    if (obj_settings.idle_timeout > 0 && !obj_thread_start_conn_timeout_thread()) {
        exit(EXIT_FAILURE);
    }
    /* init clock handler */
    obj_clock_handler(0, 0, 0);
    /* listen */
    if (!obj_server_sockets(obj_settings.port)) {
        fprintf(stderr, "failed to listen on TCP port %d\n", obj_settings.port);
        exit(EX_OSERR);
    }
    /* event loop */
    while (!obj_stop_mainloop) {
        if (event_base_loop(obj_main_base, EVLOOP_ONCE) != 0) {
            retval = EXIT_FAILURE;
            break;
        }
    }
    switch (obj_stop_mainloop) {
        case OBJ_EXIT_NORMALLY:
            /* normal exit */
            break;
        default:
            fprintf(stderr, "exiting on error\n");
            break;
    }
    /* stop worker threads */
    obj_thread_stop_threads();
    /* clean up base */
    event_base_free(obj_main_base);
    return retval;
}