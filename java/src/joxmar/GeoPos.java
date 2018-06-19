package joxmar;

public final class GeoPos {
    public double lat;
    public double lon;

    public GeoPos (double la, double lo) {
        lat = la;
        lon = lo;
    }

    @Override public String toString () {
        return String.format ("(GeoPos){ lat: %f, lon: %f }", lat, lon);
    }
}
