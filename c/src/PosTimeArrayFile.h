#pragma once

// Read only (mmapped) view into a file containing an array of PosTime structs

#include <czmq.h>

#include "PosTime.h"
#include "MmFile.h"


//  ----------------------------------------------------------------------
//  Opaque class handle

typedef struct PosTimeArrayFile_t {
    MmFile_t *mmfile;
    
    const PosTime *firstElem;
    size_t         numElems;
} PosTimeArrayFile_t;


//  ----------------------------------------------------------------------
//  Ctr, dtr

static inline void
PosTimeArrayFile_destroy (PosTimeArrayFile_t **self_p) {
    assert (self_p);
    if (*self_p) {
        PosTimeArrayFile_t *self = *self_p;
        
        MmFile_destroy (&self->mmfile);
        
        free (self);
        *self_p = NULL;
    }
}

// Dies if the file length isn't an even multiple of PosTime size
static inline PosTimeArrayFile_t *
PosTimeArrayFile_new (const char *filepath) {
    PosTimeArrayFile_t *self = calloc (1, sizeof (*self));
    assert (self);
    
    self->mmfile = MmFile_new_rdonly (filepath);
    if (! self->mmfile)
        goto die;

    self->firstElem = (const PosTime *) MmFile_data (self->mmfile);
    
    if (MmFile_size (self->mmfile) % sizeof (PosTime) == 0) {
        self->numElems = MmFile_size (self->mmfile) / sizeof (PosTime);
    } else {
        goto die;  // invalid file length
    }
    
    return self;

 die:
    PosTimeArrayFile_destroy (&self);
    return NULL;
}


//  ----------------------------------------------------------------------
//  Accessors

// Number of stored PosTime elems
static inline size_t
PosTimeArrayFile_size (const PosTimeArrayFile_t *self) {
    assert (self);
    return self->numElems;
}


//  ----------------------------------------------------------------------
//  STL-style iteration

// Pointer to first elem
static inline const PosTime *
PosTimeArrayFile_begin (const PosTimeArrayFile_t *self) {
    assert (self);
    return self->firstElem;
}

// Pointer to one past last elem
static inline const PosTime *
PosTimeArrayFile_end (const PosTimeArrayFile_t *self) {
    assert (self);
    return PosTimeArrayFile_begin (self) + PosTimeArrayFile_size (self);
}
