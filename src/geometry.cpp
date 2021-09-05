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

#include "geometry.h"

#include <cmath>

const float EARTH_RADIUS = 6371.0f;

// Convert from a internal angle of a circle to the viewing angle of a point above the circle.
float earth2sat_angle(float radius, float height, float angle) {
    return -std::atan(std::sin(angle)*radius / (std::cos(angle)*radius - (radius+height)));
}

// Based off https://github.com/Xerbo/meteor_corrector
QImage correct_geometry(QImage image, SatID satellite) {
    const SatelliteInfo info = satellite_info.at(satellite);
    const size_t output_width = info.swath/info.resolution;

    size_t *lut = new size_t[output_width];

    float view_angle = info.swath / EARTH_RADIUS;
    float sat_edge = earth2sat_angle(EARTH_RADIUS, info.orbit_height, view_angle/2);

    // Compute a look up table of pixel positions
    for (size_t x = 0; x < output_width; x++) {
        float angle = (static_cast<float>(x)/static_cast<float>(output_width) - 0.5f) * view_angle;
        angle = earth2sat_angle(EARTH_RADIUS, info.orbit_height, angle);

        lut[x] = (angle/sat_edge + 1.0f)/2.0f * static_cast<float>(image.width());
    }

    // Copy pixels over from the source to the corrected image
    QImage corrected(output_width, image.height(), image.format());
    for (size_t y = 0; y < static_cast<size_t>(image.height()); y++) {
        for (size_t x = 0; x < output_width; x++) {
            corrected.setPixelColor(x, y, image.pixelColor(lut[x], y));
        }
    }

    delete[] lut;
    return corrected;
}
