/*
 * Modified from https://github.com/Digitelektro/MeteorDemod
 * Its license is as follows:
 *
 * MIT License
 *
 * Copyright (c) 2020 Digitelektro
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "geolocation.h"

#include <limits>
#include <stdexcept>

#include "util.h"

Vector locationToVector(const Geodetic &location) {
    double cosLat = cos(location.latitude);
    double sinLat = sin(location.latitude);
    double cosLon = cos(location.longitude);
    double sinLon = sin(location.longitude);

    double radA = 6378.137;
    double f = 1.0 / 298.257223563;  // Flattening factor WGS84 Model
    double radB = radA * (1 - f);

    double N = pow(radA, 2) / sqrt(pow(radA, 2) * pow(cosLat, 2) + pow(radB, 2) * pow(sinLat, 2));

    double x = (N + location.altitude) * cosLat * cosLon;
    double y = (N + location.altitude) * cosLat * sinLon;
    double z = ((pow(radB, 2) / pow(radA, 2)) * N + location.altitude) * sinLat;

    return Vector(x, y, z);
}

Geodetic vectorToLocation(const Vector &vector) {
    double a = 6378.137;
    double f = 1.0 / 298.257223563;  // Flattening factor WGS84 Model
    double b = a * (1 - f);

    double r = sqrt(pow(vector.x, 2) + pow(vector.y, 2) + pow(vector.z, 2));

    double lat = asin(vector.z / r);
    double lon = atan2(vector.y, vector.x);

    double e = sqrt((pow(a, 2) - pow(b, 2)) / pow(a, 2));
    double e2 = sqrt((pow(a, 2) - pow(b, 2)) / pow(b, 2));
    double p = sqrt(pow(vector.x, 2) + pow(vector.y, 2));
    double phi = atan2(vector.z * a, p * b);
    lat = atan2(vector.z + pow(e2, 2) * b * pow(sin(phi), 3), p - pow(e, 2) * a * pow(cos(phi), 3));

    return Geodetic(lat, lon, 0);
}

Geodetic los_to_earth(const Geodetic &position, double roll, double pitch, double yaw) {
    Vector vector = locationToVector(position);
    return los_to_earth(vector, roll, pitch, yaw);
}

Geodetic los_to_earth(const Vector &position, double roll, double pitch, double yaw) {
    double a = 6371.0087714;
    double b = 6371.0087714;
    double c = 6356.752314245;

    double x = position.x;
    double y = position.y;
    double z = position.z;

    Matrix4x4 matrix(1, 0, 0, position.x, 0, 1, 0, position.y, 0, 0, 1, position.z, 0, 0, 0, 1);

    Vector lookVector(0, 0, 0);
    Matrix4x4 lookMatrix = lookAt(position, lookVector, Vector(0, 0, 1));
    Matrix4x4 rotateX = Matrix4x4::CreateRotationX(roll);
    Matrix4x4 rotateY = Matrix4x4::CreateRotationY(pitch);
    Matrix4x4 rotateZ = Matrix4x4::CreateRotationZ(yaw);
    matrix = matrix * lookMatrix * rotateZ * rotateY * rotateX;

    Vector vector3(matrix.mElements[2], matrix.mElements[6], matrix.mElements[10]);

    double u = vector3.x;
    double v = vector3.y;
    double w = vector3.z;

    double value = -pow(a, 2) * pow(b, 2) * w * z - pow(a, 2) * pow(c, 2) * v * y - pow(b, 2) * pow(c, 2) * u * x;
    double radical = pow(a, 2) * pow(b, 2) * pow(w, 2) + pow(a, 2) * pow(c, 2) * pow(v, 2) - pow(a, 2) * pow(v, 2) * pow(z, 2) +
                     2 * pow(a, 2) * v * w * y * z - pow(a, 2) * pow(w, 2) * pow(y, 2) + pow(b, 2) * pow(c, 2) * pow(u, 2) -
                     pow(b, 2) * pow(u, 2) * pow(z, 2) + 2 * pow(b, 2) * u * w * x * z - pow(b, 2) * pow(w, 2) * pow(x, 2) -
                     pow(c, 2) * pow(u, 2) * pow(y, 2) + 2 * pow(c, 2) * u * v * x * y - pow(c, 2) * pow(v, 2) * pow(x, 2);
    double magnitude = pow(a, 2) * pow(b, 2) * pow(w, 2) + pow(a, 2) * pow(c, 2) * pow(v, 2) + pow(b, 2) * pow(c, 2) * pow(u, 2);

    /*double t = -(1 / (pow(c, 2) * (pow(u, 2) + pow(v, 2)) + pow(a, 2) * pow(w, 2))) *
       (pow(c, 2) * (u * x + v * y) + pow(a, 2) * w * z +
       0.5 * sqrt(4 * pow((pow(a, 2) * (u * x + v * y) + pow(a, 2) * w * z), 2) - 4 * (pow(a, 2) * (pow(u, 2) + pow(v, 2)) +
       pow(a, 2) * pow(w, 2)) * (pow(c, 2) * (-pow(a, 2) + pow(x, 2) + pow(y, 2)) + pow(a, 2) * pow(z, 2))));
    */

    if (radical < 0) {
        return Geodetic(0, 0, 0);
    }

    double d = (value - a * b * c * sqrt(radical)) / magnitude;

    if (d < 0) {
        return Geodetic(0, 0, 0);
    }

    x += d * u;
    y += d * v;
    z += d * w;

    return vectorToLocation(Vector(x, y, z));
}

GeodeticCurve CalculateGeodeticCurve(Ellipsoid ellipsoid, Geodetic start, Geodetic end, double tolerance) {
    //
    // All equation numbers refer back to Vincenty's publication:
    // See http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
    //

    // get constants
    double a = ellipsoid.SemiMajorAxisMeters;
    double b = ellipsoid.SemiMinorAxisMeters;
    double f = ellipsoid.Flattening;

    // get parameters as radians
    double phi1 = start.latitude;
    double lambda1 = start.longitude;
    double phi2 = end.latitude;
    double lambda2 = end.longitude;

    // calculations
    double a2 = a * a;
    double b2 = b * b;
    double a2b2b2 = (a2 - b2) / b2;

    double omega = lambda2 - lambda1;

    double tanphi1 = tan(phi1);
    double tanU1 = (1.0 - f) * tanphi1;
    double U1 = atan(tanU1);
    double sinU1 = sin(U1);
    double cosU1 = cos(U1);

    double tanphi2 = tan(phi2);
    double tanU2 = (1.0 - f) * tanphi2;
    double U2 = atan(tanU2);
    double sinU2 = sin(U2);
    double cosU2 = cos(U2);

    double sinU1sinU2 = sinU1 * sinU2;
    double cosU1sinU2 = cosU1 * sinU2;
    double sinU1cosU2 = sinU1 * cosU2;
    double cosU1cosU2 = cosU1 * cosU2;

    // eq. 13
    double lambda = omega;

    // intermediates we'll need to compute 's'
    double A = 0.0;
    double B = 0.0;
    double sigma = 0.0;
    double deltasigma = 0.0;
    double lambda0;
    bool converged = false;

    for (int i = 0; i < 20; i++) {
        lambda0 = lambda;

        double sinlambda = sin(lambda);
        double coslambda = cos(lambda);

        // eq. 14
        double cosU1sinU2_sinU2cosU2coslambda = cosU1sinU2 - sinU1cosU2 * coslambda;
        double sin2sigma =
            (cosU2 * sinlambda * cosU2 * sinlambda) + (cosU1sinU2_sinU2cosU2coslambda * cosU1sinU2_sinU2cosU2coslambda);
        double sinsigma = sqrt(sin2sigma);

        // eq. 15
        double cossigma = sinU1sinU2 + (cosU1cosU2 * coslambda);

        // eq. 16
        sigma = atan2(sinsigma, cossigma);

        // eq. 17    Careful!  sin2sigma might be almost 0!
        double sinalpha = (sin2sigma == 0) ? 0.0 : cosU1cosU2 * sinlambda / sinsigma;
        double alpha = asin(sinalpha);
        double cosalpha = cos(alpha);
        double cos2alpha = cosalpha * cosalpha;

        // eq. 18    Careful!  cos2alpha might be almost 0!
        double cos2sigmam = cos2alpha == 0.0 ? 0.0 : cossigma - 2 * sinU1sinU2 / cos2alpha;
        double u2 = cos2alpha * a2b2b2;

        double cos2sigmam2 = cos2sigmam * cos2sigmam;

        // eq. 3
        A = 1.0 + u2 / 16384 * (4096 + u2 * (-768 + u2 * (320 - 175 * u2)));

        // eq. 4
        B = u2 / 1024 * (256 + u2 * (-128 + u2 * (74 - 47 * u2)));

        // eq. 6
        deltasigma =
            B * sinsigma *
            (cos2sigmam +
             B / 4 * (cossigma * (-1 + 2 * cos2sigmam2) - B / 6 * cos2sigmam * (-3 + 4 * sin2sigma) * (-3 + 4 * cos2sigmam2)));

        // eq. 10
        double C = f / 16 * cos2alpha * (4 + f * (4 - 3 * cos2alpha));

        // eq. 11 (modified)
        lambda = omega + (1 - C) * f * sinalpha * (sigma + C * sinsigma * (cos2sigmam + C * cossigma * (-1 + 2 * cos2sigmam2)));

        if (i < 2) {
            continue;
        }

        // see how much improvement we got
        double change = abs((lambda - lambda0) / lambda);

        if (change < tolerance) {
            converged = true;
            break;
        }
    }

    // eq. 19
    double s = b * A * (sigma - deltasigma);
    double alpha1;
    double alpha2;

    // didn't converge?  must be N/S
    if (!converged) {
        if (phi1 > phi2) {
            alpha1 = 180 * DEG2RAD;
            alpha2 = 0;
        } else if (phi1 < phi2) {
            alpha1 = 0;
            alpha2 = 180 * DEG2RAD;
        } else {
            alpha1 = NAN;
            alpha2 = NAN;
            throw std::out_of_range("Well fuck");
        }
    } else {
        double radians;

        // eq. 20
        radians = atan2(cosU2 * sin(lambda), (cosU1sinU2 - sinU1cosU2 * cos(lambda)));
        if (radians < 0.0) radians += M_2PI;
        alpha1 = radians;

        // eq. 21
        radians = atan2(cosU1 * sin(lambda), (-sinU1cosU2 + cosU1sinU2 * cos(lambda))) + M_PI;
        if (radians < 0.0) radians += M_2PI;
        alpha2 = radians;
    }

    if (alpha1 >= M_2PI) alpha1 -= M_2PI;
    if (alpha2 >= M_2PI) alpha2 -= M_2PI;

    return GeodeticCurve(s, alpha1, alpha2);
}

Matrix4x4 lookAt(const Vector3 &position, const Vector3 &target, const Vector3 &up) {
    Vector3 k = Vector3(target) - position;
    double m = k.DistanceSquared();
    if (m < std::numeric_limits<double>::epsilon()) {
        return Matrix4x4();
    }
    k = k * (1.0 / sqrt(m));

    Vector3 i = up.Cross(k);
    i.Normalize();

    Vector3 j = k.Cross(i);
    j.Normalize();

    return Matrix4x4(i.x, j.x, k.x, 0.0, i.y, j.y, k.y, 0.0, i.z, j.z, k.z, 0.0, 0.0, 0.0, 0.0, 1.0);
}
