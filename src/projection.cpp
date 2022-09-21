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

#include <QLocale>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>

#include "config/config.h"
#include "geo/geolocation.h"
#include "util.h"

static double str2double(std::string str) {
    QLocale l(QLocale::C);
    return l.toDouble(QString::fromStdString(str));
}

std::vector<std::pair<xy, Geodetic>> Projector::calculate_gcps(const std::vector<double> &timestamps, size_t pointsy,
                                                               size_t pointsx, Imager sensor, SatID sat, size_t width) {
    std::vector<std::pair<xy, Geodetic>> gcps;

    if (timestamps.size() == 0) {
        return gcps;
    }

    Config proj_info("projection.ini");
    if (!proj_info.sections.count(satellite_info.at(sat).name + "_" + sensor_info.at(sensor).name)) {
        return gcps;
    }
    auto params = proj_info.sections.at(satellite_info.at(sat).name + "_" + sensor_info.at(sensor).name);
    double fov = str2double(params["fov"]);
    double yaw = str2double(params["yaw"]);
    double roll = str2double(params["roll"]);
    double pitch = str2double(params["pitch"]);
    double pitchscale = str2double(params["pitchscale"]);
    double toffset = str2double(params["toffset"]);
    bool curved = params["curved"] == "true";

    for (size_t j = 0; j < pointsy; j++) {
        size_t i = (double)j / double(pointsy - 1) * double(timestamps.size() - 1);
        double timestamp = timestamps[i] + toffset;

        struct predict_position orbit = predictor.predict(timestamp);

        double azimuth;
        {
            struct predict_position a = predictor.predict(timestamp - 0.05);
            struct predict_position b = predictor.predict(timestamp + 0.05);
            azimuth = deg2rad(90) - CalculateGeodeticCurve(WGS84, Geodetic(a), Geodetic(b)).Azimuth;
            if (azimuth < -M_PI) {
                azimuth += deg2rad(yaw);
            } else {
                azimuth -= deg2rad(yaw);
            }
        }

        auto scan = calculate_scan(Geodetic(orbit), azimuth, fov, roll, pitch, pitchscale, curved, pointsx);
        for (auto &point : scan) {
            gcps.push_back(std::make_pair(std::make_pair(point.first * double(width - 1), (double)i), point.second));
        }
    }

    return gcps;
}

void Projector::save_gcp_file(const std::vector<double> &timestamps, size_t pointsy, size_t pointsx, Imager sensor, SatID sat,
                              std::string filename, size_t width) {
    auto gcps = calculate_gcps(timestamps, pointsy, pointsx, sensor, sat, width);

    std::filebuf file;
    if (!file.open(filename, std::ios::out)) return;
    std::ostream stream(&file);

    for (size_t i = 0; i < gcps.size(); i++) {
        double x = gcps[i].first.first;
        double y = gcps[i].first.second;
        double lat = (gcps[i].second.latitude * RAD2DEG);
        double lon = (gcps[i].second.longitude * RAD2DEG);
        stream << x << "," << y << "," << lat << "," << lon << "\n";
    }
    file.close();
}

std::vector<float> Projector::calculate_sunz(const std::vector<double> &timestamps, Imager sensor, SatID sat, size_t width) {
    const size_t pointsx = 21;
    const size_t pointsy = (double)timestamps.size() / (double)width * 21.0;
    auto gcps = calculate_gcps(timestamps, pointsy, pointsx, sensor, sat, width);
    if (gcps.size() == 0) return {};

    std::vector<float> sunz(pointsy * pointsx);
    for (size_t i = 0; i < gcps.size(); i++) {
        double y = gcps[i].first.second;
        double lon = gcps[i].second.longitude;
        double lat = gcps[i].second.latitude;
        double timestamp = timestamps[y];

        // UNIX to Julian date
        predict_julian_date_t prediction_time = (timestamp / 86400.0) - 3651.0;
        predict_observer_t observer = {"", lat, lon, 0};

        struct predict_observation observation;
        predict_observe_sun(&observer, prediction_time, &observation);

        // Elevation to zenith
        sunz[i] = deg2rad(90.0) - observation.elevation;
    }

    // Interpolate points to the full image size
    size_t height = timestamps.size();
    std::vector<float> full_sunz(height * width);
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            double y2 = (double)y / double(height - 1) * (pointsy - 1);
            double x2 = (double)x / double(width - 1) * (pointsx - 1);

            float a = lerp<float>(sunz[floor(y2) * pointsx + floor(x2)], sunz[floor(y2) * pointsx + ceil(x2)], fmod(x2, 1.0));
            float b = lerp<float>(sunz[ceil(y2) * pointsx + floor(x2)], sunz[ceil(y2) * pointsx + ceil(x2)], fmod(x2, 1.0));
            full_sunz[y * width + x] = lerp<float>(a, b, fmod(y2, 1.0));
        }
    }

    return full_sunz;
}

std::vector<std::pair<double, Geodetic>> Projector::calculate_scan(const Geodetic &position, double azimuth, double fov,
                                                                   double roll, double pitch, double pitchscale, bool curved,
                                                                   size_t n) {
    std::vector<std::pair<double, Geodetic>> scan;

    for (size_t i = 0; i < n; i++) {
        // The angle that the sensor is pointing at
        double sensor_angle = (double)i / double(n - 1) * 2.0 - 1.0;
        double _pitch = pitch;
        if (curved) {
            _pitch *= sqrt(1.0 - pow(sensor_angle * pitchscale, 2.0));
        }
        sensor_angle *= fov * DEG2RAD;

        // The position that the sensor is looking at
        Geodetic point = los_to_earth(position, sensor_angle + deg2rad(roll), deg2rad(_pitch), azimuth);

        // Create a list of points and their respective coordinates
        scan.push_back({(double)i / double(n - 1), point});
    }

    return scan;
}

bool Projector::is_northbound(const std::vector<double> &timestamps) {
    double lower_quartile = timestamps[timestamps.size() / 4 * 1];
    double upper_quartile = timestamps[timestamps.size() / 4 * 3];

    struct predict_position a = predictor.predict(lower_quartile);
    struct predict_position b = predictor.predict(upper_quartile);
    double azimuth = CalculateGeodeticCurve(WGS84, Geodetic(a), Geodetic(b)).Azimuth;

    return azimuth > M_PI * 1.5;
}
