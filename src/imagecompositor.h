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
#include <cmath>
#include "generic/rawimage.h"
#include "satinfo.h"
#include "math.h"

enum Equalization {
    None,
    Histogram,
    Stretch
};

inline QRgba64 lerp(QRgba64 a, QRgba64 b, float x) {
    return QRgba64::fromRgba64(
        a.red()  *(1.0f-x) + b.red()  *x,
        a.green()*(1.0f-x) + b.green()*x,
        a.blue() *(1.0f-x) + b.blue() *x,
        UINT16_MAX
    );
}

class ImageCompositor {
    public:
        void import(RawImage *image, SatID satellite, Imager sensor, std::map<std::string, double> caldata);

        // Manipulation functions
        void setFlipped(bool state);
        bool flipped() { return m_isFlipped; }

        void getChannel(QImage &image, size_t channel);
        void getComposite(QImage &image, std::array<size_t, 3> chs);
        void getExpression(QImage &image, std::string expression);

        size_t width()    { return m_width; };
        size_t height()   { return m_height; };
        size_t channels() { return m_channels; };

        QImage map;
        bool enable_map = false;
        QColor map_color;
        std::vector<float> sunz;

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
            if (ir_blend) {
                QImage copy(m_channels == 6 ? rawChannels[4] : rawChannels[3]);
                equalise(copy, Equalization::Histogram, 0.7f, false);
                if (m_isFlipped) copy = copy.mirrored(true, true);

                uint16_t *ir = (uint16_t *)copy.bits();
                std::tuple<QRgba64 *, quint16 *> bits = std::make_tuple(
                    reinterpret_cast<QRgba64 *>(image.bits()),
                    reinterpret_cast<quint16 *>(image.bits())
                );

                for (size_t i = 0; i < m_height*m_width; i++) {
                    float irval = m_channels == 6 ? (UINT16_MAX - ir[i]) : ir[i];

                    float _sunz = m_isFlipped ? sunz[(m_width*m_height-1) - i] : sunz[i];
                    float x = clamp(_sunz*10.0f-14.8f, 0.0f, 1.0f);

                    if (image.format() == QImage::Format_RGBX64) {
                        std::get<0>(bits)[i] = lerp(std::get<0>(bits)[i], QRgba64::fromRgba64(irval, irval, irval, UINT16_MAX), x);
                    } else {
                        std::get<1>(bits)[i] = lerp<float>(std::get<1>(bits)[i], irval, x);
                    }
                }
            }

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

        void enableIRBlend(bool enable) {
            ir_blend = enable;
        }

        static void equalise(QImage &image, Equalization equalization, float clipLimit, bool brightness_only);
    private:
        size_t m_width;
        size_t m_height;
        size_t m_channels;
        bool m_isFlipped;
        std::vector<QImage> rawChannels;
        std::map<std::string, double> d_caldata;
        bool ir_blend = false;

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
