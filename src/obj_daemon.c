#include "obj_core.h"

// daemonize the server process 
obj_bool_t obj_daemonize(obj_bool_t nochdir) {
    pid_t pid;
    int fd;
    // create new process 
    pid = fork();
    if (pid == -1) {
        return false;
    } else if (pid != 0) {
        // use _exit instead of exit to avoid flush buffers 
        _exit(EXIT_SUCCESS);
    }
    // create new session and process group 
    if (setsid() == -1) {
        return false;
    }
    // set the working directory to the root directory 
    if (!nochdir) {
        if (chdir("/") == -1) {
            perror("chdir");
            return false;
        }
    }

    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 stdin");
            return false;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 stdout");
            return false;
        }
        if (dup2(fd, STDERR_FILENO) < 0) {
            perror("dup2 stderr");
            return false;
        }
        if (fd > STDERR_FILENO) {
            if (close(fd) < 0) {
                perror("close");
                return false;
            }
        }
    }
    return true;

}