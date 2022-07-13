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
#include <vector>

enum class Protocol {
    Unknown,
    LRPT,
    HRPT,
    AHRPT,
    MeteorHRPT,
    FengYunHRPT,
    GAC,
    GACReverse,
    DSB
};

enum Mission {
    POES,
    MeteorM,
    FengYun3,
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
    MeteorM2,
    MeteorM22,
    Unknown
};

enum Imager {
    AVHRR,
    VIRR,
    MSUMR,
    MHS,
    MTVZA,
    HIRS
};

struct SensorInfo {
    std::string name;
    float swath; // in km
    float resolution; // in km/px
    size_t width;
};

const std::map<Imager, SensorInfo> sensor_info = {
    { Imager::AVHRR, SensorInfo {"AVHRR",  2900.0f, 1.1f,  2048 } },
    { Imager::VIRR,  SensorInfo {"VIRR",   2800.0f, 1.1f,  2048 } },
    { Imager::MSUMR, SensorInfo {"MSU-MR", 2800.0f, 1.0f,  1572 } },
    { Imager::MHS,   SensorInfo {"MHS",    2180.0f, 16.0f, 90   } },
    { Imager::MTVZA, SensorInfo {"MTVZA",  1500.0f, 4.0f,  200  } },
    { Imager::HIRS,  SensorInfo {"HIRS",   2160.0f, 40.0f, 56   } },
};

const std::map<std::string, Imager> sensors = {
    { "AVHRR",  Imager::AVHRR },
    { "VIRR",   Imager::VIRR },
    { "MSU-MR", Imager::MSUMR },
    { "MHS",    Imager::MHS },
    { "MTVZA",  Imager::MTVZA },
    { "HIRS",   Imager::HIRS },
};

struct SatelliteInfo {
    float orbit_height; // in km
    Mission mission;
    std::string name;
    Imager default_imager;
};

const std::map<SatID, SatelliteInfo> satellite_info {
    { SatID::MetOpA,    SatelliteInfo { 827.0f, Mission::MetOp,    "MetOp-A",    Imager::AVHRR } },
    { SatID::MetOpB,    SatelliteInfo { 827.0f, Mission::MetOp,    "MetOp-B",    Imager::AVHRR } },
    { SatID::MetOpC,    SatelliteInfo { 817.0f, Mission::MetOp,    "MetOp-C",    Imager::AVHRR } },
    { SatID::FengYun3A, SatelliteInfo { 834.0f, Mission::FengYun3, "FengYun-3A", Imager::VIRR } },
    { SatID::FengYun3B, SatelliteInfo { 836.0f, Mission::FengYun3, "FengYun-3B", Imager::VIRR } },
    { SatID::FengYun3C, SatelliteInfo { 836.0f, Mission::FengYun3, "FengYun-3C", Imager::VIRR } },
    { SatID::NOAA15,    SatelliteInfo { 813.0f, Mission::POES,     "NOAA-15",    Imager::AVHRR } },
    { SatID::NOAA18,    SatelliteInfo { 854.0f, Mission::POES,     "NOAA-18",    Imager::AVHRR } },
    { SatID::NOAA19,    SatelliteInfo { 870.0f, Mission::POES,     "NOAA-19",    Imager::AVHRR } },
    { SatID::MeteorM2,  SatelliteInfo { 820.0f, Mission::MeteorM,  "Meteor-M2",  Imager::MSUMR } },
    { SatID::MeteorM22, SatelliteInfo { 821.0f, Mission::MeteorM,  "Meteor-M22", Imager::MSUMR } },
};

#endif
