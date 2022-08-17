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

#include "map.h"

#include <shapefil.h>

#include <QPainter>
#include <QPolygonF>
#include <QTransform>
#include <set>

#include "math.h"

bool map::verify_shapefile(std::string filename) {
    SHPHandle shapefile = SHPOpen(filename.c_str(), "rb");
    if (shapefile == NULL) {
        return false;
    }

    int shapetype;
    SHPGetInfo(shapefile, NULL, &shapetype, NULL, NULL);
    SHPClose(shapefile);

    return shapetype == SHPT_ARC || shapetype == SHPT_POLYGON;
}

std::vector<QLineF> map::read_shapefile(std::string filename) {
    SHPHandle shapefile = SHPOpen(filename.c_str(), "rb");
    if (shapefile == NULL) return {};

    int entities;
    SHPGetInfo(shapefile, &entities, NULL, NULL, NULL);

    std::vector<QLineF> line_segments;
    for (int i = 0; i < entities; i++) {
        SHPObject *object = SHPReadObject(shapefile, i);

        for (int j = 0; j < object->nParts; j++) {
            int start = object->panPartStart[j];
            int end = (j == object->nParts - 1) ? object->nVertices : object->panPartStart[j + 1];

            for (int k = start; k < end - 1; k++) {
                // clang-format off
                line_segments.push_back(QLineF(
                    object->padfX[k],
                    object->padfY[k],
                    object->padfX[k+1],
                    object->padfY[k+1]
                ));
                // clang-format on
            }
        }

        SHPDestroyObject(object);
    }

    SHPClose(shapefile);
    return line_segments;
}

std::array<std::vector<QLineF>, 36 * 18> map::index_line_segments(const std::vector<QLineF> &line_segments) {
    std::array<std::vector<QLineF>, 36 * 18> buckets;

    for (const auto &segment : line_segments) {
        QPointF center = segment.center();
        int lon = (center.x() + 180) / 10;
        int lat = (center.y() + 90) / 10;
        buckets[lat * 36 + lon].push_back(segment);
    }

    return buckets;
}

std::vector<QLineF> map::warp_to_pass(const std::array<std::vector<QLineF>, 36 * 18> &buckets,
                                      const std::vector<std::pair<xy, Geodetic>> &points, size_t xn) {
    std::vector<QLineF> warped;

    for (size_t y = 0; y < points.size() / xn - 1; y++) {
        for (size_t x = 0; x < xn - 1; x++) {
            //                     top left      top right     bottom right  bottom left
            size_t vertices[4] = {(y + 0) * xn + x + 0, (y + 0) * xn + x + 1, (y + 1) * xn + x + 1, (y + 1) * xn + x + 0};

            QPolygonF geo, pixels;
            for (size_t vertex : vertices) {
                geo << QPointF(points[vertex].second.longitude * RAD2DEG, points[vertex].second.latitude * RAD2DEG);
                pixels << QPointF(points[vertex].first.first, points[vertex].first.second);
            }

            // TODO: phase unwrapping
            QRectF bounds = geo.boundingRect();
            if (bounds.width() > 180) continue;

            // Find out what "buckets" are required for this polygon
            std::set<size_t> indices;
            for (int y = (bounds.top() + 90) / 10; y <= (bounds.bottom() + 90) / 10; y++)
                for (int x = (bounds.left() + 180) / 10; x <= (bounds.right() + 180) / 10; x++) {
                    indices.insert(y * 36 + x);
                }

            // Create the transformation
            QTransform trans;
            if (!QTransform::quadToQuad(geo, pixels, trans)) continue;

            // Warp lines within the selected buckets
            for (size_t indice : indices) {
                for (size_t i = 0; i < buckets[indice].size(); i++) {
                    QLineF line = buckets[indice][i];

                    if (geo.containsPoint(line.center(), Qt::OddEvenFill)) {
                        QPointF p1 = buckets[indice][i].p1();
                        QPointF p2 = buckets[indice][i].p2();
                        warped.push_back(QLineF(trans.map(p1), trans.map(p2)));
                    }
                }
            }
        }
    }

    return warped;
}

// Convert from the center of a pixel at `x` to 0.0 to 1.0 (non inclusive)
double px2r(double x, double range) { return (x + 0.5) / range; }

// Opposite of px2r
double r2px(double x, double range) { return x * range - 0.5; }

QImage map::project(const QImage &image, const std::vector<std::pair<xy, Geodetic>> &points, size_t xn, QSize resolution,
                    double xa, double xb, double ya, double yb) {
    QImage warped(resolution, image.format());
    warped.fill(0);

    int height = resolution.height();
    int width = resolution.width();

#pragma omp parallel for
    for (size_t y = 0; y < points.size() / xn - 1; y++) {
        for (size_t x = 0; x < xn - 1; x++) {
            //                     top left      top right     bottom right  bottom left
            size_t vertices[4] = {(y + 0) * xn + x + 0, (y + 0) * xn + x + 1, (y + 1) * xn + x + 1, (y + 1) * xn + x + 0};

            QPolygonF px, geo;
            for (size_t vertex : vertices) {
                geo << QPointF(points[vertex].second.longitude * RAD2DEG, points[vertex].second.latitude * RAD2DEG);
                px << QPointF(points[vertex].first.first, points[vertex].first.second);
            }

            // West/east unwrapping
            if (geo.boundingRect().width() > 180.0) {
                for (size_t i = 0; i < 4; i++) {
                    if (geo[i].x() < 0.0) geo[i].rx() += 360.0;
                }
            }
            QRectF bounds = geo.boundingRect();

            // Project as rectangular for non polar regions
            if (fabs(bounds.center().y()) < 88.0) {
                // Create the transformation
                QTransform trans;
                if (!QTransform::quadToQuad(geo, px, trans)) continue;

                // Fill all pixels
                for (int i = r2px((bounds.top() - yb) / ya, height); i < r2px((bounds.bottom() - yb) / ya, height); i++)
                    for (int j = r2px((bounds.left() - xb) / xa, width); j < r2px((bounds.right() - xb) / xa, width); j++) {
                        if (i < 0 || i > height || j < 0 || j > width * 2) break;

                        QPointF point(px2r(j, width) * xa + xb, px2r(i, height) * ya + yb);
                        if (geo.containsPoint(point, Qt::OddEvenFill)) {
                            QPointF pixel = trans.map(point);
                            QPoint out(j % width, (height - 1) - i);

                            warped.setPixelColor(out, lerp2(image, pixel));
                        }
                    }
            }

            // Project as azimuthal equidistant for polar regions
            // This is less efficient as every pixel has to be checked
            if (fabs(bounds.center().y()) > 86.0) {
                bool north = bounds.center().y() > 0.0;

                // Convert to azimuthal equidistant
                QPolygonF polar;
                for (size_t i = 0; i < 4; i++) {
                    double lon = geo[i].x() * DEG2RAD;
                    double lat = (geo[i].y() + 90.0) / 180.0;
                    if (north) lat = 1.0 - lat;

                    polar << QPointF(cos(lon) * lat, sin(lon) * lat);
                }

                // Create the transformation
                QTransform trans;
                if (!QTransform::quadToQuad(polar, px, trans)) continue;

                int offset = north ? 86 : -90;
                for (int i = r2px((offset - yb) / ya, height); i < r2px(((offset + 4) - yb) / ya, height); i++)
                    for (int j = r2px((-180 - xb) / xa, width); j < r2px((180 - xb) / xa, width); j++) {
                        if (i < 0 || i > height || j < 0 || j > width) break;

                        double lon = px2r(j, width) * xa + xb;
                        double lat = px2r(i, height) * ya + yb;
                        lon *= DEG2RAD;
                        lat = (lat + 90.0) / 180.0;
                        if (north) lat = 1.0 - lat;

                        QPointF point(cos(lon) * lat, sin(lon) * lat);
                        if (polar.containsPoint(point, Qt::OddEvenFill)) {
                            QPointF pixel = trans.map(point);
                            QPoint out(j, (height - 1) - i);

                            warped.setPixelColor(out, lerp2(image, pixel));
                        }
                    }
            }
        }
    }

    return warped;
}

void map::add_overlay(QImage &image, std::vector<QLineF> &line_segments, QColor color, double xa, double xb, double ya,
                      double yb) {
    image = image.convertToFormat(QImage::Format_RGBX64);

    QPainter painter(&image);
    painter.setPen(color);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const QLineF &line : line_segments) {
        QPointF p1 = line.p1();
        QPointF p2 = line.p2();

        p1.rx() = r2px((p1.x() - xb) / xa, image.width());
        p2.rx() = r2px((p2.x() - xb) / xa, image.width());
        p1.ry() = (image.height() - 1) - r2px((p1.y() - yb) / ya, image.height());
        p2.ry() = (image.height() - 1) - r2px((p2.y() - yb) / ya, image.height());

        painter.drawLine(p1, p2);
    }
}

QRectF map::bounds(const std::vector<std::pair<xy, Geodetic>> &points) {
    QPointF min(180, 90);
    QPointF max(-180, -90);
    for (const auto &x : points) {
        min.rx() = std::min(min.x(), x.second.longitude * RAD2DEG);
        min.ry() = std::min(min.y(), x.second.latitude * RAD2DEG);
        max.rx() = std::max(max.x(), x.second.longitude * RAD2DEG);
        max.ry() = std::max(max.y(), x.second.latitude * RAD2DEG);
    }

    return QRectF(min, max);
}
