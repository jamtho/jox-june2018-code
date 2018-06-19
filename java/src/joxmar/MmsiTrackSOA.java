package joxmar;

// Immutable 'struct of arrays' class for holding MMSI track data.
//
// Use the static (mutable) Builder class to create one of these.
//
// TODO write a code generator for these SOA classes; there's nothing special
// about this one.

public final class MmsiTrackSOA implements MmsiTrackView {
    // All set (once) by builder
    private double[] _lats;
    private double[] _lons;
    private int[]    _mmsis;
    private int[]    _timestamps;
    private float[]  _courses;
    private float[]  _speeds;

    public double msgLat       (int n) { return _lats       [n]; }
    public double msgLon       (int n) { return _lons       [n]; }
    public int    msgMmsi      (int n) { return _mmsis      [n]; }
    public int    msgTimestamp (int n) { return _timestamps [n]; }
    public float  msgCourse    (int n) { return _courses    [n]; }
    public float  msgSpeed     (int n) { return _speeds     [n]; }

    // Can only construct via Builder
    private MmsiTrackSOA() {}

    // Simple accessors
    public int size () { return _lats.length; }  // number of messages
    public int firstTimestamp () { return _timestamps [0]; }
    public int lastTimestamp  () { return _timestamps [size() - 1]; }
    public boolean coversTimestamp (int ts) {
        return firstTimestamp() <= ts && ts <= lastTimestamp();
    }

    // Getting a single message's data
    public AisMsg msg (int n) {
        AisMsg res = new AisMsg ();
        copyMsgInto (n, res);
        return res;
    }
    public void copyMsgInto (int n, AisMsg msg) {
        msg.lat = msgLat (n);
        msg.lon = msgLon (n);
        msg.mmsi = msgMmsi (n);
        msg.timestamp = msgTimestamp (n);
        msg.course = msgCourse (n);
        msg.speed = msgSpeed (n);
    }


    //  --------------------------------------------------
    //  Builder class (handles the mutability)

    public final static class Builder {
        private final Vec.Double _lats       = new Vec.Double ();
        private final Vec.Double _lons       = new Vec.Double ();
        private final Vec.Int    _mmsis      = new Vec.Int ();
        private final Vec.Int    _timestamps = new Vec.Int ();
        private final Vec.Float  _courses    = new Vec.Float ();
        private final Vec.Float  _speeds     = new Vec.Float ();
        
        public Builder () {}

        public void push (final AisMsg msg) {
            // All messages must have the same mmsi
            if (_mmsis.size() > 0) {
                Util.enforce (msg.mmsi == _mmsis.get(0));
            }
            // Time ascending ordering is required
            if (_timestamps.size() > 0) {
                Util.enforce (   _timestamps.get (_timestamps.size() - 1)
                              <= msg.timestamp);
            }

            _lats.push       (msg.lat);
            _lons.push       (msg.lon);
            _mmsis.push      (msg.mmsi);
            _timestamps.push (msg.timestamp);
            _courses.push    (msg.course);
            _speeds.push     (msg.speed);
        }

        public MmsiTrackSOA build () {
            MmsiTrackSOA res = new MmsiTrackSOA ();
            res._lats       = _lats.toArray ();
            res._lons       = _lons.toArray ();
            res._mmsis      = _mmsis.toArray ();
            res._timestamps = _timestamps.toArray ();
            res._courses    = _courses.toArray ();
            res._speeds     = _speeds.toArray ();
            return res;
        }
    }
}
