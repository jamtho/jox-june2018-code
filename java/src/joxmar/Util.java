package joxmar;

// Static utility functions

import java.io.File;
import java.io.RandomAccessFile;
import java.nio.MappedByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;


public final class Util {

    //  ------------------------------------------------------------
    //  Condition checking
    
    public static void enforce (boolean cond) {
        // TODO better Ex class
        if (!cond)
            throw new RuntimeException ("Enforcement failed");
    }
    public static void enforce (boolean cond, String msg) {
        // TODO better Ex class
        if (!cond)
            throw new RuntimeException ("Enforcement failed: " + msg);
    }


    //  ------------------------------------------------------------
    //  Copying arrays

    public static int[] copy (int[] src) {
        int[] res = new int [src.length];
        System.arraycopy (src, 0, res, 0, src.length);
        return res;
    }
    


    //  ------------------------------------------------------------
    //  Opening memory mapped files

    public static MappedByteBuffer mmapFile_ro_le (String filepath) {
        try {
            File file = new File (filepath);
            FileChannel fileChannel = new RandomAccessFile (file, "r")
                                              .getChannel();
            MappedByteBuffer buf = fileChannel.map
                (FileChannel.MapMode.READ_ONLY, 0, fileChannel.size());
            buf.order (ByteOrder.LITTLE_ENDIAN);
            return buf;
        } catch (java.io.IOException e) {
            throw new RuntimeException (e);
        }
    }
}
    
