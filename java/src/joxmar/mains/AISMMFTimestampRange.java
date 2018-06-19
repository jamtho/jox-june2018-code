package joxmar.mains;

import joxmar.Util;
import joxmar.AISMmFile;
import joxmar.AisMsg;
import joxmar.AisMsgView;

public final class AISMMFTimestampRange {
    public static void main (String[] args) {
        Util.enforce (args.length == 1);
        String filepath = args [0];
        System.err.println ("Reading from file: " + filepath);

        AISMmFile mmf = new AISMmFile (filepath);
        int[] mmsis = mmf.mmsis ();

        int minTS = -1;
        int maxTS = -1;
        boolean have_TSs = false;

        int numMmsis = 0;
        int numMsgs = 0;

        for (int mm : mmsis) {
            ++numMmsis;
            
            for (AisMsg msg : mmf.mmsi(mm)) {
            //for (AISMmFile.MsgRef msg : mmf.mmsi(mm).itermut()) {
                ++numMsgs;

                //int ts = msg.timestamp();
                int ts = msg.timestamp;
                
                if (! have_TSs) {
                    minTS = ts;
                    maxTS = ts;
                    have_TSs = true;
                } else {
                    if (ts < minTS)
                        minTS = ts;
                    
                    if (maxTS < ts)
                        maxTS = ts;
                }
            }
        }

        System.out.println ("");
        System.out.println ("Min timestamp: " + minTS);
        System.out.println ("Max timestamp: " + maxTS);
        System.out.println ("");
        System.out.println ("Num mmsis:     " + numMmsis);
        System.out.println ("Num messages:  " + numMsgs);
    }
}



// Normally runs in about 410ms, using the simple (allocating) track iterator.
//
// Reduces to around 330ms when using the IterMut (mutating pointer) iterator.
