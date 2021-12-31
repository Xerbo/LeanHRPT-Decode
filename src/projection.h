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

#ifndef LEANHRPT_PROJECTION_H
#define LEANHRPT_PROJECTION_H

#include "satinfo.h"
#include "orbit.h"
#include "geo/geodetic.h"
#include <utility>
#include <cmath>
#include <vector>
#include <QDebug>

const double EARTH_RADIUS = 6371.0;

namespace geo {
    // Convert from a internal angle of a circle to the viewing angle of a point above the circle.
    inline double earth2sat_angle(double radius, double height, double angle) {
        return -atan(sin(angle)*radius / (cos(angle)*radius - (radius+height)));
    }
}

class Projector {
    public:
        Projector(std::pair<std::string, std::string> tle)
            : predictor(tle) { }

        void save_gcp_file(std::vector<double> &timestamps, size_t pointsy, size_t pointsx, Imager sensor, SatID sat, std::string filename);
        std::vector<float> calculate_sunz(const std::vector<double> &timestamps, Imager sensor, SatID sat);

        bool is_northbound(const std::vector<double> &timestampts);
    private:
        std::vector<std::pair<double, Geodetic>> calculate_scan(const Geodetic &position, double azimuth, double fov, double roll, double pitch, size_t n);

        OrbitPredictor predictor;
        SensorInfo d_sensor;
};

#endif
