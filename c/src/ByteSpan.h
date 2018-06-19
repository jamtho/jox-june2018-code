#pragma once

// Span of contiguous array of const bytes
//
// Value type, doesn't own contents

#include <czmq.h>


//  ----------------------------------------------------------------------
//  Opaque class body

typedef struct ByteSpan {
    const byte *data;
    size_t size;  // number of bytes
} ByteSpan;


//  ----------------------------------------------------------------------
//  Ctrs

static inline ByteSpan 
ByteSpan_make (const byte *data, size_t size) {
    assert (data);
    return (ByteSpan) { .data=data, .size=size };
}

static inline ByteSpan
ByteSpan_makeNull () {
    return (ByteSpan) { .data=NULL, .size=0 };
}


//  ----------------------------------------------------------------------
//  Utils

// Check if span is null span
static inline bool
ByteSpan_isNull (const ByteSpan self) {
    return self.data == NULL;
}

// Print a human-readable representation to a file stream, followed
// by a newline
static inline void
ByteSpan_fprintln (const ByteSpan self, FILE *file) {
    fprintf (file, "(ByteSpan) { .data: %p, .size: %zu }\n",
             self.data, self.size);
}


//  ----------------------------------------------------------------------
//  Accessors
//    These die on an assert if you call for a null ByteSpan, so make
//    sure to check first if a null val is semantically valid in your case
//    TODO reconsider whether this is what I want

inline static const byte *
ByteSpan_data (const ByteSpan self) {
    assert (! ByteSpan_isNull (self));
    return self.data;
}

inline static size_t
ByteSpan_size (const ByteSpan self) {
    assert (! ByteSpan_isNull (self));
    return self.size;
}


// -- Check if pointers or ranges are inside the span

static inline bool
ByteSpan_contains (const ByteSpan self, const void *ptr) {
    assert (! ByteSpan_isNull (self));

    uintptr_t start   = (uintptr_t) self.data;
    uintptr_t pastEnd = start + self.size;
    uintptr_t ptr_ui  = (uintptr_t) ptr;

    return start <= ptr_ui && ptr_ui <= pastEnd;
}

static inline bool
ByteSpan_contains_ByteSpan (const ByteSpan self, const ByteSpan other) {
    assert (! ByteSpan_isNull (self));
    assert (! ByteSpan_isNull (other));

    const byte  *otherStart = ByteSpan_data (other);
    const size_t otherSz    = ByteSpan_size (other);
    
    if (otherSz == 0) {
        return ByteSpan_contains (self, otherStart);
    } else {
        return    ByteSpan_contains (self, otherStart)
               && ByteSpan_contains (self, otherStart + (otherSz - 1));
    }
}


//  ----------------------------------------------------------------------
//  Getting a sub-span

// Offset is how far into the span you want to start from (zero indexed),
// length is how many bytes you want from this point
static inline ByteSpan
ByteSpan_subspan (const ByteSpan self, size_t offset, size_t length) {
    assert (offset + length <= self.size);

    return ByteSpan_make (self.data + offset, length);
}

    
