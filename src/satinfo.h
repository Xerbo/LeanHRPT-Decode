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
#include <string>

enum Mission {
    NOAA, // Should be POES
    Meteor, // Should be MeteorM
    FengYun, // Should be FengYun3
    MetOp
};

enum SatID {
    MetOpA,
    MetOpB,
    MetOpC,
    FengYun3A,
    FengYun3B,
    FengYun3C,
    NOAA15,
    NOAA18,
    NOAA19,
    MeteorM2, // Not used
    MeteorM22,
    Unknown
};

struct SatelliteInfo {
    float orbit_height; // in km
    float resolution; // in km/px
    float swath; // in km
    Mission mission;
    std::string name;
    std::string imager;
};

const std::map<SatID, SatelliteInfo> satellite_info {
    { SatID::MetOpA,    SatelliteInfo { 827.0f, 1.1f, 2900.0f, Mission::MetOp,   "MetOp-A",    "AVHRR" } },
    { SatID::MetOpB,    SatelliteInfo { 827.0f, 1.1f, 2900.0f, Mission::MetOp,   "MetOp-B",    "AVHRR" } },
    { SatID::MetOpC,    SatelliteInfo { 817.0f, 1.1f, 2900.0f, Mission::MetOp,   "MetOp-C",    "AVHRR" } },
    { SatID::FengYun3A, SatelliteInfo { 834.0f, 1.1f, 2800.0f, Mission::FengYun, "FengYun-3A", "VIRR" } },
    { SatID::FengYun3B, SatelliteInfo { 836.0f, 1.1f, 2800.0f, Mission::FengYun, "FengYun-3B", "VIRR" } },
    { SatID::FengYun3C, SatelliteInfo { 836.0f, 1.1f, 2800.0f, Mission::FengYun, "FengYun-3C", "VIRR" } },
    { SatID::NOAA15,    SatelliteInfo { 813.0f, 1.1f, 2900.0f, Mission::NOAA,    "NOAA-15",    "AVHRR" } },
    { SatID::NOAA18,    SatelliteInfo { 854.0f, 1.1f, 2900.0f, Mission::NOAA,    "NOAA-18",    "AVHRR" } },
    { SatID::NOAA19,    SatelliteInfo { 870.0f, 1.1f, 2900.0f, Mission::NOAA,    "NOAA-19",    "AVHRR" } },
    { SatID::MeteorM2,  SatelliteInfo { 820.0f, 1.0f, 2800.0f, Mission::Meteor,  "Meteor-M2",  "MSU-MR" } },
    { SatID::MeteorM22, SatelliteInfo { 821.0f, 1.0f, 2800.0f, Mission::Meteor,  "Meteor-M22", "MSU-MR" } },
};

#endif
