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

#include "crs.h"

#include <stdexcept>

#include "util.h"

transform::XY forward_equdistant(transform::Geo geo) {
    double lon = geo.x();
    double lat = geo.y();
    return transform::XY((lon / M_PI + 1.0) / 2.0, (-lat / M_PI_2 + 1.0) / 2.0);
}
transform::XY forward_mercator(transform::Geo geo) {
    double lon = geo.x();
    double lat = geo.y();
    double y = -log(tan(M_PI_4 + lat / 2.0)) / M_PI;
    return transform::XY((lon / M_PI + 1.0) / 2.0, (y + 1.0) / 2.0);
}
transform::XY forward_north_polar(transform::Geo geo) {
    double lon = geo.x();
    double lat = geo.y();
    double x = sin(lon) * (-lat + M_PI_2) / M_PI;
    double y = cos(lon) * (-lat + M_PI_2) / M_PI;
    return transform::XY((x + 1.0) / 2.0, (y + 1.0) / 2.0);
}
transform::XY forward_south_polar(transform::Geo geo) {
    double lon = geo.x();
    double lat = geo.y();
    double x = sin(lon) * (lat + M_PI_2) / M_PI;
    double y = -cos(lon) * (lat + M_PI_2) / M_PI;
    return transform::XY((x + 1.0) / 2.0, (y + 1.0) / 2.0);
}
transform::XY transform::forward(transform::Geo geo, CRS crs) {
    switch (crs) {
        case CRS::Equdistant:
            return forward_equdistant(geo);
        case CRS::Mercator:
            return forward_mercator(geo);
        case CRS::North_Polar:
            return forward_north_polar(geo);
        case CRS::South_Polar:
            return forward_south_polar(geo);
        default:
            throw std::out_of_range("Unknown CRS");
    }
}

transform::Geo reverse_equdistant(transform::XY xy) {
    double x = xy.x() * 2.0 - 1.0;
    double y = xy.y() * 2.0 - 1.0;
    return transform::Geo(x * M_PI, -y * M_PI_2);
}
transform::Geo reverse_mercator(transform::XY xy) {
    double x = xy.x() * 2.0 - 1.0;
    double y = xy.y() * 2.0 - 1.0;
    double lat = 2.0 * atan(exp(-y * M_PI)) - M_PI_2;
    return transform::Geo(x * M_PI, lat);
}
transform::Geo reverse_north_polar(transform::XY xy) {
    double x = xy.x() * 2.0 - 1.0;
    double y = xy.y() * 2.0 - 1.0;
    double lon = atan2(x, y);
    double lat = hypot(x, y) * M_PI - M_PI_2;
    return transform::Geo(lon, -lat);
}
transform::Geo reverse_south_polar(transform::XY xy) {
    double x = xy.x() * 2.0 - 1.0;
    double y = xy.y() * 2.0 - 1.0;
    double lon = atan2(x, -y);
    double lat = hypot(x, y) * M_PI - M_PI_2;
    return transform::Geo(lon, lat);
}
transform::Geo transform::reverse(transform::XY xy, CRS crs) {
    switch (crs) {
        case CRS::Equdistant:
            return reverse_equdistant(xy);
        case CRS::Mercator:
            return reverse_mercator(xy);
        case CRS::North_Polar:
            return reverse_north_polar(xy);
        case CRS::South_Polar:
            return reverse_south_polar(xy);
        default:
            throw std::out_of_range("Unknown CRS");
    }
}
