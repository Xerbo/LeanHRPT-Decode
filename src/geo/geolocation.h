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

#ifndef LEANHRPT_GEO_GEOLOCATION_H
#define LEANHRPT_GEO_GEOLOCATION_H

#include "geodetic.h"
#include "matrix.h"
#include "vector.h"

Vector locationToVector(const Geodetic &location);
Geodetic vectorToLocation(const Vector &vector);
Geodetic los_to_earth(const Geodetic &position, double roll, double pitch, double yaw);
Geodetic los_to_earth(const Vector &position, double roll, double pitch, double yaw);
double calculateBearingAngle(const Geodetic &start, const Geodetic &end);
Matrix4x4 lookAt(const Vector3 &position, const Vector3 &target, const Vector3 &up);

#endif
