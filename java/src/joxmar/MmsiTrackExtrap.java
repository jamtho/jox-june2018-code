package joxmar;


public class MmsiTrackExtrap {

    //  ----------------------------------------------------------------------
    //  Public API

    // SOA version
    public static GeoPos simple (MmsiTrackSOA track, int ts) {
        for (int i=0; i < track.size() - 1; ++i) {
            if (track.msgTimestamp (i) <= ts && ts <= track.msgTimestamp (i+1))
                //return new GeoPos (0, 0);
                return posAtTimeBetweenMsgs (track.msg (i), track.msg (i+1), ts);
        }
        Util.enforce (false, "Track didn't cover timestamp");
        return null;  // pacify compiler
    }

    // Main version
    public static GeoPos simple (AISMmFile.MmsiTrackRef track, int ts) {
        assert (track.coversTimestamp (ts));
        assert (track.size() >= 2);

        // STYLE 1
        for (int i=0; i < track.size() - 1; ++i) {
            if (track.msgTimestamp (i) <= ts && ts <= track.msgTimestamp (i+1))
                //return new GeoPos (0, 0);
                return posAtTimeBetweenMsgs (track.msg (i), track.msg (i+1), ts);
        }

        // // STYLE 2
        // AisMsg msg1 = null;
        // AisMsg msg2 = null;
        // for (AisMsg newMsg : track) {
        //     msg1 = msg2;
        //     msg2 = newMsg;
        //     if (msg1 != null && msg1.timestamp <= ts && ts <= msg2.timestamp) {
        //         return posAtTimeBetweenMsgs (msg1, msg2, ts);
        //     }
        // }

        // // STYLE 3
        // AisMsg msg1 = new AisMsg ();
        // AisMsg msg2 = new AisMsg ();
        // int i = 0;
        // for (AISMmFile.MsgRef newMsg : track.itermut()) {
        //     msg2.copyInto (msg1);
        //     newMsg.copyInto (msg2);
        //     if (i > 0 && msg1.timestamp <= ts && ts <= msg2.timestamp)
        //         return posAtTimeBetweenMsgs (msg1, msg2, ts);
        //     ++i;
        // }

        // // STYLE 4
        // AisMsg msg1 = new AisMsg ();
        // AisMsg msg2 = new AisMsg ();
        // int i = 0;
        // for (AisMsg newMsg : track) {
        //     msg2.copyInto (msg1);
        //     newMsg.copyInto (msg2);
        //     if (i > 0 && msg1.timestamp <= ts && ts <= msg2.timestamp) {
        //         return posAtTimeBetweenMsgs (msg1, msg2, ts);
        //     }
        //     ++i;
        // }

        Util.enforce (false, "Track didn't cover timestamp");
        return null;  // pacify compiler
    }


    //  ------------------------------------------------------------
    //  Private utils
    
    private static GeoPos posAtTimeBetweenMsgs (AisMsg m1, AisMsg m2, int ts) {
        assert (m1.timestamp <= ts);
        assert (ts           <= m2.timestamp);

        double partThrough =   ((double)(ts - m1.timestamp))
                             / ((double)(m2.timestamp - m1.timestamp));
        return new GeoPos (lerp (m1.lat, m2.lat, partThrough),
                           lerp (m1.lon, m2.lon, partThrough));
    }

    private static double lerp (double start, double end, double proportion) {
        double delta = end - start;
        return start + (proportion * delta);
    }
}
        



// Assuming "STYLE 1" for(int i=0... for loop iteraton of track allows
// correlation at 13mm track/sar comparisons per second...
// 
// Changing to "SYTLE 2" with two AisMsg's and an allocating for loop,
// makes TONS of garbage, and speed drops to 2.8mm comparions per second
//
// "STYLE 3" is also very bad, 4.5mm comparions per second. Still working
// out why... Making the MsgRef.copyMsgInto() call concrete rather than
// virtual doesn't seem to change anything
//
// With STYLE 1, when I remove the offset check on MmsiTrackRef.msgTimestamp,
// it makes absolutely no difference to the performance.
//
// STYLE 4 gets 5.8mm msgs per second, i.e. faster than 3, and makes much
// much less garbage than it. Obviously EA works there.

