package joxmar;

public interface MmsiTrackView {

    //  --------------------------------------------------
    //  Accessors

    // How many messages in the track
    public int size ();

    // Get a copy of the n'th message
    public AisMsg msg (int n);

    // Get the fields of the n'th message
    public double msgLat       (int n);
    public double msgLon       (int n);
    public int    msgMmsi      (int n);
    public int    msgTimestamp (int n);
    public float  msgCourse    (int n);
    public float  msgSpeed     (int n);


    //  --------------------------------------------------
    //  Mixin utils

    public default int firstTimestamp () {
        Util.enforce (size () > 0);
        return msgTimestamp (0);
    }
    public default int lastTimestamp () {
        Util.enforce (size () > 0);
        return msgTimestamp (size () - 1);
    }

    // Does the track's time period cover the given timestamp?
    public default boolean coversTimestamp (int ts) {
        if (size () == 0)
            return false;
        return firstTimestamp() <= ts && ts <= lastTimestamp();
    }
}
    
