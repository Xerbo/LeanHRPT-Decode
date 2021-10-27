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
#include "config.h"

#include <cmath>
#include <fstream>
#include <algorithm>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)

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

static double str2double(std::string str) {
    QLocale l(QLocale::C);
    return l.toDouble(QString::fromStdString(str));
}

void Projector::save_gcp_file(std::vector<double> &timestamps, size_t pointsy, size_t pointsx, Imager sensor, SatID sat, std::string filename) {
    if (timestamps.size() == 0) {
        return;
    }

    std::filebuf file;
    if (!file.open(filename, std::ios::out)) {
        return;
    }
    std::ostream stream(&file);
    stream << "<GCPList Projection=\"EPSG:4326\">\n";

    Config proj_info("projection.ini");
    auto params = proj_info.sections.at(satellite_info.at(sat).name + "_" + sensor_info.at(sensor).name);
    double fov = str2double(params["fov"]);
    double yaw = str2double(params["yaw"]);
    double roll = str2double(params["roll"]);
    double toffset = str2double(params["toffset"]);

    d_sensor = sensor_info.at(sensor);

    size_t n = 0;
    size_t need = 0;
    for (size_t i = 0; i < timestamps.size(); i++) {
        bool line_ok = false;

        need++;
        if (need > timestamps.size()/pointsy && timestamps[i] != 0.0) {
            line_ok = true;
            need = 0;
        }

        if (line_ok) {
            double timestamp = timestamps[i] + toffset;
            struct predict_position orbit = predictor.predict(timestamp);

            double az;
            {
                struct predict_position a = predictor.predict(timestamp-0.1);
                struct predict_position b = predictor.predict(timestamp+0.1);
                az = geo::azimuth({a.latitude, a.longitude}, {b.latitude, b.longitude});
                if (az < -M_PI/2 || az > M_PI/2) {
                    az = az + (-90 - yaw)*DEG2RAD;
                } else {
                    az = az + (-90 + yaw)*DEG2RAD;
                }
            }

            auto scan = calculate_scan({orbit.latitude, orbit.longitude}, az, orbit.altitude, fov, roll, pointsx);
            for (auto &point : scan) {
                stream << "<GCP Id=\"" << n << "\" Pixel=\"" << point.first << "\" Line=\"" << i << "\" X=\"" << (point.second.second*RAD2DEG) << "\" Y=\"" << (point.second.first*180.0/M_PI) << "\" />\n";
                n++;
            }
        }
    }

    stream << "</GCPList>\n";
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
