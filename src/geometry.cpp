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

#include "projection.h"
#include "util.h"

/// Based off https://github.com/Xerbo/meteor_corrector
QImage correct_geometry(QImage image, SatID satellite, Imager sensor, size_t width) {
    const SatelliteInfo satinfo = satellite_info.at(satellite);
    const SensorInfo sensorinfo = sensor_info.at(sensor);

    double ratio = (double)width / (double)sensorinfo.width;
    const size_t output_width = sensorinfo.swath / sensorinfo.resolution * ratio;
    std::vector<float> lut(output_width);

    float view_angle = sensorinfo.swath / EARTH_RADIUS;
    float sat_edge = geo::earth2sat_angle(EARTH_RADIUS, satinfo.orbit_height, view_angle / 2);

    // Compute a look up table of pixel positions
    for (size_t x = 0; x < output_width; x++) {
        float angle = (static_cast<float>(x) / static_cast<float>(output_width) - 0.5f) * view_angle;
        angle = geo::earth2sat_angle(EARTH_RADIUS, satinfo.orbit_height, angle);

        lut[x] = (angle / sat_edge + 1.0f) / 2.0f * static_cast<float>(image.width() - 1);
    }

    // Copy pixels over from the source to the corrected image
    QImage corrected(output_width, image.height(), image.format());
#pragma omp parallel for
    for (size_t y = 0; y < static_cast<size_t>(image.height()); y++) {
        for (size_t x = 0; x < output_width; x++) {
            QColor a = image.pixelColor(floor(lut[x]), y);
            QColor b = image.pixelColor(ceil(lut[x]), y);
            corrected.setPixelColor(x, y, lerp(a, b, fmod(lut[x], 1.0)));
        }
    }

    return corrected;
}

void correct_points(std::vector<QPointF> &points, SatID satellite, Imager sensor, size_t width) {
    const SatelliteInfo satinfo = satellite_info.at(satellite);
    const SensorInfo sensorinfo = sensor_info.at(sensor);

    double ratio = (double)width / (double)sensorinfo.width;
    const size_t output_width = sensorinfo.swath / sensorinfo.resolution * ratio;
    std::vector<float> lut(output_width);

    float view_angle = sensorinfo.swath / EARTH_RADIUS;
    float sat_edge = geo::earth2sat_angle(EARTH_RADIUS, satinfo.orbit_height, view_angle / 2);

    // Copy pixels over from the source to the corrected image
    for (QPointF &point : points) {
        point.rx() =
            geo::sat2earth_angle(EARTH_RADIUS, satinfo.orbit_height, (point.x() / width - 0.5) * 2.0 * sat_edge) / view_angle +
            0.5;
        point.rx() *= output_width;
    }
}

void correct_lines(std::vector<QLineF> &lines, SatID satellite, Imager sensor, size_t width) {
    correct_points((std::vector<QPointF> &)lines, satellite, sensor, width);
}
