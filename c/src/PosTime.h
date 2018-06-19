#pragma once

#include <czmq.h>

typedef struct PosTime {
    double lat;
    double lon;
    int32_t timestamp;
    int32_t id;
} PosTime;
