#pragma once

#include "MmsiTrack.h"
#include "MmFile.h"
#include "ByteSpan.h"

#include <czmq.h>


//  ----------------------------------------------------------------------
//  INTERNAL: Header section entry

typedef struct AISMmFile__HeadEnt {
    int32_t mmsi, offset, length, _padding;
} AISMmFile__HeadEnt;


//  ----------------------------------------------------------------------
//  Opaque class body

typedef struct AISMmFile_t {
    MmFile_t *file;
    
    int64_t numMmsis;

    ByteSpan bs_file;   // from mmfile
    ByteSpan bs_header; // size calc'd from numMmsis 
    ByteSpan bs_body;   // bytes left over in file, not checked against header

    const AISMmFile__HeadEnt *headEnts;  // ptr to bs_header data
    const AisMsg             *bodyMsgs;  // ptr to bs_body data
} AISMmFile_t;


//  ----------------------------------------------------------------------
//  Ctrs, dtr

// Dtr
static inline void
AISMmFile_destroy (AISMmFile_t **self_p) {
    assert (self_p);
    if (*self_p) {
        AISMmFile_t *self = *self_p;
        MmFile_destroy (&self->file);
        free (self);
        *self_p = NULL;
    }
}

// Ctr
static inline AISMmFile_t *
AISMmFile_new (const char *filepath) {
    assert (filepath);
    
    AISMmFile_t *self = calloc (1, sizeof (*self));
    assert (self);

    // MMap the file
    self->file = MmFile_new_rdonly (filepath);
    if (!self->file)
        goto die;

    // Load the number-of-mmsis value
    memcpy (&self->numMmsis, MmFile_data (self->file), sizeof(int64_t));

    // Make spans of the sections in the file
    
    self->bs_file = ByteSpan_make (MmFile_data (self->file),
                                   MmFile_size (self->file));

    int headerPos = sizeof (int64_t);
    int headerLen = self->numMmsis * sizeof (AISMmFile__HeadEnt);
    self->bs_header = ByteSpan_subspan (self->bs_file, headerPos, headerLen);

    size_t bodyPos = sizeof (int64_t) + headerLen;
    size_t bodyLen = ByteSpan_size (self->bs_file) - bodyPos;
    self->bs_body = ByteSpan_subspan (self->bs_file, bodyPos, bodyLen);

    // Bail if file is corrupt
    if (! ByteSpan_contains_ByteSpan (self->bs_file, self->bs_header))
        goto die;
    if (! ByteSpan_contains_ByteSpan (self->bs_file, self->bs_body))
        goto die;

    // Make the easy to use handles to the byte spans
    self->headEnts = (const AISMmFile__HeadEnt *) ByteSpan_data (self->bs_header);
    self->bodyMsgs = (const AisMsg *) ByteSpan_data (self->bs_body);

    return self;

 die:
    AISMmFile_destroy (&self);
    return NULL;
}


//  ----------------------------------------------------------------------
//  MMSI values access

// Get a pointer to the first mmsi value in the file header, or NULL if none exist
static inline const int32_t *
AISMmFile_mmsi_first (const AISMmFile_t *self) {
    assert (self);
    
    if (self->numMmsis == 0)
        return NULL;

    const AISMmFile__HeadEnt *ent = (const AISMmFile__HeadEnt *)
                                        ByteSpan_data (self->bs_header);
    return &ent->mmsi;
}

// Given a pointer to an mmsi value in the header from _first() or _next(),
// returns a pointer to the next, or NULL if there are no more left
static inline const int32_t *
AISMmFile_mmsi_next (const AISMmFile_t *self, const int32_t *cur) {
    assert (self);
    assert (ByteSpan_contains (self->bs_header, cur));

    // Find the header entry we're on
    const byte *cur_bytes = (const byte *) cur;
    const byte *he_bytes  = cur_bytes - offsetof (AISMmFile__HeadEnt, mmsi);
    const AISMmFile__HeadEnt *he = (const AISMmFile__HeadEnt *) he_bytes;
    assert (ByteSpan_contains (self->bs_header, he));

    // Which number header entry is this?
    int heNum = (int) (he - self->headEnts);
    /* fprintf(stderr, "--- heNum: %d\n", heNum); */
    
    // If we've done enough give up
    if ((heNum + 1) >= self->numMmsis) {
        return NULL;
    // Otherwise find the next header entry and return a ptr to its mmsi
    } else {
        const AISMmFile__HeadEnt *nextHe = he + 1;
        assert (ByteSpan_contains (self->bs_header, nextHe));
        return &nextHe->mmsi;
    }
}


//  ----------------------------------------------------------------------
//  Checking if an MMSI is present in the file

static inline bool
AISMmFile_hasMmsi (const AISMmFile_t *self, int32_t mmsi) {
    assert (self);

    const int32_t *mmsip = AISMmFile_mmsi_first (self);
    while (mmsip) {
        if (mmsi == *mmsip)
            return true;
        mmsip = AISMmFile_mmsi_next (self, mmsip);
    }

    return false;
}


//  ----------------------------------------------------------------------
//  Fetching tracks

static inline MmsiTrack
AISMmFile_mmsiTrack (const AISMmFile_t *self, int32_t mmsi) {
    // Loop over the header entries to find the one for this mmsi
    for (const AISMmFile__HeadEnt *he = self->headEnts;
         he - self->headEnts < self->numMmsis;
         ++he)
    {
        if (he->mmsi == mmsi) {  // Got it, return the span
            const AisMsg *firstMsg = self->bodyMsgs + he->offset;

            // Check range is within mmap and bail if not
            ByteSpan datas = ByteSpan_make ((const void *)firstMsg,
                                            he->length*sizeof(AisMsg));
            if (! ByteSpan_contains_ByteSpan (self->bs_body, datas)) {
                assert (0 && "Track data outside mmaped region");
                return MmsiTrack_makeNull ();
            }

            return MmsiTrack_make (firstMsg, he->length);
        }
    }

    // Didn't find such a header, return NULL track
    assert (0 && "Called _mmsiTrack() with non-present MMSI");
    return MmsiTrack_makeNull ();
}

            













            //if (! ByteSpan_contains_ByteSpan (

            // Bail if header points outside file
            // FIXME the lastMsg check isn't enough, as assumes struct has sizeof 1
            /* if (   !AISMmFile__ptrInBody (self, firstMsg) */
            /*     || !AISMmFile__ptrInBody (self, lastMsg)) */
            /*         return MmsiTrack_makeNull (); */
            /* else */
            /*     return MmsiTrack_make (firstMsg, he->length); */
    




//  ----------------------------------------------------------------------
//  INTERNAL: Utils

/* // Returns true iff ptr is in address range [start, pastEnd) */
/* static inline bool */
/* AISMmFile__ptrInRange (const void *ptr, const void *start, const void *pastEnd) { */
/*     assert (ptr); */
/*     assert (start); */
/*     assert (pastEnd); */

/*     fprintf (stderr, "AISMmFile__ptrInRange(): ptr: %p, start: %p, pastEnd: %p\n", */
/*              ptr, start, pastEnd); */

/*     uintptr_t ptr_ui     = (uintptr_t) ptr; */
/*     uintptr_t start_ui   = (uintptr_t) start; */
/*     uintptr_t pastEnd_ui = (uintptr_t) pastEnd; */

/*     fprintf (stderr, "RESULT: %d\n", start_ui <= ptr_ui && ptr_ui < pastEnd_ui); */

/*     return start_ui <= ptr_ui && ptr_ui < pastEnd_ui; */
/* } */

/* static inline bool */
/* AISMmFile__ptrInHeader (const AISMmFile_t *self, const void *ptr) { */
/*     assert (self); */
/*     assert (ptr); */

/*     const AISMmFile__HeadEnt *headerPastEnd = self->headerStart + self->numMmsis; */
/*     return AISMmFile__ptrInRange (ptr, (const void *) self->headerStart, */
/*                                   (const void *) headerPastEnd); */
/* } */

/* static inline bool */
/* AISMmFile__ptrInBody (const AISMmFile_t *self, const void *ptr) { */
/*     assert (self); */
/*     assert (ptr); */

/*     const byte *fileStart = MmFile_data (self->file); */
/*     const byte *fileEnd   = fileStart + MmFile_size (self->file); */
    
/*     return AISMmFile__ptrInRange (ptr, (const void *) self->bodyStart, */
/*                                   (const void *) fileEnd); */
/* } */







    

    /* // The first 8 bytes of the file are the MMSI count */
    /* memcpy (&self->numMmsis, MmFile_data(self->file), sizeof(int64_t)); */
    /* fprintf(stderr, "Copied out num mmsis: %zi\n", self->numMmsis); */

    /* //  */
    
    /* self->bs_file = ByteSpan_make (MmFile_data (self->file), */
    /*                                MmFile_size (self->file)); */
    /* self->bs_header = ByteSpan_make (Mm */

    /* self->headerStart = (const AISMmFile__HeadEnt *) */
    /*                         (MmFile_data (self->file) + 8); */
    /* /\* assert (AISMmFile__ptrInHead (self, (void *) self->headerStart));  // sanity *\/ */

    /* self->bodyStart = (const AisMsg *) */
    /*     (self->headerStart + (self->numMmsis * sizeof (AISMmFile__HeadEnt))); */
    /* /\* assert (AISMmFile__ptrInBody (self, (void *) self->bodyStart));  // sanity *\/ */

    /* return self; */




    /* fprintf (stderr, "bs_file: data: %p, size: %zu\n", */
    /*          ByteSpan_data (self->bs_file), ByteSpan_size(self->bs_file)); */
    /* fprintf (stderr, "bs_header: data: %p, size: %zu\n", */
    /*          ByteSpan_data (self->bs_header), ByteSpan_size(self->bs_header)); */
    /* fprintf (stderr, "bs_body: data: %p, size: %zu\n", */
    /*          ByteSpan_data (self->bs_body), ByteSpan_size(self->bs_body)); */




//        fprintf(stderr, "Num mmsis: %zi\n", self->numMmsis);
        /* ByteSpan_fprintln(self->bs_file, stderr); */
        /* ByteSpan_fprintln(self->bs_header, stderr); */
        /* ByteSpan_fprintln(self->bs_body, stderr); */
        
    /* //size_t curNum = he - self->headEnts; */
    /* if ((int32_t) curNum < (self->numMmsis - 1)) { */
    /*     const int32_t *res = &( (he + 1)->mmsi ); */
    /*     assert (ByteSpan_contains (self->bs_header, res)); */
    /*     return res; */
    /* } else { */
    /*     return NULL; */
    /* } */



            /* fprintf(stderr, */
            /*         "FOUND MMSI TRACK mmsi: %d, length: %d, offset: %d\n", */
            /*         he->mmsi, he->length, he->offset); */
            /* fprintf (stderr, "offset * sizeof(AisMsg): %zu\n", */
            /*          he->offset * sizeof(AisMsg)); */
            /* fprintf (stderr, "MMAP size: %zu\n", MmFile_size(self->file)); */
            
