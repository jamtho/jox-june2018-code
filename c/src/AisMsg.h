#pragma once

// Main AIS message datum

#include <czmq.h>

typedef struct AisMsg {
    double lat;
    double lon;
    int32_t mmsi;
    int32_t timestamp;
    float course;
    float speed;
} AisMsg;
