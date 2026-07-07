#ifdef CONSOLE_ENABLE
#include "print.h"

#define LOG(msg)       \
print(msg)
#define LOGF(msg, ...) \
printf(msg, ##__VA_ARGS__)

#else
#define LOG(msg)
#define LOGF(mg, ...)
#endif
