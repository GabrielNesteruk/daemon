#pragma once

struct ProgramData
{
    const char* source_path;
    const char* destination_path;
    int sleeping_time;
};

enum WakeyReason
{
    SLEEP = 0,
    SIGNAL
};

extern const int sleep_time_initializer;
extern volatile enum WakeyReason wakey_reason;