package joxmar;


public final class GeoUtils {

    //  ------------------------------------------------------------
    //  Public api
    
    // Returns dist between two points in metres.
    // Provide positions in degrees.
    public static double distMetres (double lat1, double lon1,
                                     double lat2, double lon2) {
        return   EARTH_RADIUS_METRES
               * arcLengthHaverRads (degToRad (lat1), degToRad (lon1),
                                     degToRad (lat2), degToRad (lon2));
    }


    //  ------------------------------------------------------------
    //  Private utils
    
    // Calcs arc lengh in metres by haversine formula.
    // Provide values in radians.
    private static double arcLengthHaverRads (double lat1, double lon1,
                                              double lat2, double lon2) {
        double latArc = lat1 - lat2;
        double lonArc = lon1 - lon2;

        double latH = Math.sin (latArc * 0.5);
        latH *= latH;

        double lonH = Math.sin (lonArc * 0.5);
        lonH *= lonH;

        double tmp = Math.cos (lat1) * Math.cos (lat2);
        return 2.0 * Math.asin (Math.sqrt (latH + tmp * lonH));
    }

    // Converts degrees to radians
    private static double degToRad (double degs) {
        return degs * (Math.PI / 180.0);
    }

    private static final double EARTH_RADIUS_METRES = 6367444.7;
}
