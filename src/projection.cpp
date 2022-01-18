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
#include "math.h"
#include "geo/geolocation.h"

#include <cmath>
#include <fstream>
#include <algorithm>
#include <map>

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
    double pitch = str2double(params["pitch"]);
    double pitchscale = str2double(params["pitchscale"]);
    double toffset = str2double(params["toffset"]);
    bool curved = params["curved"] == "true";

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

            double azimuth;
            {
                struct predict_position a = predictor.predict(timestamp);
                struct predict_position b = predictor.predict(timestamp+0.1);
                azimuth = deg2rad(90) - calculateBearingAngle(Geodetic(a), Geodetic(b));
                if (azimuth > M_PI) {
                    azimuth += deg2rad(yaw);
                } else {
                    azimuth -= deg2rad(yaw);
                }
            }

            auto scan = calculate_scan(Geodetic(orbit), azimuth, fov, roll, pitch, pitchscale, curved, pointsx);
            for (auto &point : scan) {
                stream << "<GCP Id=\"" << n << "\" Pixel=\"" << point.first << "\" Line=\"" << i << "\" X=\"" << (point.second.longitude*RAD2DEG) << "\" Y=\"" << (point.second.latitude*180.0/M_PI) << "\" />\n";
                n++;
            }
        }
    }

    stream << "</GCPList>\n";
    stream.flush();
    file.close();
}

std::vector<float> Projector::calculate_sunz(const std::vector<double> &timestamps, Imager sensor, SatID sat) {
    if (timestamps.size() == 0) {
        return { };
    }

    size_t width = sensor_info.at(sensor).width;
    size_t height = timestamps.size();
    const size_t pointsx = 21;
    const size_t pointsy = 41;
    std::vector<float> sunz(pointsy * pointsx);
    
    Config proj_info("projection.ini");
    auto params = proj_info.sections.at(satellite_info.at(sat).name + "_" + sensor_info.at(sensor).name);
    double fov = str2double(params["fov"]);
    double yaw = str2double(params["yaw"]);
    double roll = str2double(params["roll"]);
    double pitch = str2double(params["pitch"]);
    double pitchscale = str2double(params["pitchscale"]);
    double toffset = str2double(params["toffset"]);
    bool curved = params["curved"] == "true";
    d_sensor = sensor_info.at(sensor);

    for (size_t i = 0; i < pointsy; i++) {
        double timestamp = timestamps[round(i * (double)height/(double)pointsy)] + toffset;
        struct predict_position orbit = predictor.predict(timestamp);

        double azimuth;
        {
            struct predict_position a = predictor.predict(timestamp);
            struct predict_position b = predictor.predict(timestamp+0.1);
            azimuth = deg2rad(90) - calculateBearingAngle(Geodetic(a), Geodetic(b));
            if (azimuth > M_PI) {
                azimuth += deg2rad(yaw);
            } else {
                azimuth -= deg2rad(yaw);
            }
        }

        auto scan = calculate_scan(Geodetic(orbit), azimuth, fov, roll, pitch, pitchscale, curved, pointsx);
        for (size_t j = 0; j < pointsx; j++) {
            // UNIX to Julian date
            predict_julian_date_t prediction_time = (timestamp / 86400.0) - 3651.0;
            predict_observer_t observer = { "", scan[j].second.latitude, scan[j].second.longitude, 0 };

            struct predict_observation observation;
            predict_observe_sun(&observer, prediction_time, &observation);

            // Elevation to zenith angle
            sunz[i*pointsx + j] = deg2rad(90.0) - observation.elevation;
        }
    }

    // Interpolate points to the full image size
    std::vector<float> full_sunz(height * width);
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            double y2 = (double)y/(double)height * (pointsy-1);
            double x2 = (double)x/(double)width * (pointsx-1);

            float a = lerp(sunz[floor(y2)*pointsx + floor(x2)], sunz[floor(y2)*pointsx + ceil(x2)], fmodf(x2, 1.0));
            float b = lerp(sunz[ceil(y2) *pointsx + floor(x2)], sunz[ceil(y2) *pointsx + ceil(x2)], fmodf(x2, 1.0));
            full_sunz[y*width + x] = lerp(a, b, fmodf(y2, 1.0));
        }
    }

    return full_sunz;
}

std::vector<std::pair<double, Geodetic>> Projector::calculate_scan(const Geodetic &position, double azimuth, double fov, double roll, double pitch, double pitchscale, bool curved, size_t n) {
    std::vector<std::pair<double, Geodetic>> scan;

    for (size_t i = 0; i < n; i++) {
        // The angle that the sensor is pointing at
        double sensor_angle = (double)i/double(n-1)*2.0 - 1.0;
        double _pitch = pitch;
        if (curved) {
            _pitch *= sqrt(1.0 - pow(sensor_angle*pitchscale, 2.0));
        }
        sensor_angle *= fov*DEG2RAD;

        // The position that the sensor is looking at
        Geodetic point = los_to_earth(position, sensor_angle + deg2rad(roll), deg2rad(_pitch), azimuth);

        // Create a list of points and their respective coordinates
        scan.push_back({(double)i/double(n-1) * (double)d_sensor.width, point});
    }

    return scan;
}

bool Projector::is_northbound(const std::vector<double> &timestampts) {
    double lower_quartile = timestampts[timestampts.size()/4 * 1];
    double upper_quartile = timestampts[timestampts.size()/4 * 3];

    struct predict_position a = predictor.predict(lower_quartile);
    struct predict_position b = predictor.predict(upper_quartile);
    double azimuth = calculateBearingAngle(Geodetic(a), Geodetic(b));

    if (azimuth < -M_PI/2.0 || azimuth > M_PI/2.0) {
        return false;
    } else {
        return true;
    }
}
