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

enum Equalization {
    None,
    Histogram,
    Stretch
};

// Takes in raw data, does stuff to it, and puts out a QImage
class ImageCompositor {
    public:
        ImageCompositor();
        ~ImageCompositor();

        void importFromRaw(RawImage *image);

        // Manipulation functions
        void setEqualization(Equalization type) { m_equalization = type; };
        void setFlipped(bool state);
        void flip();
        void setClipLimit(float limit) { m_clipLimit = limit; };

        // Return images
        void getChannel(QImage *image, unsigned int channel);
        void getComposite(QImage *image, int chs[3]);
        void getExpression(QImage *image, std::string experssion);

        unsigned int width() { return m_width; };
        unsigned int height() { return m_height; };
        unsigned int channels() { return m_channels; };
    private:
        void equalise(QImage *image, size_t mul, size_t shift);
        void equalise_rgb(QImage *image) { equalise(image, 4, 0); equalise(image, 4, 1); equalise(image, 4, 2); };
        void equalise_bw(QImage *image) { equalise(image, 1, 0); };
        size_t m_width;
        size_t m_height;
        size_t m_channels;
        float m_clipLimit = 1.0f;
        std::vector<QImage> rawChannels;

        Equalization m_equalization;

        bool isFlipped;

        size_t *histogram;
        size_t *cf;
};

#endif
