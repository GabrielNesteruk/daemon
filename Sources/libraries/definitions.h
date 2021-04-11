#pragma once

#include <stdbool.h>

struct ProgramData
{
    const char* source_path;
    const char* destination_path;
    int sleeping_time;
    bool recursive_scan;
    long f_size;
};

enum WakeyReason
{
    SLEEP = 0,
    SIGNAL
};

extern const int sleep_time_initializer;
extern const int file_size_limit;
extern volatile enum WakeyReason wakey_reason;