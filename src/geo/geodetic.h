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

#ifndef LEANHRPT_GEO_GEODETIC_H_
#define LEANHRPT_GEO_GEODETIC_H_

#include <predict/predict.h>

// A data structure containing latitude, longitude and altitude
struct Geodetic {
    double latitude;
    double longitude;
    double altitude;

   public:
    Geodetic(double lat, double lon, double alt) : latitude(lat), longitude(lon), altitude(alt) {}

    Geodetic(const predict_position &position) : Geodetic(position.latitude, position.longitude, position.altitude) {}
};

#endif
