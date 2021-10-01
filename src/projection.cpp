/*
 * LeanHRPT Decode
 * Copyright (C) 2021 Xerbo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "projection.h"

#include <cmath>
#include <fstream>
#include <algorithm>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace geo {
    // Convert from a internal angle of a circle to the viewing angle of a point above the circle.
    double earth2sat_angle(double radius, double height, double angle) {
        return -atan(sin(angle)*radius / (cos(angle)*radius - (radius+height)));
    }

    double sat2earth_angle(double radius, double height, double angle) {
        return asin((radius+height)/radius * sin(angle)) - angle;
    }

    // These functions (azimuth and reckon) are adapted from https://github.com/martinber/noaa-apt/blob/master/src/geo.rs
    double azimuth(LatLon a, LatLon b) {
        // https://en.wikipedia.org/w/index.php?title=Azimuth&oldid=750059816#Calculating_azimuth

        double delta_lon = b.second - a.second;

        return atan2(sin(delta_lon), cos(a.first) * tan(b.first) - sin(a.first) * cos(delta_lon));
    }
    LatLon reckon(double lat, double lon, double range, double azimuth) {
        // Based on reckon from Alexander Barth
        // https://sourceforge.net/p/octave/mapping/ci/3f19801d4b93d3b3923df9fa62d268660e5cb4fa/tree/inst/reckon.m
        // relicenced to LGPL-v3
        double tmp = sin(lat) * cos(range) + cos(lat) * sin(range) * cos(azimuth);

        // Clamp tmp to [-1, 1]
        tmp = std::min(std::max(tmp, -1.0), 1.0);

        double lato = M_PI / 2.0 - acos(tmp);

        double cos_y = (cos(range) - sin(lato) * sin(lat)) / (cos(lato) * cos(lat));
        double sin_y = sin(azimuth) * sin(range) / cos(lato);

        double y = atan2(sin_y, cos_y);

        double lono = lon + y;

        // Bring the lono into the interval [-pi, pi]
        lono = fmod(lono + M_PI, 2.0 * M_PI) - M_PI;

        return {lato, lono};
    }
}

struct ProjectionInfo {
    double angle; // deg;
    double fov; // +/- deg
    double xoffset; // deg
    double time_offset; // seconds
};

const std::map<SatID, ProjectionInfo> projection_factors = {
    { NOAA15, { -92.6, 55.31, 0.1, -2.0/6.0 }}, // Untested
    { NOAA18, { -92.6, 55.31, 0.1, -2.0/6.0 }}, // Untested
    { NOAA19, { -92.6, 55.31, 0.1, -2.0/6.0 }}, // Needs further tuning
    { MetOpA, { -90.0, 55.31, 0.1, 1.0/6.0 }}, // Untested
    { MetOpB, { -90.0, 55.31, 0.1, 1.0/6.0 }}, // Good
    { MetOpC, { -90.0, 55.31, 0.1, 1.0/6.0 }}, // Untested
};

void Projector::save_gcp_file(std::vector<double> &timestamps, size_t pointsy, size_t pointsx, Imager sensor, SatID sat, std::string filename) {
    if (timestamps.size() == 0) {
        return;
    }

    std::filebuf file;
    file.open(filename, std::ios::out);
    if (!file.is_open()) {
        return;
    }
    std::ostream stream(&file);

    d_sensor = sensor_info.at(sensor);
    ProjectionInfo info;
    try {
        info = projection_factors.at(sat);
    } catch(const std::exception& e) {
        return;
    }

    for (size_t i = 0; i < pointsy; i++) {
        double y = ((double)i/(double)(pointsy)) * (double)timestamps.size();

        if (timestamps[(int)y] == 0) continue;

        double timestamp = timestamps[(int)y] + info.time_offset;
        struct predict_position orbit = predictor.predict(timestamp);

        double az;
        {
            struct predict_position a = predictor.predict(timestamp-0.1);
            struct predict_position b = predictor.predict(timestamp+0.1);
            az = geo::azimuth({a.latitude, a.longitude}, {b.latitude, b.longitude}) + info.angle*M_PI/180.0;
        }

        auto scan = calculate_scan({orbit.latitude, orbit.longitude}, az, orbit.altitude, info.fov, info.xoffset, pointsx);
        for (auto &point : scan) {
            stream << "-gcp " << point.first << " " << y << " " << (point.second.second*180.0/M_PI) << " " << (point.second.first*180.0/M_PI) << " ";
        }
    }

    stream.flush();
    file.close();
}

std::vector<std::pair<double, geo::LatLon>> Projector::calculate_scan(geo::LatLon position, double az, double altitude, double fov, double xoffset, size_t points) {
    std::vector<std::pair<double, geo::LatLon>> scan;
    double view_angle = geo::sat2earth_angle(EARTH_RADIUS, altitude, fov*M_PI/180.0)*2.0;

    for (size_t i = 0; i < points; i++) {
        // The angle that the sensor is pointing at
        double sensor_angle = ((double)i/(double)(points-1) - 0.5) * view_angle;
        // The coordinate that the sensor is pointing at
        auto pos = geo::reckon(position.first, position.second, sensor_angle, az);

        // Convert scan angle to angle on the earth
        double sat_edge = geo::earth2sat_angle(EARTH_RADIUS, altitude, view_angle/2);
        double angle = geo::earth2sat_angle(EARTH_RADIUS, altitude, sensor_angle) + xoffset*M_PI/180.0;

        // Create a list of points and their respective coordinates
        scan.push_back({(angle/sat_edge + 1.0)/2.0 * (double)d_sensor.width, pos});
    }

    return scan;
}
