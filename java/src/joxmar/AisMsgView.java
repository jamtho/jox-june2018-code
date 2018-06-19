package joxmar;

public interface AisMsgView {
    public double lat ();
    public double lon ();
    public int    mmsi ();
    public int    timestamp ();
    public float  course ();
    public float  speed ();


    //  --------------------------------------------------
    //  Mixin utils etc

    public default String csv () {
        return String.format ("%f,%f,%d,%d,%f,%f",
                              lat(), lon(), mmsi(), timestamp(),
                              course(), speed());
    }
    public static String csvHeader () {
        return "lat,lon,mmsi,timestamp,course,speed";
    }

    public default String str () {
        return String.format ("(AisMsgView){ lat: %f, lon: %f, mmsi: %d, "
                              + "timestamp: %d, course: %f, speed: %f }",
                              lat(), lon(), mmsi(), timestamp(),
                              course(), speed());
    }

    // Copy all data from this into 'other'
    public default void copyInto (AisMsg other) {
        other.lat = lat ();
        other.lon = lon ();
        other.mmsi = mmsi ();
        other.timestamp = timestamp ();
        other.course = course ();
        other.speed = speed ();
    }
    

    //  --------------------------------------------------
    //  Wrap an AisMsg as an AisMsgView (for mixins etc)
    
    static final class AisMsgWrapper implements AisMsgView {
        private final AisMsg _msg;
        AisMsgWrapper (AisMsg msg) { _msg = msg; }
        public double lat       () { return _msg.lat; }
        public double lon       () { return _msg.lon; }
        public int    mmsi      () { return _msg.mmsi; }
        public int    timestamp () { return _msg.timestamp; }
        public float  course    () { return _msg.course; }
        public float  speed     () { return _msg.speed; }
    }
    public static AisMsgView of (AisMsg msg) {
        return new AisMsgWrapper (msg);
    }
}
