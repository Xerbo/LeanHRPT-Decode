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

#ifndef LEANHRPT_GEOMETRY_H_
#define LEANHRPT_GEOMETRY_H_

#include <QImage>

#include "satinfo.h"

QImage correct_geometry(QImage image, SatID satellite, Imager sensor, size_t width);
void correct_points(std::vector<QPointF> &points, SatID satellite, Imager sensor, size_t width);
void correct_lines(std::vector<QLineF> &lines, SatID satellite, Imager sensor, size_t width);

#endif
