#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include "daemon.h"

void daemonInit()
{
        pid_t pid, sid;
        
        pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        }

        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        umask(0);
                
        openlog("DIR_DAEMON", LOG_PID, LOG_DAEMON);        
                
        sid = setsid();
        if (sid < 0) {
            exit(EXIT_FAILURE);
        }
        
        if ((chdir("/")) < 0) {
            exit(EXIT_FAILURE);
        }
        
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
}


void daemonExit()
{
    closelog();
}