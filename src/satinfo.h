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

#ifndef LEANHRPT_SATINFO_H
#define LEANHRPT_SATINFO_H

#include <map>

enum Satellite {
    Unknown,
    NOAA,
    Meteor,
    FengYun,
    MetOp
};

struct SatelliteInfo {
    float orbit_height;
    float resolution;
    float swath;
};

// Derek Approved™
const std::map<Satellite, SatelliteInfo> satellite_info {
    {Satellite::NOAA,    SatelliteInfo { 862.0f, 1.1f, 2900.0f }},
    {Satellite::MetOp,   SatelliteInfo { 822.0f, 1.1f, 2900.0f }},
    {Satellite::FengYun, SatelliteInfo { 836.0f, 1.1f, 2800.0f }},
    {Satellite::Meteor,  SatelliteInfo { 820.0f, 1.0f, 2800.0f }}
};

const float EARTH_RADIUS = 6371.0f;

#endif
