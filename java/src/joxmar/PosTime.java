package joxmar;

// Representation of 'position at a time (with an id)' data


// Binary format we're echoing
// ---------------------------
// 
// typedef struct PosTime {
//     double lat;
//     double lon;
//     int32_t timestamp;
//     int32_t id;
// } PosTime;


public final class PosTime {
    public double lat;
    public double lon;
    public int timestamp;
    public int id;

    @Override public String toString () {
        return String.format ("(PosTime){ lat: %f, lon: %f, timestamp: %d, id: %d",
                              lat, lon, timestamp, id);
    }
}
