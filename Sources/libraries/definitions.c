#include "definitions.h"

const int sleep_time_initializer = 5 * 60;
volatile enum WakeyReason wakey_reason = SLEEP;
extern const int file_size_limit = 1000; // 1kB