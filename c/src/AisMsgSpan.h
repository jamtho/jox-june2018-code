#pragma once

// Contiguous array of AisMsg structs
//
// Value type - keep instances of this on the stack

#include "AisMsg.h"


//  ----------------------------------------------------------------------
//  Opaque struct body (please treat a such)

typedef struct AisMsgSpan {
    const AisMsg *data;  // First elem of span, or NULL for null span
    size_t size;         // Number of elems in span
} AisMsgSpan;


//  ----------------------------------------------------------------------
//  Ctrs

static inline AisMsgSpan
AisMsgSpan_make (const AisMsg *data, size_t size) {
    assert (data);
    return (AisMsgSpan) { .data=data, .size=size };
}

static inline AisMsgSpan
AisMsgSpan_makeNull () {
    return (AisMsgSpan) { .data=NULL, .size=0 };
}


//  ----------------------------------------------------------------------
//  Accessors

static inline bool
AisMsgSpan_isNull (const AisMsgSpan self) {
    return self.data == NULL;
}

static inline const AisMsg *
AisMsgSpan_data (const AisMsgSpan self) {
    assert (! AisMsgSpan_isNull (self));
    return self.data;
}

static inline size_t
AisMsgSpan_size (const AisMsgSpan self) {
    assert (! AisMsgSpan_isNull (self));
    return self.size;
}


//  ----------------------------------------------------------------------
//  STL-style iteration

// Get pointer to first elem
static inline const AisMsg *
AisMsgSpan_begin (const AisMsgSpan self) {
    assert (! AisMsgSpan_isNull (self));
    return AisMsgSpan_data (self);
}

// Get pointer to one past last elem
static inline const AisMsg *
AisMsgSpan_end (const AisMsgSpan self) {
    assert (! AisMsgSpan_isNull (self));
    return AisMsgSpan_data (self) + AisMsgSpan_size (self);
}
