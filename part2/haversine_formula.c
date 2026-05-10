#include <math.h>
#include <stdint.h>

double square(double a) { return (a * a); }

double radians_from_degrees(double degrees) {
  return 0.01745329251994329577 * degrees;
}

double reference_haversine(double lon1, double lat1, double lon2, double lat2,
                           double earth_radius) {
  double dLat = radians_from_degrees(lat2 - lat1);
  double dLon = radians_from_degrees(lon2 - lon1);
  lat1 = radians_from_degrees(lat1);
  lat2 = radians_from_degrees(lat2);

  double a =
      square(sin(dLat / 2.0)) + cos(lat1) * cos(lat2) * square(sin(dLon / 2));
  double c = 2.0 * asin(sqrt(a));

  double Result = earth_radius * c;

  return Result;
}
