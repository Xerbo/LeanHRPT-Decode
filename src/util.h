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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_2PI
#define M_2PI (M_PI * 2.0)
#endif
#ifndef M_PI_2
#define M_PI_2 (M_PI / 2.0)
#endif
#ifndef M_PI_4
#define M_PI_4 (M_PI / 2.0)
#endif

#ifndef LEANHRPT_UTIL_H_
#define LEANHRPT_UTIL_H_

#include <cmath>

#define RAD2DEG (180.0 / M_PI)
#define DEG2RAD (M_PI / 180.0)

inline double deg2rad(double deg) { return deg * DEG2RAD; }
inline double rad2deg(double rad) { return rad * RAD2DEG; }
#ifdef QPOINT_H
inline QPointF deg2rad(QPointF deg) { return QPointF(deg.x() * DEG2RAD, deg.y() * DEG2RAD); }
inline QPointF rad2deg(QPointF rad) { return QPointF(rad.x() * RAD2DEG, rad.y() * RAD2DEG); }
#endif

template <typename T>
inline T clamp(T v, T lo, T hi) {
    return std::max(lo, std::min(hi, v));
}

template <typename T>
inline T lerp(T a, T b, T x) {
    return a * (1.0 - x) + b * x;
}

#ifdef QCOLOR_H
// clang-format off
inline QRgba64 lerp(QRgba64 a, QRgba64 b, double x) {
    return QRgba64::fromRgba64(
        a.red()   * (1.0 - x) + b.red()   * x,
        a.green() * (1.0 - x) + b.green() * x,
        a.blue()  * (1.0 - x) + b.blue()  * x,
        UINT16_MAX
    );
}

inline QColor lerp(QColor a, QColor b, double x) {
    return QColor::fromRgbF(
        a.redF()   * (1.0 - x) + b.redF()   * x,
        a.greenF() * (1.0 - x) + b.greenF() * x,
        a.blueF()  * (1.0 - x) + b.blueF()  * x
    );
}
// clang-format on
#endif

#if defined(QIMAGE_H) && defined(QCOLOR_H)
// clang-format off
inline QColor lerp2(const QImage &image, double x, double y) {
    QColor a = lerp(image.pixelColor(floor(x), floor(y)), image.pixelColor(ceil(x), floor(y)), fmod(x, 1.0));
    QColor b = lerp(image.pixelColor(floor(x),  ceil(y)), image.pixelColor(ceil(x),  ceil(y)), fmod(x, 1.0));

    return lerp(a, b, fmod(y, 1.0));
}
// clang-format on

inline QColor lerp2(const QImage &image, QPointF point) { return lerp2(image, point.x(), point.y()); }
#endif

#endif
