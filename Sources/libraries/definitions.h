#pragma once

struct ProgramData
{
    const char* source_path;
    const char* destination_path;
    int sleeping_time;
};

extern const int sleep_time_initializer;