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
#include "math.h"

#include <set>
#include <QTransform>
#include <QPolygonF>
#include <shapefil.h>

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
            int end = (j == object->nParts-1) ? object->nVertices : object->panPartStart[j+1];

            for (int k = start; k < end-1; k++) {
                line_segments.push_back(QLineF(
                    object->padfX[k],
                    object->padfY[k],
                    object->padfX[k+1],
                    object->padfY[k+1]
                ));
            }
        }

        SHPDestroyObject(object);
    }

    SHPClose(shapefile);
    return line_segments;
}

std::array<std::vector<QLineF>, 36*18> map::index_line_segments(const std::vector<QLineF> &line_segments) {
    std::array<std::vector<QLineF>, 36*18> buckets;

    for (const auto &segment : line_segments) {
        QPointF center = segment.center();
        int lon = (center.x()+180)/10;
        int lat = (center.y()+90)/10;
        buckets[lat*36 + lon].push_back(segment);
    }

    return buckets;
}

std::vector<QLineF> map::warp_to_pass(const std::array<std::vector<QLineF>, 36*18> &buckets, const std::vector<std::pair<xy, Geodetic>> &points, size_t xn) {
    std::vector<QLineF> warped;

    for (size_t y = 0; y < points.size()/xn-1; y++) {
        for (size_t x = 0; x < xn-1; x++) {
            // top left, top right, bottom right, bottom left
            size_t vertices[4] = {
                (y+0)*xn+x+0,
                (y+0)*xn+x+1,
                (y+1)*xn+x+1,
                (y+1)*xn+x+0
            };

            QPolygonF geo, pixels;
            for (size_t vertex : vertices) {
                geo    << QPointF(points[vertex].second.longitude*RAD2DEG, points[vertex].second.latitude*RAD2DEG);
                pixels << QPointF(points[vertex].first.first,              points[vertex].first.second);
            }

            // TODO: phase unwrapping
            QRectF bounds = geo.boundingRect();
            if (bounds.width() > 180) continue;

            // Find out what "buckets" are required for this polygon
            std::set<size_t> indices;
            for (int y = (bounds.top()+90)/10;  y <= (bounds.bottom()+90)/10; y++)
            for (int x = (bounds.left()+180)/10; x <= (bounds.right()+180)/10; x++) {
                indices.insert(y*36 + x);
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
                        warped.push_back(QLineF(
                            trans.map(p1),
                            trans.map(p2)
                        ));
                    }
                }
            }
        }
    }

    return warped;
}
