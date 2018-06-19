#pragma once

// Geospatial functions


//  --------------------------------------------------------------------------
//  Geospatial distance


// -- Local helpers

// Calcs arc lengh in metres by haversine formula. Provide values in radians.
inline static double
GeoUtils__arclength_haver (double lat1, double lon1,
                            double lat2, double lon2) {
    double latArc = lat1 - lat2;
    double lonArc = lon1 - lon2;

    double latH = sin (latArc * 0.5);
    latH *= latH;

    double lonH = sin (lonArc * 0.5);
    lonH *= lonH;

    double tmp = cos (lat1) * cos (lat2);
    return 2.0 * asin (sqrt (latH + tmp * lonH));
}

// Converts degrees to radians
inline static double
GeoUtils__degToRad (double degs) {
    return degs * (M_PI / 180.0);
}

// Radius of earth in metres
inline static double
GeoUtils__earthRadius_metres () {
    return 6367444.7;
}


// -- Public API

// Provide degrees
inline static double
GeoUtils_distMetres (double lat1, double lon1,
                     double lat2, double lon2) {
    return   GeoUtils__earthRadius_metres ()
           * GeoUtils__arclength_haver (GeoUtils__degToRad (lat1),
                                        GeoUtils__degToRad (lon1),
                                        GeoUtils__degToRad (lat2),
                                        GeoUtils__degToRad (lon2));
}
