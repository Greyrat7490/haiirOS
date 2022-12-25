#ifndef H_TIME
#define H_TIME

#include "types.h"

typedef uint64_t time_t;

typedef struct {
    time_t tv_sec;
    uint64_t tv_nsec;
} timespec_t;

#endif // H_TIME
