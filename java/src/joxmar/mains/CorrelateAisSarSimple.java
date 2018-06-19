package joxmar.mains;

// Give AISMMF file as first arg, sar PosTime file as second

import joxmar.Util;
import joxmar.GeoPos;
import joxmar.GeoUtils;
import joxmar.AISMmFile;
import joxmar.PosTime;
import joxmar.PosTimeArrayFile;
import joxmar.MmsiTrackExtrap;
import joxmar.MmsiTrackView;
import joxmar.MmsiTrackSOA;


public final class CorrelateAisSarSimple {

    public static void main (String[] args) {
        final long startTimeMillis = System.currentTimeMillis ();
        
        Util.enforce (args.length == 2);
        String aisFilepath = args [0];
        String sarFilepath = args [1];

        AISMmFile mmf = new AISMmFile (aisFilepath);
        int[] mmsis = mmf.mmsis ();
        
        PosTimeArrayFile sars = new PosTimeArrayFile (sarFilepath);

        System.out.println
            ("mmsi,sar_id,sar_timestamp,sar_lat,sar_lon,dist_metres");

        int numMmsisDone = 0;
        int numCompsDone = 0;

        // Iterate over all the mmsis
        for (int mm : mmsis) {
            ++numMmsisDone;
            
            // Skip ahead to more interesting mmsis
            if (numMmsisDone < 40*1000)
                continue;
            // And limit run to 200 mmsis
            //if (numMmsisDone > 40*1000 + 200)
            if (numMmsisDone > 40*1000 + 20)
                break;

            // Log progress
            if (numMmsisDone % 10 == 0)
                System.err.println ("-- Done mmsis: " + numMmsisDone);
            
            AISMmFile.MmsiTrackRef track = mmf.mmsi (mm);
            //MmsiTrackSOA soa = track.soa ();

            // And for each mmsi, iterate over all the sars
            for (PosTime sar : sars) {
                ++numCompsDone;
                int ts = sar.timestamp;
                //int ts = sar.timestamp();
                
                if (track.coversTimestamp (ts)) {
                    //GeoPos exPos = MmsiTrackExtrap.simple (soa, ts);
                    GeoPos exPos = MmsiTrackExtrap.simple (track, ts);
                    double distMetres = GeoUtils.distMetres
                        (sar.lat, sar.lon, exPos.lat, exPos.lon);
                    // Only bother to print if within 200m
                    if (distMetres <= 200) {
                        System.out.println (String.format
                                            ("%d,%d,%d,%f,%f,%f",
                                             mm, sar.id, ts,
                                             sar.lat, sar.lon, distMetres));
                    }
                }
            }  // sar loop
        }  // mmsi track loop

        final long endTimeMillis = System.currentTimeMillis ();

        System.err.println ("-- Total comparisons: " + numCompsDone);
        System.err.println ("-- Total mmsis:       " + numMmsisDone);

        double runSecs = (double)(endTimeMillis - startTimeMillis) / 1000.0;
        System.err.println ("-- Run time (secs): " + runSecs);

        double compsPerSec = (double)numCompsDone / runSecs;
        System.err.println ("-- Comparions per sec: " + compsPerSec);
    }
}
        


// Does 13.0mm comparions per second, with simple sar iteration (ish)
// 13.4mm when using ref based sar iteration (ish)

// Does 21.5mm 'comparions' per second when I disable the geo code, and
// just traverse the tracks

// The normal 13mm goes to 10.7mm with escape analysis disabled

// And when we do the extrapolate on an MmsiSOA, generating it just once
// per mmsi, we get 19mm comparions per second!!!! (or maybe 18, but anyway :))


            //for (PosTimeArrayFile.ElemRef sar : sars.itermut()) {
