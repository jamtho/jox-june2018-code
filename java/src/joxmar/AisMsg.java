package joxmar;

public final class AisMsg {
    public double lat;
    public double lon;
    public int    mmsi;
    public int    timestamp;
    public float  course;
    public float  speed;

    // Copy all data from this into 'other'
    public void copyInto (AisMsg other) {
        other.lat = lat;
        other.lon = lon;
        other.mmsi = mmsi;
        other.timestamp = timestamp;
        other.course = course;
        other.speed = speed;
    }
}
    
