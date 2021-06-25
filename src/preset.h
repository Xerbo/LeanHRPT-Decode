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

#ifndef LEANHRPT_PRESET_H
#define LEANHRPT_PRESET_H

#include <map>
#include <set>
#include <string>

#include "satinfo.h"

struct Preset {
    std::string description;
    std::string author;
    std::set<Satellite> satellites;
    std::string expression;
};

const std::map<std::string, Preset> presets = {
    // Universal
    { "NDVI",             { "Normalized Difference Vegetation Index", "Xerbo", { NOAA, Meteor, FengYun, MetOp }, "bw((ch2-ch1)/(ch2+ch1)*0.5 + 0.5)" } },
    { "RGB221",           { "Direct RGB221 composite",                "Xerbo", { NOAA, Meteor, FengYun, MetOp }, "rgb(ch2, ch2, ch1)" } },
    // FengYun exclusives
    { "True Color",       { "Direct RGB197 composite",                "Xerbo", { FengYun },                      "rgb(ch1, ch9, ch7)" } },
    { "Natural Color",    { "A mix of true color and SWIR",           "Derek", { FengYun },                      "rgb(blend(ch1, max(ch1, ch6), 0.85), blend(ch9, (ch9, ch2), 0.85), blend(ch7, max(ch7, ch1), 0.85))" } },
    // SWIR
    { "FY SWIR",          { "Direct RGB621 composite",                "Xerbo", { FengYun },                      "rgb(ch6, ch2, ch1)" } },
    { "SWIR",             { "Direct RGB321 composite",                "Xerbo", { Meteor, MetOp },                "rgb(ch3, ch2, ch1)" } },
    { "FY Enhanced SWIR", { "RGB621 without the blue tint",           "Derek", { FengYun },                      "rgb(max(ch6, ch2), ch2, ch1)" } },
    { "Enhanced SWIR",    { "RGB321 without the blue tint",           "Derek", { Meteor, MetOp },                "rgb(max(ch3, ch2), ch2, ch1)" } },
};

#endif
