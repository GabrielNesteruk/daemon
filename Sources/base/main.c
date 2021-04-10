#include "../libraries/daemon.h"
#include "../libraries/definitions.h"
#include "../libraries/parser.h"
#include <syslog.h>

int main(int argc, char *argv[])
{
    struct ProgramData data = parseCommandLine(argc, argv);
    daemonInit();

    while (1)
    {
        switch(wakey_reason)
        {
            case SLEEP:
                syslog(LOG_INFO, "Daemon has been woked up by sleeping timer.");
                break;
            case SIGNAL:
                syslog(LOG_INFO, "Daemon has been woked up by signal.");
                break;
        }
        daemonService(data);
        wakey_reason = SLEEP;
        syslog(LOG_INFO, "Daemon says zzz....");
        sleep(data.sleeping_time);   
    }
 
    daemonExit();

    return 0;
}