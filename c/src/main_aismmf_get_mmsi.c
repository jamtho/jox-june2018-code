#include "AISMmFile.h"


const char *
s_usage =
    "USAGE:\n"
    "    aismmf_get_mmsi FILE.aismmf MMSI";




// Parses the base 10 int in 'str' and writes the result to 'output'.
// Returns 0 on success or -1 on failure.
// Assume 'output' is modified in either case.
// TODO make more robust to errors
static int
parseInt (const char *str, int *output) {
    assert (str);
    
    long l = strtol (str, NULL, 10);
    if (l == 0 && !streq (str, "0")) {
        return -1;
    } else {
        *output = (int) l;
        return 0;
    }
}



int main (int argc, char **argv) {
    if (argc != 3)
        goto die;

    char *filepath = argv [1];

    int mmsi = -1;
    int ret = parseInt (argv [2], &mmsi);
    if (ret)
        goto die;

    AISMmFile_t *mmf = AISMmFile_new (filepath);
    if (!mmf)
        goto die;

    if (! AISMmFile_hasMmsi (mmf, mmsi)) {
        fprintf (stderr, "MMSI NOT FOUND IN FILE: %d\n", mmsi);
        goto die;
    }

    MmsiTrack track = AISMmFile_mmsiTrack (mmf, mmsi);
    MmsiTrack_fprintln (track, stdout);

    AISMmFile_destroy (&mmf);
    return 0;

 die:
    AISMmFile_destroy (&mmf);
    
    fprintf (stderr, "============================\n");
    fprintf (stderr, "Died with an error\n");
    fprintf (stderr, "%s\n", s_usage);

    exit (1);
}
