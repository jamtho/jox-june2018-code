#include "PosTimeArrayFile.h"
#include "AISMmFile.h"
#include "MmsiTrackExtrap.h"
#include "GeoUtils.h"


//  ----------------------------------------------------------------------
//  Help text

const char *
s_usage =
    "USAGE:\n"
    "    correlate_ais_sar_simple AIS.mmf SAR.bin\n";


//  ----------------------------------------------------------------------
//  main()

int main (int argc, char **argv) {
    char *error = NULL;
    int64_t startTimeMillis = zclock_mono ();

    //  --------------------------------------------------
    //  Get cli args
    
    if (argc != 3) {
        error = "Wrong number of args";
        goto die;
    }
    char *mmf_filepath = argv [1];
    char *sar_filepath = argv [2];

    //  --------------------------------------------------
    //  Open files

    AISMmFile_t *mmf = AISMmFile_new (mmf_filepath);
    if (!mmf) {
        error = "Failed to open AISMmFile";
        goto die;
    }
    PosTimeArrayFile_t *sars = PosTimeArrayFile_new (sar_filepath);
    if (!sars) {
        error = "Failed to open SAR file";
        goto die;
    }


    //  --------------------------------------------------
    //  Do the correlation process...

    puts ("mmsi,sar_id,sar_timestamp,sar_lat,sar_lon,dist_metres");

    int numMmsisDone = 0;
    long numCompsDone = 0;

    // Iterate over all the mmsis
    for (const int *mmsip = AISMmFile_mmsi_first (mmf);
         mmsip;
         mmsip = AISMmFile_mmsi_next (mmf, mmsip))
    {
        ++numMmsisDone;
        
        // Skip ahead to the more interesting mmsis
        if (numMmsisDone < 40*1000)
            continue;

        // Limit the run for now
        if (numMmsisDone > (40*1000 + 200))
            break;

        // Ongoing status log
        if (numMmsisDone % 10 == 0)
            fprintf (stderr, "-- Done mmsis: %d\n", numMmsisDone);
                     
        MmsiTrack track = AISMmFile_mmsiTrack (mmf, *mmsip);
        
        // Then all the sar points, for each mmsi
        for (const PosTime *sar = PosTimeArrayFile_begin (sars),
                           *sar_END = PosTimeArrayFile_end (sars);
             sar != sar_END;
             ++sar)
        {
            ++numCompsDone;
            
            int32_t ts = sar->timestamp;
            
            // Ignore sar/ais pair if ais track doesn't include sar ts
            if (! MmsiTrack_coversTS (track, ts))
                continue;

            PosTime exPoint = MmsiTrackExtrap_simple (track, ts);
            double distMetres = GeoUtils_distMetres (sar->lat, sar->lon,
                                                     exPoint.lat, exPoint.lon);
                
            // Only print the results if we we're within 200m
            if (distMetres <= 200) {
                printf ("%d,%d,%d,%f,%f,%f\n",
                        *mmsip, sar->id, ts,
                        sar->lat, sar->lon, distMetres);
            }
        }
    }
    

    //  --------------------------------------------------
    //  All done, cleanup and return

    fprintf (stderr, "\n");
    fprintf (stderr, "-- Total comparisons: %zi\n", numCompsDone);
    fprintf (stderr, "-- Total mmsis:       %d\n", numMmsisDone);

    int64_t endTimeMillis = zclock_mono ();
    double runTimeSecs = (double)(endTimeMillis - startTimeMillis) / 1000.0;
    fprintf (stderr, "-- Run time (secs):    %f\n", runTimeSecs);

    double compsPerSec = (double)numCompsDone / runTimeSecs;
    fprintf (stderr, "-- Comparions per sec: %f\n", compsPerSec);
    
    AISMmFile_destroy (&mmf);
    PosTimeArrayFile_destroy (&sars);
    
    exit (0);
        

    //  --------------------------------------------------
    //  Error handler
    
 die:
    AISMmFile_destroy (&mmf);
    PosTimeArrayFile_destroy (&sars);
    
    fprintf (stderr, "=============================================\n");
    fprintf (stderr, "Died with error: %s\n", error);
    fprintf (stderr, "=============================================\n");
    
    exit (1);
}
    



            



// 19mm comparions (track v sardet) per second, assuming 40k mmsis skipped
// and 200 mmsis compared

