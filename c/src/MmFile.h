#pragma once

// Memory mapped file
//
// Only tested on Linux (and definitely won't work on windows as is)

#include <sys/mman.h>

#include <czmq.h>





/* int testwmmap(char * name, int N, bool advise, bool shared) { */
/*    int answer = 0; */
/*    int fd = ::open(name, O_RDONLY); */
/*    size_t length = N * (512 + 1) * 4; */
/*    // for Linux: */
/* #ifdef __linux__ */
/*    int *  addr = reinterpret_cast<int *>(mmap(NULL, length, PROT_READ, MAP_FILE | (shared?MAP_SHARED:MAP_PRIVATE) | MAP_POPULATE , fd, 0)); */
/* #else */
/*    int *  addr = reinterpret_cast<int *>(mmap(NULL, length, PROT_READ, MAP_FILE | (shared?MAP_SHARED:MAP_PRIVATE), fd, 0)); */
/* #endif */
/*    int * initaddr = addr; */
/*    if (addr == MAP_FAILED) { */
/*          cout<<"Data can't be mapped???"<<endl; */
/*            return -1; */
/*    } */
/*    if(advise) */
/*      if(madvise(addr,length,MADV_SEQUENTIAL|MADV_WILLNEED)!=0) */
/*        cerr<<" Couldn't set hints"<<endl; */
/*    close(fd); */
/*    for(int t = 0; t < N; ++t) { */
/*        int size = *addr++; */
/*        answer += doSomeComputation(addr,size); */
/*        addr+=size; */
/*    } */
/*    munmap(initaddr,length); */
/*    return answer; */
/* } */



//  ----------------------------------------------------------------------
//  Opaque class body

typedef struct MmFile_t {
    zfile_t *file;
    const void *mappedRegion;
    size_t length;
} MmFile_t;


//  ----------------------------------------------------------------------
//  Ctr, dtr

// Dtr
static inline void
MmFile_destroy (MmFile_t **self_p) {
    assert (self_p);
    if (*self_p) {
        MmFile_t *self = *self_p;

        munmap ((void*)self->mappedRegion, self->length);

        zfile_destroy (&self->file);
        
        free (self);
        *self_p = NULL;
    }
}

// Ctr
static inline MmFile_t *
MmFile_new_rdonly (const char *filepath) {
    assert (filepath);
    
    MmFile_t *self = calloc (1, sizeof (*self));
    assert (self);

    self->file = zfile_new (NULL, filepath);
    if (!self->file)
        goto die;
    
    int ret = zfile_input (self->file);
    if (ret != 0)
        goto die;

    self->length = zfile_cursize (self->file);

    self->mappedRegion = mmap (NULL, self->length, PROT_READ,
                               // TODO experiment with and w/o populate
                               MAP_SHARED | MAP_POPULATE,
                               fileno (zfile_handle (self->file)), 0);
    if (self->mappedRegion == MAP_FAILED)
        goto die;

    // TODO experiment with madvise, e.g.
    /*      if(madvise(addr,length,MADV_SEQUENTIAL|MADV_WILLNEED)!=0) */

    return self;

 die:
    MmFile_destroy (&self);
    return NULL;
}


//  ----------------------------------------------------------------------
//  Accessors

// Pointer to first byte in mapped section
// TODO reconsider const
static inline const byte *
MmFile_data (MmFile_t *self) {
    assert (self);
    return self->mappedRegion;
}

// Number of bytes in mapped file
static inline size_t
MmFile_size (MmFile_t *self) {
    assert (self);
    return self->length;
}
    
