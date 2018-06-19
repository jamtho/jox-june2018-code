package joxmar;

// Reader for aismmf files. It's relatively fast.
//
// You probably want just want the mmsi(int mmsi) method, as it's the easiest
// to use. However, we provide a host of other methods in the class for data
// access too in case your specific performance requirements are different.
//
// Binary structures in file
// -------------------------
// 
// typedef int64_t NumberOfMmsisInFile;
// 
// AisMsg struct: {
//     double lat;
//     double lon;
//     int32_t mmsi;
//     int32_t timestamp;
//     float course;
//     float speed;
// };
// 
// Header entry struct: {
//     int32_t mmsi;
//     int32_t offset;
//     int32_t length;
//     int32_t _padding;
// };

import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.ByteOrder;
import java.util.Iterator;
import java.util.HashMap;


public final class AISMmFile {
    private static final int MMSI_COUNT_SIZE = 8;
    private static final int AISMSG_SIZE = 32;
    private static final int HEADENT_SIZE = 16;

    private final MappedByteBuffer _file;
    private final FileHeader _header;
    private final int _headerSizeBytes;  // calc just once, includes mmsi count
    private final int[] _mmsis;  // calc just once for speed


    //  ------------------------------------------------------------
    //  What mmsis does the file contain?

    public int[] mmsis () { return Util.copy (_mmsis); }


    //  ------------------------------------------------------------
    //  File header data storage (all in ram, read from file at init)

    private final class FileHeader {
        // Map from mmsis to relevant header entry
        private final HashMap <Integer, HeaderEntry> _ents = new HashMap<>();
        private final Vec.Int _mmsis = new Vec.Int();  // allow same-order traversal
        
        private FileHeader (ByteBuffer file) {
            ByteBuffer buf = file.duplicate ();
            buf.order (ByteOrder.LITTLE_ENDIAN);
            
            long numEnts = buf.getLong (0);
            for (int i=0; i < numEnts; ++i) {
                int base = 8 + (HEADENT_SIZE * i);
                
                HeaderEntry newEnt = new HeaderEntry ();
                newEnt.mmsi   = buf.getInt (base);
                newEnt.offset = buf.getInt (base + 4);
                newEnt.length = buf.getInt (base + 4 + 4);

                Util.enforce (! _ents.containsKey (newEnt.mmsi));
                _ents.put (newEnt.mmsi, newEnt);

                _mmsis.push (newEnt.mmsi);
            }
        }

        // Fetch header entry for a given mmsi
        private HeaderEntry get (int mmsi) { return _ents.get (mmsi); }
        // Number of entries
        private int size  () { return _ents.size (); }
        // Copy out mmsis array
        private int[] dupMmsis () { return _mmsis.toArray (); }
    }
    private final class HeaderEntry {
        int mmsi, offset, length;
        private HeaderEntry () {};
    }


    //  ------------------------------------------------------------
    //  Ctr

    public AISMmFile (String filepath) {
        _file = Util.mmapFile_ro_le (filepath);
        Util.enforce (_file.capacity () >= 8, "Invalid file size");

        _header = new FileHeader (_file);
        _headerSizeBytes = 8 + (_header.size() * HEADENT_SIZE);
        _mmsis = _header.dupMmsis ();

        int bodySize = _file.capacity () - _headerSizeBytes;
        Util.enforce (bodySize % AISMSG_SIZE == 0, "Invalid file size");
    }
    
    
    //  ------------------------------------------------------------
    //  Access to one mmsi track's data

    public MmsiTrackRef mmsi (int mmsi) { return new MmsiTrackRef (mmsi); }

    public final class MmsiTrackRef implements MmsiTrackView, Iterable<AisMsg> {
        private final int _mmsi;
        private final HeaderEntry _he;
        private MmsiTrackRef (int mmsi) {
            _mmsi = mmsi;
            _he = _header.get (mmsi);
        }

        // Number of messages in track
        public int size () { return _he.length; }
        // NB allocates new AisMsg
        public AisMsg msg (int n) {
            return mmsiMsg (_mmsi, n); }
        // These don't allocate, but do check n on each call
        public double msgLat       (int n) {
            Util.enforce (n < size());
            return mmsiMsg_lat_unchecked       (_he, n); }
        public double msgLon       (int n) {
            Util.enforce (n < size());
            return mmsiMsg_lon_unchecked       (_he, n); }
        public int    msgMmsi      (int n) {
            Util.enforce (n < size ());
            return mmsiMsg_mmsi_unchecked      (_he, n); }
        public int    msgTimestamp (int n) {
            Util.enforce (n < size ());
            return mmsiMsg_timestamp_unchecked (_he, n); }
        public float  msgCourse    (int n) {
            Util.enforce (n < size ());
            return mmsiMsg_course_unchecked    (_he, n); }
        public float  msgSpeed     (int n) {
            Util.enforce (n < size ());
            return mmsiMsg_speed_unchecked     (_he, n); }

        //  --------------------------------------------------
        //  Convert to a SOA representation (copies all data out)

        public MmsiTrackSOA soa () {
            MmsiTrackSOA.Builder builder = new MmsiTrackSOA.Builder ();
            for (AisMsg msg : this)
                builder.push (msg);
            return builder.build ();
        }                
        
        //  --------------------------------------------------
        //  Iterating over an mmsi track's data (allocating version)
        //    Doesn't need to check position (n) on each call, so faster

        public IterSimple iterator () { return new IterSimple (_he); }
        
        private final class IterSimple implements Iterator<AisMsg> {
            private final HeaderEntry _he;
            private       int _nextPos = 0;
            private IterSimple (HeaderEntry he) { _he = he; }
            public boolean hasNext () { return _nextPos < _he.length; }
            public AisMsg next () {
                AisMsg res = mmsiMsg_unchecked (_he, _nextPos);
                ++_nextPos;
                return res;
            }
        }

        //  --------------------------------------------------
        //  Iterating over an mmsi track's data (self-mutating version)
        //    Doesn't allocate or check poses, so potentially faster

        public IterMut itermut () { return new IterMut (_he, 0); }

        public final class IterMut
            implements Iterable<MsgRef>, Iterator<MsgRef>
        {
            private final HeaderEntry _he;
            private int _nextOffset;
            private final MsgRef _ref;

            private IterMut (HeaderEntry he, int firstOffset) {
                Util.enforce (firstOffset < he.length);
                _he = he;
                _nextOffset = firstOffset;
                _ref = new MsgRef (_he, _nextOffset);
            }

            // -- Public interface
            
            // Get the iterator, or a new copy of the current one
            public IterMut iterator () {
                return new IterMut (_he, _nextOffset);
            }
            public boolean hasNext () {
                return _nextOffset < _he.length;
            }
            public MsgRef next () {
                _ref.setOffset (_nextOffset);
                ++_nextOffset;
                return _ref;
            }
        }
    }


    //  ------------------------------------------------------------
    //  Pointer to one stored message

    public final class MsgRef implements AisMsgView {
        private final HeaderEntry _he;
        private int _offset;
        
        private MsgRef (HeaderEntry he, int offset) {
            _he = he; _offset = offset; }
        
        // Unchecked, so make sure you get it right
        private void setOffset (int offset) { _offset = offset; }

        public double lat () {
            return mmsiMsg_lat_unchecked (_he, _offset); }
        public double lon () {
            return mmsiMsg_lon_unchecked (_he, _offset); }
        public int mmsi () {
            return mmsiMsg_mmsi_unchecked (_he, _offset); }
        public int timestamp () {
            return mmsiMsg_timestamp_unchecked (_he, _offset); }
        public float course () {
            return mmsiMsg_timestamp_unchecked (_he, _offset); }
        public float speed () {
            return mmsiMsg_timestamp_unchecked (_he, _offset); }
    }

    //  ------------------------------------------------------------
    //  Return the n'th message of a track as newly allocated data

    public AisMsg mmsiMsg (int mmsi, int n) {
        HeaderEntry he = _header.get (mmsi);
        Util.enforce (n < he.length);
        return mmsiMsg_unchecked (he, n);
    }
    // Unchecked, will read invalid data if n is too large for the MMSI
    // private AisMsg mmsiMsg_unchecked (int mmsi, int n) {
    //     AisMsg res = new AisMsg ();
    //     mmsiMsg_copyInto_unchecked (_header.get(mmsi), n, res);
    //     return res;
    // }
    private AisMsg mmsiMsg_unchecked (HeaderEntry he, int n) {
        AisMsg res = new AisMsg ();
        mmsiMsg_copyInto_unchecked (he, n, res);
        return res;
    }
    


    //  ------------------------------------------------------------
    //  Copy data from an mmsi's n'th message into a target

    // public void mmsiMsg_copyInto (int mmsi, int n, AisMsg res) {
    //     Util.enforce (n < _header.get (mmsi).length);
    //     mmsiMsg_copyInto_unchecked (mmsi, n, res);
    // }
    // Unchecked, will read invalid data if n is too large for the MMSI
//    private void mmsiMsg_copyInto_unchecked (int mmsi, int n, AisMsg res) {
    private void mmsiMsg_copyInto_unchecked (HeaderEntry he, int n, AisMsg res) {
        res.lat       = mmsiMsg_lat_unchecked       (he, n);
        res.lon       = mmsiMsg_lon_unchecked       (he, n);
        res.mmsi      = mmsiMsg_mmsi_unchecked      (he, n);
        res.timestamp = mmsiMsg_timestamp_unchecked (he, n);
        res.course    = mmsiMsg_course_unchecked    (he, n);
        res.speed     = mmsiMsg_speed_unchecked     (he, n);
    }

    
    //  ------------------------------------------------------------
    //  Get individual fields of an mmsi track's n'th message
    //    These don't allocate, so are faster than mmsiMsg()

    // Helper, enforce n inside track then return header
    private HeaderEntry getMmsiHEEnf (int mmsi, int n) {
        HeaderEntry he = _header.get (mmsi);
        Util.enforce (n < he.length);
        return he;
    }
    public double mmsiMsg_lat        (int mmsi, int n) {
        return mmsiMsg_lat_unchecked (getMmsiHEEnf (mmsi,n), n);
    }
    public double mmsiMsg_lon       (int mmsi, int n) {
        return mmsiMsg_lon_unchecked (getMmsiHEEnf (mmsi,n), n);
    }
    public int    mmsiMsg_mmsi      (int mmsi, int n) {
        return mmsiMsg_mmsi_unchecked (getMmsiHEEnf (mmsi,n), n);
    }
    public int    mmsiMsg_timestamp (int mmsi, int n) {
        return mmsiMsg_timestamp_unchecked (getMmsiHEEnf (mmsi,n), n);
    }
    public float  mmsiMsg_course    (int mmsi, int n) {
        return mmsiMsg_course_unchecked (getMmsiHEEnf (mmsi,n), n);
    }
    public float  mmsiMsg_speed     (int mmsi, int n) {
        return mmsiMsg_speed_unchecked (getMmsiHEEnf (mmsi,n), n);
    }

    // Unchecked, will read invalid data if n is too large for the MMSI
    private double mmsiMsg_lat_unchecked       (HeaderEntry he, int n) {
        return _file.getDouble (msgBytePos (he, n));
    }
    private double mmsiMsg_lon_unchecked       (HeaderEntry he, int n) {
        return _file.getDouble (msgBytePos (he, n) + 8);
    }
    private int    mmsiMsg_mmsi_unchecked      (HeaderEntry he, int n) {
        return _file.getInt    (msgBytePos (he, n) + 8 + 8);
    }
    private int    mmsiMsg_timestamp_unchecked (HeaderEntry he, int n) {
        return _file.getInt    (msgBytePos (he, n) + 8 + 8 + 4);
    }
    private float  mmsiMsg_course_unchecked    (HeaderEntry he, int n) {
        return _file.getFloat  (msgBytePos (he, n) + 8 + 8 + 4 + 4);
    }
    private float  mmsiMsg_speed_unchecked     (HeaderEntry he, int n) {
        return _file.getFloat  (msgBytePos (he, n) + 8 + 8 + 4 + 4 + 4);
    }
    

        
    //  ------------------------------------------------------------
    //  Where are the mmsi's messages' bytes located in the file?
    //  NB doesn't check that 'n' is valid for mmsi

    private int msgBytePos (HeaderEntry he, int n) {
        return   _headerSizeBytes
               + (AISMSG_SIZE * (n + he.offset));
    }
}
