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
    FengYunHRPT
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
    MHS
};

enum class Format {
    Unknown,
    DegreeC,
    Percent,
    Raw10,
    Raw16
};

struct FormatInfo {
    std::string unit;
    double scale;
    double offset;
};

const std::map<Format, FormatInfo> format_info = {
    { Format::Unknown, FormatInfo { "?", 100.0/(double)UINT16_MAX, 0.0 }},
    { Format::DegreeC, FormatInfo { "°C", 160.0/(double)UINT16_MAX, -80.0 }},
    { Format::Percent, FormatInfo { "%", 100.0/(double)UINT16_MAX, 0.0 }},
    { Format::Raw10, FormatInfo { "counts", 1.0/64.0, 0.0 }},
    { Format::Raw16, FormatInfo { "counts", 1.0, 0.0 }}
};

struct ChannelInfo {
    double wavelength;
    std::string wl_unit;
    Format format;
};

const std::map<Imager, std::vector<ChannelInfo>> channels = {
    {Imager::AVHRR, {
        ChannelInfo { 0.630, "µm", Format::Percent },
        ChannelInfo { 0.862, "µm", Format::Percent },
        ChannelInfo { -1.0,  "?",  Format::Unknown }, // Fucking channel 3A/3B
        ChannelInfo { 10.80, "µm", Format::Raw10 },
        ChannelInfo { 12.00, "µm", Format::Raw10 },
    }},
    {Imager::MHS, {
        ChannelInfo { 89.0,    "GHz (V)", Format::Raw16 },
        ChannelInfo { 157.0,   "GHz (V)", Format::Raw16 },
        ChannelInfo { 183.331, "GHz (H)", Format::Raw16 },
        ChannelInfo { 183.331, "GHz (H)", Format::Raw16 },
        ChannelInfo { 190.331, "GHz (V)", Format::Raw16 },
    }}
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
    { Imager::MHS,   SensorInfo {"MHS",    2180.0f, 16.0f, 90   } }
};

const std::map<std::string, Imager> sensors = {
    { "AVHRR",  Imager::AVHRR },
    { "VIRR",   Imager::VIRR },
    { "MSU-MR", Imager::MSUMR },
    { "MHS",    Imager::MHS },
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
