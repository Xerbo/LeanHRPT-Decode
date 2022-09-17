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

#ifndef LEANHRPT_MAP_H_
#define LEANHRPT_MAP_H_

#include <QImage>
#include <QLineF>
#include <array>
#include <string>
#include <vector>

#include "geo/crs.h"
#include "projection.h"

namespace map {
// Checks that a Shapefile is readable and supported (Polyline/Polygon)
bool verify_shapefile(std::string filename);

// Decompose a Polyline/Polygon Shapefile into a list of line segments
std::vector<QLineF> read_shapefile(std::string filename);

// Sort line segments into 10x10 "buckets"
std::array<std::vector<QLineF>, 36 * 18> index_line_segments(const std::vector<QLineF> &line_segments);

// Warp an (indexed) map to fit a pass based off a point grid
std::vector<QLineF> warp_to_pass(const std::array<std::vector<QLineF>, 36 * 18> &buckets,
                                 const std::vector<std::pair<xy, Geodetic>> &points, size_t xn);

// Project a pass into Rectangular projection
QImage project(const QImage &image, const std::vector<std::pair<xy, Geodetic>> &points, size_t xn, QSize resolution,
               QRectF bounds);

QImage reproject(const QImage &image, transform::CRS crs, QRectF source_bounds, QRectF target_bounds);

// Render a map overlay on an image with Rectangular projection
void add_overlay(QImage &image, std::vector<QLineF> &line_segments, QColor color, transform::CRS crs, QRectF bounds);

// Calculate bounds of a pass, height is inverted
QRectF bounds(const std::vector<std::pair<xy, Geodetic>> &points);
QRectF bounds_crs(const std::vector<std::pair<xy, Geodetic>> &points, transform::CRS crs);
}  // namespace map

#endif
