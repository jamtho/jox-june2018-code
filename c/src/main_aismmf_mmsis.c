#include "AISMmFile.h"


const char *
s_usage =
    "USAGE:\n"
    "    aismmf_mmsis FILE.aismmf";




int main (int argc, char **argv) {
    if (argc != 2)
        goto die;

    char *filepath = argv [1];

    AISMmFile_t *mmf = AISMmFile_new (filepath);
    if (!mmf)
        goto die;

    puts ("mmsi");

    for (const int32_t *mmsip = AISMmFile_mmsi_first (mmf);
         mmsip;
         mmsip = AISMmFile_mmsi_next (mmf, mmsip))
    {
        printf ("%d\n", *mmsip);
    }

    AISMmFile_destroy (&mmf);
    return 0;

 die:
    AISMmFile_destroy (&mmf);
    
    fprintf (stderr, "============================\n");
    fprintf (stderr, "Died with an error\n");
    fprintf (stderr, "%s\n", s_usage);

    exit (1);
}
