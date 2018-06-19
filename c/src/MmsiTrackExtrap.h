#pragma once

// MMSI track extrapolation functions

#include "PosTime.h"
#include "MmsiTrack.h"


//  ----------------------------------------------------------------------
//  INTERNAL: Utils

static inline double
MmsiTrackExtrap__lerp (double start, double end, double proportion) {
    double delta = end - start;
    return start + (proportion * delta);
}
        

//  ----------------------------------------------------------------------
//  Track extrapolation functions

// Using constant-speed straight lines between points, extrapolates a
// position on the provided track at the timestamp 'ts'.
// Dies on an assert if 'ts' isn't covered by the track.
static inline PosTime
MmsiTrackExtrap_simple (const MmsiTrack track, int32_t ts) {
    assert (MmsiTrack_coversTS (track, ts));

    // To hold the messages just before and after 'ts'
    const AisMsg *bef = NULL;
    const AisMsg *aft = NULL;

    // Shuffle messages through bef and aft until we get a ts-valid pair
    for (const AisMsg *msg = MmsiTrack_begin (track),
                      *msg_END = MmsiTrack_end (track);
         msg != msg_END;
         ++msg)
    {
        bef = aft;
        aft = msg;
        if (bef && aft &&
            bef->timestamp <= ts && ts <= aft->timestamp)
                break;
    }

    double prop =   (double)(ts             - bef->timestamp)
                  / (double)(aft->timestamp - bef->timestamp);
    
    double lat = MmsiTrackExtrap__lerp (bef->lat, aft->lat, prop);
    double lon = MmsiTrackExtrap__lerp (bef->lon, aft->lon, prop);

    return (PosTime) { .lat=lat, .lon=lon, .timestamp=ts, .id=-1 };
}

