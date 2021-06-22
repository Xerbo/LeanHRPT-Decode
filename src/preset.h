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
    std::set<Satellite> satellites;
    std::string expression;
};

const std::map<std::string, Preset> presets = {
    { "NDVI", { "Normalized Difference Vegetation Index", { NOAA, Meteor, FengYun, MetOp }, "bw((ch2-ch1)/(ch2+ch1)*0.5 + 0.5)" } },
    { "FengYun Truecolor", { "RGB197 composite from the VIRR imager", { FengYun }, "rgb(ch1, ch9, ch7)" } }
};

#endif
