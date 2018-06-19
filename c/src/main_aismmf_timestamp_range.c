// Program that prints the lowest and highest timestamps stored in
// an aismmf file

#include "AISMmFile.h"
#include "MmsiTrack.h"
#include "AisMsg.h"

#include <czmq.h>
#include <time.h>


//  ----------------------------------------------------------------------
//  Utils

// Make a time string from a timestamp.
// Caller must free return value.
char * timestampStr (int32_t ts) {
    time_t tst = ts; //time (ts);
    struct tm *tmstruct = gmtime (&tst);
    char formatted [20];
    strftime (formatted, 20, "%Y-%m-%d %H:%M:%S", tmstruct);
    return strdup (formatted);
}


//  ----------------------------------------------------------------------
//  Program usage info

static const char *s_usage =
    "USAGE:\n"
    "    aismmf_timestamp_range FILE.mmf\n"
    "\n"
    "Prints the min and max timestamps stored in any message in the file";


//  ----------------------------------------------------------------------
//  main()

int main (int argc, char **argv) {
    const char *error = NULL;  // set on error

    if (argc != 2) {
        error = "Too few arguments";
        goto die;
    }
    char *filepath = argv[1];

    AISMmFile_t *mmf = AISMmFile_new (filepath);
    if (!mmf) {
        error = "AISMmFile failed to open";
        goto die;
    }

    // Values we're going to output
    int lowestTS = 0;
    int highestTS = 0;
    bool haveTSs = false;

    // Loop over MMSIs
    for (const int32_t *mmsip = AISMmFile_mmsi_first (mmf);
         mmsip;
         mmsip = AISMmFile_mmsi_next (mmf, mmsip))
    {
        MmsiTrack track = AISMmFile_mmsiTrack (mmf, *mmsip);
        assert (! MmsiTrack_isNull (track));

        // Loop over each MMSI's messages
        for (const AisMsg *msg     = MmsiTrack_begin (track),
                          *msg_END = MmsiTrack_end (track);
             msg != msg_END;
             ++msg)
        {
            assert (msg->mmsi == *mmsip);
            
            if (!haveTSs) {
                haveTSs = true;
                lowestTS = msg->timestamp;
                highestTS = msg->timestamp;
            }
            else {
                if (msg->timestamp < lowestTS)
                    lowestTS = msg->timestamp;
                if (msg->timestamp > highestTS)
                    highestTS = msg->timestamp;
            }
        }
    }
 
    // Print results
    if (!haveTSs) {
        puts ("No data in file");
    }
    else {
        char *lowestTS_str  = timestampStr (lowestTS);
        char *highestTS_str = timestampStr (highestTS);
        
        printf ("Lowest timestamp:  %d - %s\n",
                lowestTS, lowestTS_str);
        printf ("Highest timestamp: %d - %s\n",
                highestTS, highestTS_str);
        
        free (lowestTS_str);
        free (highestTS_str);
    }

    // Cleanup and close
    AISMmFile_destroy (&mmf);
    exit (0);
    
    // Error handler
 die:
    AISMmFile_destroy (&mmf);
    puts ("=====================================");
    printf ("Died with error: %s\n", error);
    puts ("");
    puts (s_usage);
    puts ("=====================================");
    exit (1);
}
    
