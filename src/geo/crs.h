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

#ifndef LEANHRPT_GEO_CRS_H_
#define LEANHRPT_GEO_CRS_H_

#include <QPointF>
#include <string>
#include <vector>

namespace transform {
enum class CRS { Equdistant, Mercator, North_Polar, South_Polar };

const std::vector<std::string> CRS_NAMES = {"Equdistant", "Mercator", "North Polar", "South Polar"};

// Lat/Long in radians
using Geo = QPointF;
// XY in relative units (0 to 1)
using XY = QPointF;

// Input in radians, output in XY (0 to 1)
XY forward(Geo geo, CRS crs);
// Input in XY (0 to 1), output in radians
Geo reverse(XY xy, CRS crs);
}  // namespace transform

#endif
