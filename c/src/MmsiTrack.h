#pragma once

// Wrapper round AisMsgSpan with one ship's data, giving
// additional functionality.
//
// Value type - keep instances of this on the stack.
//
// Assumes all messages have same MMSI and are in ascending timestamp
// order, but doesn't check.
// TODO add an optional checking function

#include "AisMsgSpan.h"


//  ----------------------------------------------------------------------
//  Opaque class body (please treat as such)

typedef struct MmsiTrack {
    const AisMsgSpan msgs;
} MmsiTrack;


//  ----------------------------------------------------------------------
//  Ctrs

static inline MmsiTrack
MmsiTrack_make (const AisMsg *data, size_t size) {
    assert (data);
    return (MmsiTrack) { .msgs = AisMsgSpan_make (data, size) };
}

static inline MmsiTrack
MmsiTrack_makeNull () {
    return (MmsiTrack) { .msgs = AisMsgSpan_makeNull () };
}

// TODO make a ctr that does a data integrity check and returns null span
// if it fails.


//  ----------------------------------------------------------------------
//  Accessors

static inline bool
MmsiTrack_isNull (const MmsiTrack self) {
    return (AisMsgSpan_isNull (self.msgs));
}

// Number of messages in track
static inline size_t
MmsiTrack_size (const MmsiTrack self) {
    assert (! MmsiTrack_isNull (self));
    return AisMsgSpan_size (self.msgs);
}


//  ----------------------------------------------------------------------
//  STL-style iteration

// Get pointer to first message in track
static inline const AisMsg *
MmsiTrack_begin (const MmsiTrack self) {
    assert (! MmsiTrack_isNull (self));
    return AisMsgSpan_begin (self.msgs);
}

// Get pointer one past last message in track
static inline const AisMsg *
MmsiTrack_end (const MmsiTrack self) {
    assert (! MmsiTrack_isNull (self));
    return AisMsgSpan_end (self.msgs);
}

    
//  ----------------------------------------------------------------------
//  Utils

static inline void
MmsiTrack_fprintln (const MmsiTrack self, FILE *file) {
    fprintf (file, "BEGIN MmsiTrack: [\n");
    
    for (const AisMsg *msg = MmsiTrack_begin (self),
                      *msg_END = MmsiTrack_end (self);
         msg != msg_END;
         ++msg)
    {
        fprintf (file, "    (AisMsg){ lat: %f, lon: %f, mmsi: %d, "
                 "timestamp: %d, course: %f, speed: %f }\n",
                 msg->lat, msg->lon, msg->mmsi, msg->timestamp,
                 msg->course, msg->speed);
    }
    
    fprintf (file, "] END MmsiTrack\n");
}
        

//  ----------------------------------------------------------------------
//  Track timestamp checking

// Get the first timestamp of the track.
// Dies on assert if the track is empty.
static inline int32_t
MmsiTrack_firstTS (const MmsiTrack self) {
    assert (MmsiTrack_size (self) > 0);
    return (MmsiTrack_begin (self))->timestamp;
}

// Get the last timestamp of the track.
// Dies on assert if the track is empty.
static inline int32_t
MmsiTrack_lastTS (const MmsiTrack self) {
    assert (MmsiTrack_size (self) > 0);
    const AisMsg *elem = MmsiTrack_begin (self) + (MmsiTrack_size (self) - 1);
    return elem->timestamp;
}

// Is the provided timestamp within the time region covered by the track?
static inline bool
MmsiTrack_coversTS (const MmsiTrack self, int32_t ts) {
    if (MmsiTrack_size (self) < 2)
        return false;
    return MmsiTrack_firstTS (self) <= ts && ts <= MmsiTrack_lastTS (self);
}
    
             
