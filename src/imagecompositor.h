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
#include "generic/rawimage.h"
#include "satinfo.h"

enum Equalization {
    None,
    Histogram,
    Stretch
};

class ImageCompositor {
    public:
        void import(RawImage *image, SatID satellite);

        // Manipulation functions
        void flip();
        void setFlipped(bool state);

        void getChannel(QImage &image, size_t channel);
        void getComposite(QImage &image, std::array<size_t, 3> chs);
        void getExpression(QImage &image, std::string expression);

        size_t width()    { return m_width; };
        size_t height()   { return m_height; };
        size_t channels() { return m_channels; };

        static void equalise(QImage &image, Equalization equalization, float clipLimit);
    private:
        size_t m_width;
        size_t m_height;
        size_t m_channels;
        bool m_isFlipped;
        std::vector<QImage> rawChannels;

        void calibrate_avhrr(QImage &image, double a1, double b1, double a2, double b2, double c);
        void calibrate_linear(QImage &image, double a, double b);

        template<typename T, size_t A, size_t B>
        static std::vector<size_t> create_histogram(QImage &image);
        template<typename T, size_t A, size_t B>
        static void _equalise(QImage &image, Equalization equalization, float clipLimit);
        static void clip_histogram(std::vector<size_t>& histogram, float clip_limit);
};

#endif
