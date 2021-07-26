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
#include <iostream>

#include <muParser.h>

template<typename T>
T clamp(T v, T lo, T hi) {
    return std::max(lo, std::min(hi, v));
}

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
    equalise_bw(image);
}

double set_rgb(double r, double g, double b) {
    QRgba64 a;
    a.setRed(clamp(r, 0.0, 1.0) * (double)UINT16_MAX);
    a.setGreen(clamp(g, 0.0, 1.0) * (double)UINT16_MAX);
    a.setBlue(clamp(b, 0.0, 1.0) * (double)UINT16_MAX);
    return *(double *)&a;
}
double set_bw(double val) {
    QRgba64 a;
    a.setRed(clamp(val, 0.0, 1.0) * (double)UINT16_MAX);
    a.setGreen(clamp(val, 0.0, 1.0) * (double)UINT16_MAX);
    a.setBlue(clamp(val, 0.0, 1.0) * (double)UINT16_MAX);
    return *(double *)&a;
}

void ImageCompositor::getExpression(QImage *image, std::string experssion) {
    std::vector<double> ch(m_channels);

	try {
        mu::Parser p;
        for (size_t i = 0; i < m_channels; i++) {
            p.DefineVar("ch" + std::to_string(i+1), &ch[i]);
        }
        p.DefineFun("rgb", set_rgb);
        p.DefineFun("bw", set_bw);
        p.SetExpr(experssion);

        QRgba64 *bits = reinterpret_cast<QRgba64 *>(image->bits());
        for (size_t y = 0; y < m_height; y++) {
            for (size_t x = 0; x < m_width; x++) {
                for (size_t i = 0; i < m_channels; i++) {
                    ch[i] = ((quint16 *)rawChannels[i].bits())[y*m_width + x] / (double)UINT16_MAX;
                }

                double val = p.Eval();
                bits[y*m_width + x] = *(QRgba64 *)&val;
            }
        }
    } catch (mu::Parser::exception_type &e) {
		std::cout << e.GetMsg() << std::endl;
	}

    equalise_rgb(image);
}

void ImageCompositor::getComposite(QImage *image, int chs[3]) {
    QRgba64 *bits = reinterpret_cast<QRgba64 *>(image->bits());

    QImage equalised[3];
    quint16 *equalisedBits[3];
    for (size_t i = 0; i < 3; i++) {
        equalised[i] = QImage(rawChannels[chs[i]-1]);
        equalise_bw(&equalised[i]);
        equalisedBits[i] = reinterpret_cast<quint16 *>(equalised[i].bits());
    }

    for (size_t i = 0; i < m_height*m_width; i++) {
        bits[i].setRed(equalisedBits[0][i]);
        bits[i].setGreen(equalisedBits[1][i]);
        bits[i].setBlue(equalisedBits[2][i]);
    }

    //equalise_rgb(image);
}

void ImageCompositor::equalise(QImage *image, size_t mul, size_t shift) {
    quint16 *bits = reinterpret_cast<quint16 *>(image->bits());

    std::memset(histogram, 0, sizeof(size_t) * 65536);
    for (size_t i = 0; i < m_height*m_width; i++) {
        histogram[bits[i*mul+shift]]++;
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
                bits[i*mul+shift] = cf[bits[i*mul+shift]];
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
                float val = (static_cast<float>(bits[i*mul+shift]) - low) * 65535.0f/(high - low);
                bits[i*mul+shift] = clamp(val, 0.0f, 65535.0f);
            }
            break;
        }
        default: throw std::runtime_error("Invalid equalization");
    }
}
