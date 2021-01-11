#ifndef OBJ_H
#define OBJ_H

#include "obj_core.h"

typedef enum obj_stop_reason obj_stop_reason_t;
typedef struct obj_settings_s obj_settings_t;

enum obj_stop_reason {
    OBJ_NOT_STOP,
    OBJ_EXIT_NORMALLY
};

struct obj_settings_s {
    int num_threads;                /* number of worker threads */
    int maxconns;                   /* max connections opened simutaneously */
    int idle_timeout;               /* number of seconds to let connections idle */
    int backlog;
    obj_bool_t maxconns_fast;       /* whether or not to close connections early */
    int port;
    int verbose;
};

extern obj_settings_t obj_settings;                         /* settings */
extern volatile obj_rel_time_t obj_rel_current_time;        /* seconds since process started */
extern time_t obj_process_started;                          /* when the process was started */  
extern struct event_base *obj_main_base;                    /* main thread event base */

extern int obj_daemonize(obj_bool_t nochdir);               /* run as a daemon */

#endif  /* OBJ_H */