package joxmar;

// View over a file containing exactly a binary array of PosTime structs

// Binary format we're echoing
// ---------------------------
// 
// typedef struct PosTime {
//     double lat;
//     double lon;
//     int32_t timestamp;
//     int32_t id;
// } PosTime;

import java.nio.MappedByteBuffer;
import java.util.Iterator;
    

public class PosTimeArrayFile implements Iterable<PosTime> {
    private static final int STRUCT_SIZE = 24;
    private MappedByteBuffer _file;


    //  ------------------------------------------------------------
    //  Ctr

    public PosTimeArrayFile (String filepath) {
        _file = Util.mmapFile_ro_le (filepath);
        Util.enforce (_file.capacity() % STRUCT_SIZE == 0,
                      "File has invalid size");
    }


    //  ------------------------------------------------------------
    //  Simple accessors

    public int size () { return _file.capacity() / STRUCT_SIZE; }

    // Get allocating iterator
    public IterSimple iterator () { return new IterSimple (); }
    
    // Get reference-mutating iterator
    public IterMut itermut () { return new IterMut (0); }


    //  ------------------------------------------------------------
    //  Getting at whole messages

    // Return the n'th message as a freshly-allocated PosTime
    // Allow the buf to do the lenght check for us, as faster
    public PosTime nth (int n) {
        PosTime res = new PosTime ();
        copyNthInto (n, res);
        return res;
    }

    // Copy the data from the n'th message into the target PosTime
    private void copyNthInto (int n, PosTime res) {
        res.lat       = nth_lat (n);
        res.lon       = nth_lon (n);
        res.timestamp = nth_timestamp (n);
        res.id        = nth_id (n);
    }


    //  ------------------------------------------------------------
    //  Getting fragments of message data
    //    We let the buffer do the length checking, for speed

    double nth_lat       (int n) {
        return _file.getDouble (elemBytePos (n)); }
    double nth_lon       (int n) {
        return _file.getDouble (elemBytePos (n) + 8); }
    int    nth_timestamp (int n) {
        return _file.getInt    (elemBytePos (n) + 8 + 8); }
    int    nth_id        (int n) {
        return _file.getInt    (elemBytePos (n) + 8 + 8 + 4); }


    //  ------------------------------------------------------------
    //  Iterator classes

    // Allocates on each next ()
    private final class IterSimple implements Iterator<PosTime> {
        private int _nextPos = 0;
        public boolean hasNext () { return _nextPos < size (); }
        public PosTime next () {
            PosTime res = nth (_nextPos);
            ++_nextPos;
            return res;
        }
    }

    // Mutates reference on each next ()
    public final class IterMut implements Iterator<ElemRef>, Iterable<ElemRef> {
        private int _nextPos;
        private final ElemRef _ref = new ElemRef (-1);
        private IterMut (int nextPos) { _nextPos = nextPos; }

        public IterMut iterator () { return new IterMut (_nextPos); }

        public boolean hasNext () { return _nextPos < size (); }
        public ElemRef next () {
            _ref.setElem (_nextPos);
            ++_nextPos;
            return _ref;
        }
    }
    public final class ElemRef implements PosTimeView {
        private int _n;
        private ElemRef (int n) { _n = n; }
        private void setElem (int n) { _n = n; }
        
        public double lat ()       { return nth_lat (_n); }
        public double lon ()       { return nth_lon (_n); }
        public int    timestamp () { return nth_timestamp (_n); }
        public int    id ()        { return nth_id (_n); }
    }


    //  ------------------------------------------------------------
    //  Where are the binary structs stored?

    private int elemBytePos (int n) { return n * STRUCT_SIZE; }
}
        
