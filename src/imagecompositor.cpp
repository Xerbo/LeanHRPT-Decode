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

#include "imagecompositor.h"

#include <cmath>
#include <cstring>

// Constructor
ImageCompositor::ImageCompositor()
    : m_equalization(None),
      isFlipped(false),
      histogram(new size_t[65536]),
      cf(new size_t[65536]) { }

ImageCompositor::~ImageCompositor() {
    delete[] histogram;
    delete[] cf;
}

void ImageCompositor::importFromRaw(RawImage *image) {
    m_width = image->width();
    m_height = image->rows();
    m_channels = image->channels();

    rawChannels.clear();
    for(size_t i = 0; i < image->channels(); i++) {
        rawChannels.push_back(QImage(m_width, m_height, QImage::Format_Grayscale16));
        std::memcpy(rawChannels[i].bits(), image->getChannel(i), sizeof(unsigned short) * m_width * m_height);
    }
}

void ImageCompositor::setFlipped(bool state) {
    if (state != isFlipped) {
        flip();
    }
}
void ImageCompositor::flip() {
    isFlipped = !isFlipped;
    for (size_t i = 0; i < rawChannels.size(); i++) {
        rawChannels[i] = rawChannels[i].mirrored(true, true);
    }
}

void ImageCompositor::getChannel(QImage *image, unsigned int channel) {
    std::memcpy(image->bits(), rawChannels[channel-1].bits(), sizeof(unsigned short) * m_width * m_height);
    equalise(image);
}

void ImageCompositor::getNdvi(QImage *image) {
    quint16 *bits = reinterpret_cast<quint16 *>(image->bits());
    quint16 *ch1 = reinterpret_cast<quint16 *>(rawChannels[0].bits());
    quint16 *ch2 = reinterpret_cast<quint16 *>(rawChannels[1].bits());

    for (size_t i = 0; i < m_height*m_width; i++) {
        float red = ch1[i];
        float NIR = ch2[i];
        float NDVI = (NIR - red) / (NIR + red);

        bits[i] = (NDVI + 1) * 32768;
    }
}

void ImageCompositor::getComposite(QImage *image, int chs[3]) {
    QRgba64 *bits = reinterpret_cast<QRgba64 *>(image->bits());

    QImage equalised[3];
    quint16 *equalisedBits[3];
    for (size_t i = 0; i < 3; i++) {
        equalised[i] = QImage(rawChannels[chs[i]-1]);
        equalise(&equalised[i]);
        equalisedBits[i] = reinterpret_cast<quint16 *>(equalised[i].bits());
    }

    for (size_t i = 0; i < m_height*m_width; i++) {
        bits[i].setRed(equalisedBits[0][i]);
        bits[i].setGreen(equalisedBits[1][i]);
        bits[i].setBlue(equalisedBits[2][i]);
    }
}

template<typename T>
T clamp(T v, T lo, T hi) {
    if (v > hi) return hi;
    if (v < lo) return lo;
    return v;
}

void ImageCompositor::equalise(QImage *image) {
    quint16 *bits = reinterpret_cast<quint16 *>(image->bits());

    std::memset(histogram, 0, sizeof(size_t) * 65536);
    for (size_t i = 0; i < m_height*m_width; i++) {
        histogram[bits[i]]++;
    }

    // Calculate cumulative frequency
	size_t sum = 0;
	for(size_t i = 0; i < 65536; i++){
		sum += histogram[i];
		cf[i] = (sum*65536) / (m_height*m_width);
	}

    switch (m_equalization) {
        case None: break;
        case Histogram: {
            for (size_t i = 0; i < m_height*m_width; i++) {
                bits[i] = cf[bits[i]];
            }
            break;
        }
        case Stretch: {
            size_t low = 0, high = 65536;
            for(size_t i = 0; i < 65536; i++) {
                if(low == 0 && cf[i] > 0.01*65536) {
                    low = i;
                }
                if(cf[i] > 0.99*65536) {
                    high = i;
                    break;
                }
            }

            // Rescale [low, high] to [0, 65535]
            for (size_t i = 0; i < m_height*m_width; i++) { 
                float val = (static_cast<float>(bits[i]) - low) * 65535.0f/(high - low);
                bits[i] = clamp(val, 0.0f, 65535.0f);
            }
            break;
        }
        default: throw std::runtime_error("Invalid equalization");
    }
}
