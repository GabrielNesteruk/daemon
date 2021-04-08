#include "../libraries/daemon.h"
#include "../libraries/definitions.h"
#include "../libraries/parser.h"

int main(int argc, char *argv[])
{
    // daemonInit();

    struct ProgramData data = parseCommandLine(argc, argv);
    daemonService(data);
    //printf("%s\n%s\n%d\n", data.source_path, data.destination_path, data.sleeping_time);

    // while (1)
    // {
          
    // }
 
    //daemonExit();

    return 0;
}