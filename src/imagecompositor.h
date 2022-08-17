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

#include <QImage>
#include <QPainter>
#include <array>
#include <cmath>
#include <vector>

#include "math.h"
#include "protocol/rawimage.h"
#include "satinfo.h"

enum Equalization { None, Histogram, Stretch };

class ImageCompositor {
   public:
    void import(RawImage *image, SatID satellite, Imager sensor, std::map<std::string, double> caldata, double reverse = false);

    // Manipulation functions
    void setFlipped(bool state);
    bool flipped() { return m_isFlipped; }

    void getChannel(QImage &image, size_t channel);
    void getComposite(QImage &image, std::array<size_t, 3> chs);
    void getExpression(QImage &image, std::string expression);

    size_t width() { return m_width; };
    size_t height() { return m_height; };
    size_t channels() { return m_channels; };

    std::vector<QLineF> overlay;
    bool enable_map = false;
    QColor map_color;
    std::vector<float> sunz;
    std::vector<QColor> stops;
    std::vector<bool> ch3a;
    bool has_ch3a = false;

    void postprocess(QImage &image, bool correct = false);

    void enableIRBlend(bool enable) { ir_blend = enable; }

    static void equalise(QImage &image, Equalization equalization, float clipLimit, bool brightness_only);

   private:
    size_t m_width;
    size_t m_height;
    size_t m_channels;
    SatID m_satellite;
    Imager m_sensor;
    bool m_isFlipped;
    std::vector<QImage> rawChannels;
    std::map<std::string, double> d_caldata;
    bool ir_blend = false;

    void calibrate_avhrr(size_t ch, double a1, double b1, double a2, double b2, double c);
    void calibrate_linear(size_t ch, double a, double b);
    void calibrate_ir(size_t ch, double Ns, double b0, double b1, double b2, double Vc, double A, double B);

    template <typename T, size_t A, size_t B>
    static std::vector<size_t> create_histogram(QImage &image, float clip_limit = 1.0f);
    template <typename T>
    static std::vector<size_t> create_rgb_histogram(QImage &image, float clip_limit = 1.0f);

    template <typename T, size_t A, size_t B>
    static void _equalise(QImage &image, Equalization equalization, std::vector<size_t> histogram);
    static void clip_histogram(std::vector<size_t> &histogram, float clip_limit);
};

#endif
