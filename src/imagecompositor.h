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

#ifndef LEANHRPT_IMAGECOMPOSITOR_H
#define LEANHRPT_IMAGECOMPOSITOR_H

#include <vector>
#include <QImage>
#include <QPainter>
#include "generic/rawimage.h"
#include "satinfo.h"

enum Equalization {
    None,
    Histogram,
    Stretch
};

class ImageCompositor {
    public:
        void import(RawImage *image, SatID satellite, Imager sensor, std::map<std::string, double> caldata);

        // Manipulation functions
        void flip();
        void setFlipped(bool state);

        void getChannel(QImage &image, size_t channel);
        void getComposite(QImage &image, std::array<size_t, 3> chs);
        void getExpression(QImage &image, std::string expression);

        size_t width()    { return m_width; };
        size_t height()   { return m_height; };
        size_t channels() { return m_channels; };

        QImage map;
        bool enable_map = false;
        QColor map_color;

        void load_map(QString filename) {
            map = QImage(filename).convertToFormat(QImage::Format_ARGB32);

            for (size_t y = 0; y < (size_t)map.height(); y++) {
                for (size_t x = 0; x < (size_t)map.width(); x++) {
                    if (map.pixelColor(x, y).red() == 0) {
                        map.setPixelColor(x, y, QColor(0, 0, 0, 0));
                    } else {
                        map.setPixelColor(x, y, map_color);
                    }
                }
            }
        }

        void overlay_map(QImage &image) {
            if (!map.isNull() && enable_map) {
                image = image.convertToFormat(QImage::Format_RGBX64);
                QPainter painter(&image);
                if (m_isFlipped) {
                    painter.drawImage(0, 0, map.mirrored(true, true));
                } else {
                    painter.drawImage(0, 0, map);
                }
            }
        }

        static void equalise(QImage &image, Equalization equalization, float clipLimit, bool brightness_only);
    private:
        size_t m_width;
        size_t m_height;
        size_t m_channels;
        bool m_isFlipped;
        std::vector<QImage> rawChannels;
        std::map<std::string, double> d_caldata;

        void calibrate_avhrr(QImage &image, double a1, double b1, double a2, double b2, double c);
        void calibrate_linear(QImage &image, double a, double b);
        void calibrate_ir(size_t ch, double Ns, double b0, double b1, double b2, double Vc, double A, double B);

        template<typename T, size_t A, size_t B>
        static std::vector<size_t> create_histogram(QImage &image, float clip_limit = 1.0f);
        template<typename T>
        static std::vector<size_t> create_rgb_histogram(QImage &image, float clip_limit = 1.0f);

        template<typename T, size_t A, size_t B>
        static void _equalise(QImage &image, Equalization equalization, std::vector<size_t> histogram);
        static void clip_histogram(std::vector<size_t>& histogram, float clip_limit);
};

#endif
