#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include "string.h"

struct ProgramData parseCommandLine(int argc, char *argv[])
{
    int opt;

    struct stat st;
    struct ProgramData data;
    data.sleeping_time = sleep_time_initializer;
    data.recursive_scan = false;

    if (argc < 4)
    {
        fprintf(stderr, "Too few arguments!\n");
        fprintf(stderr, "Usage: %s [-s source path] [-d destination path] [optional: -t seconds (sleep time)] [optional: -R (recursive scan)]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    while ((opt = getopt(argc, argv, "s:d:t:R")) != -1)
    {
        switch (opt)
        {
        case 's':
            if (stat(optarg, &st) == 0)
            {
                if (!(st.st_mode & S_IFDIR))
                {
                    fprintf(stderr, "Source path should be a directory!\n");
                    syslog(LOG_ERR, "Invalid source path.");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    if (*(optarg + strlen(optarg) - 1) == '/')
                        optarg[strlen(optarg) - 1] = 0;
                    data.source_path = optarg;
                }
            }
            else
            {
                fprintf(stderr, "Source path should be a directory!\n");
                syslog(LOG_ERR, "Invalid source path.");
                exit(EXIT_FAILURE);
            }
            break;
        case 'd':
            if (stat(optarg, &st) == 0)
            {
                if (!(st.st_mode & S_IFDIR))
                {
                    fprintf(stderr, "Destination path should be a directory!\n");
                    syslog(LOG_ERR, "Invalid destination path.");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    if (*(optarg + strlen(optarg) - 1) == '/')
                        optarg[strlen(optarg) - 1] = 0;
                    data.destination_path = optarg;
                }
            }
            else
            {
                fprintf(stderr, "Source path should be a directory!\n");
                syslog(LOG_ERR, "Invalid source path.");
                exit(EXIT_FAILURE);
            }
            break;
        case 't':
        {
            int tmp_seconds = atoi(optarg) * 60;
            if (tmp_seconds != 0)
                data.sleeping_time = tmp_seconds;
            break;
        }
        case 'R':
            data.recursive_scan = true;
            break;
        case '?':
            fprintf(stderr, "Usage: %s [-s source path] [-d destination path] [optional: -t seconds (sleep time)] [optional: -R (recursive scan)]\n", argv[0]);
                    exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Usage: %s [-s source path] [-d destination path] [optional: -t seconds (sleep time)] [optional: -R (recursive scan)]\n", argv[0]);
                    exit(EXIT_FAILURE);
        }
    }

    return data;
}