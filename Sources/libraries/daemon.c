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
#include "directory.h"
#include <signal.h>

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

        if (signal(SIGUSR1, wakeUp) == SIG_ERR)
		{
			syslog(LOG_ERR, "Error has occured while trying to init signal.");
		
			exit(EXIT_FAILURE);
		}
}


void daemonExit()
{
    closelog();
}

void daemonService(struct ProgramData data)
{
    checkForDeletion(data);
    checkForModificationTime(data);
}

void wakeUp()
{
    wakey_reason = SIGNAL;
}