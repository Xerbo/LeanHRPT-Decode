/*
 * LeanHRPT Decode
 * Copyright (C) 2021-2022 Xerbo
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

#ifndef LEANHRPT_PROJECTION_H_
#define LEANHRPT_PROJECTION_H_

#include <cmath>
#include <utility>
#include <vector>

#include "geo/geodetic.h"
#include "orbit.h"
#include "satinfo.h"
#include "util.h"

const double EARTH_RADIUS = 6371.0;
const double EARTH_CIRCUMFERENCE = EARTH_RADIUS * 2.0 * M_PI;
using xy = std::pair<double, double>;

namespace geo {
// Convert from a internal angle of a circle to the viewing angle of a point above the circle.
inline double earth2sat_angle(double radius, double height, double angle) {
    return -atan(sin(angle) * radius / (cos(angle) * radius - (radius + height)));
}
inline double sat2earth_angle(double radius, double height, double angle) {
    return asin((radius + height) / radius * sin(angle)) - angle;
}
}  // namespace geo

class Projector {
   public:
    Projector(std::pair<std::string, std::string> tle) : predictor(tle) {}

    /**
     * Create a GCP grid
     *
     * @returns A vector containing geographical tags for an image
     */
    std::vector<std::pair<xy, Geodetic>> calculate_gcps(const std::vector<double> &timestamps, size_t pointsy, size_t pointsx,
                                                        Imager sensor, SatID sat, size_t width);

    /**
     * Saves GCPs into a file, format is `x,y,lat,lon`
     */
    void save_gcp_file(const std::vector<double> &timestamps, size_t pointsy, size_t pointsx, Imager sensor, SatID sat,
                       std::string filename, size_t width);
    std::vector<float> calculate_sunz(const std::vector<double> &timestamps, Imager sensor, SatID sat, size_t width);

    /**
     * If the provided timestamps represent a northbound pass
     *
     * Internally this takes the lower and upper quartile positions, and calculates the azimuth from them
     */
    bool is_northbound(const std::vector<double> &timestamps);

   private:
    std::vector<std::pair<double, Geodetic>> calculate_scan(const Geodetic &position, double azimuth, double fov, double roll,
                                                            double pitch, double pitchscale, bool curved, size_t n);

    OrbitPredictor predictor;
};

#endif
