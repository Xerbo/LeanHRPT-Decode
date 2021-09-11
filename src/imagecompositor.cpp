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
#include <algorithm>
#include <limits>
#include <omp.h>
#include <muParser.h>
#include <QLocale>

#include "config.h"

template<typename T>
T clamp(T v, T lo, T hi) {
    return std::max(lo, std::min(hi, v));
}

void ImageCompositor::import(RawImage *image, SatID satellite) {
    m_width = image->width();
    m_height = image->rows();
    m_channels = image->channels();
    m_isFlipped = false;

    rawChannels.clear();
    rawChannels.resize(m_channels);
    for(size_t i = 0; i < m_channels; i++) {
        rawChannels[i] = QImage(m_width, m_height, QImage::Format_Grayscale16);
        std::memcpy(rawChannels[i].bits(), image->getChannel(i), m_width * m_height * sizeof(uint16_t));
    }

    Config ini("calibration.ini");

    for (size_t i = 0; i < m_channels; i++) {
        std::string name = satellite_info.at(satellite).name + "/" + std::to_string(i+1);

        if (ini.sections.count(name)) {
            std::map<std::string, std::string> coefficients = ini.sections.at(name);

            QLocale l(QLocale::C);
            if (coefficients.count("a1")) {
                double a1 = l.toDouble(QString::fromStdString(coefficients.at("a1")));
                double b1 = l.toDouble(QString::fromStdString(coefficients.at("b1")));
                double a2 = l.toDouble(QString::fromStdString(coefficients.at("a2")));
                double b2 = l.toDouble(QString::fromStdString(coefficients.at("b2")));
                double c  = l.toDouble(QString::fromStdString(coefficients.at("c")));
                calibrate_avhrr(rawChannels[i], a1, b1, a2, b2, c);
            } else if (coefficients.count("a")) {
                double a = l.toDouble(QString::fromStdString(coefficients.at("a")));
                double b = l.toDouble(QString::fromStdString(coefficients.at("b")));
                calibrate_linear(rawChannels[i], a, b);
            }
        }
    }
}

void ImageCompositor::calibrate_avhrr(QImage &image, double a1, double b1, double a2, double b2, double c) {
    quint16 *bits = reinterpret_cast<quint16 *>(image.bits());

    for (size_t i = 0; i < (size_t)image.height()*(size_t)image.width(); i++) {
        double count = bits[i]/64;
        if (count < c) {
            count = a1*count + b1;
        } else {
            count = a2*count + b2;
        }
        bits[i] = clamp(count/100.0, 0.0, 1.0) * UINT16_MAX;
    }
}

void ImageCompositor::calibrate_linear(QImage &image, double a, double b) {
    quint16 *bits = reinterpret_cast<quint16 *>(image.bits());

    for (size_t i = 0; i < (size_t)image.height()*(size_t)image.width(); i++) {
        double count = bits[i]/64;
        count = a*count + b;
        bits[i] = clamp(count/100.0, 0.0, 1.0) * UINT16_MAX;
    }
}

void ImageCompositor::getChannel(QImage &image, size_t channel) {
    if (image.format() != QImage::Format_Grayscale16) {
        image = QImage(image.width(), image.height(), QImage::Format_Grayscale16);
    }
    std::memcpy(image.bits(), rawChannels[channel-1].bits(), sizeof(unsigned short) * m_width * m_height);
}

// Composites 3 Grayscale16 images into each channel of a RGB16 image
void ImageCompositor::getComposite(QImage &image, std::array<size_t, 3> chs) {
    if (image.format() != QImage::Format_RGBX64) {
        image = QImage(image.width(), image.height(), QImage::Format_RGBX64);
    }
    QRgba64 *bits = reinterpret_cast<QRgba64 *>(image.bits());

    quint16 *chbits[3];
    for (size_t i = 0; i < 3; i++) {
        chbits[i] = reinterpret_cast<quint16 *>(rawChannels[chs[i]-1].bits());
    }

    #pragma omp parallel for
    for (size_t i = 0; i < m_height*m_width; i++) {
        bits[i] = QRgba64::fromRgba64(chbits[0][i], chbits[1][i], chbits[2][i], UINT16_MAX);
    }
}

double set_rgb(double r, double g, double b) {
    QRgba64 a = QRgba64::fromRgba64(
        clamp(r, 0.0, 1.0) * (double)UINT16_MAX,
        clamp(g, 0.0, 1.0) * (double)UINT16_MAX,
        clamp(b, 0.0, 1.0) * (double)UINT16_MAX,
        UINT16_MAX
    );
    return *(double *)&a;
}
double set_bw(double val) {
    return set_rgb(val, val, val);
}

// Evaluate `expression` and store the results in `image`
void ImageCompositor::getExpression(QImage &image, std::string experssion) {
    if (image.format() != QImage::Format_RGBX64) {
        image = QImage(image.width(), image.height(), QImage::Format_RGBX64);
    }
    std::vector<quint16 *> rawbits(m_channels);
    for (size_t i = 0; i < m_channels; i++) {
        rawbits[i] = (quint16 *)rawChannels[i].bits();
    }
    QRgba64 *bits = reinterpret_cast<QRgba64 *>(image.bits());

    #pragma omp parallel
    {
        std::vector<double> ch(m_channels);
        mu::Parser p;

        try {
            for (size_t i = 0; i < m_channels; i++) {
                p.DefineVar("ch" + std::to_string(i+1), &ch[i]);
            }

            if (m_channels == 10) {
                p.DefineVar("SWIR", &ch[5]);
            } else {
                p.DefineVar("SWIR", &ch[2]);
            }
            p.DefineVar("NIR", &ch[1]);
            p.DefineVar("RED", &ch[0]);

            p.DefineFun("rgb", set_rgb);
            p.DefineFun("bw", set_bw);
            p.SetExpr(experssion);
        } catch (mu::Parser::exception_type &e) {
            std::cout << e.GetMsg() << std::endl;
        }

        size_t start =  (float)omp_get_thread_num()      /(float)omp_get_num_threads() * (float)m_height;
        size_t end   = ((float)omp_get_thread_num()+1.0f)/(float)omp_get_num_threads() * (float)m_height;

        for (size_t y = start; y < end; y++) {
            for (size_t x = 0; x < m_width; x++) {
                for (size_t i = 0; i < m_channels; i++) {
                    ch[i] = (double)rawbits[i][y*m_width + x] / (double)UINT16_MAX;
                }

                double val = p.Eval();
                bits[y*m_width + x] = *(QRgba64 *)&val;
            }
        }
    }
}

void ImageCompositor::setFlipped(bool state) {
    if (state != m_isFlipped) {
        flip();
    }
}
void ImageCompositor::flip() {
    m_isFlipped = !m_isFlipped;
    for (size_t i = 0; i < rawChannels.size(); i++) {
        rawChannels[i] = rawChannels[i].mirrored(true, true);
    }
}

template<typename T, size_t A, size_t B>
std::vector<size_t> ImageCompositor::create_histogram(QImage &image, float clip_limit) {
    std::vector<size_t> histogram(std::numeric_limits<T>::max()+1);
    T *bits = (T *)image.bits();

    // Skip every other pixel for speed (this is a bodge but helps a lot)
    for (size_t i = 0; i < (size_t)image.width() * (size_t)image.height(); i += 2) {
        histogram[bits[i*A + B]] += 2;
    }

    clip_histogram(histogram, clip_limit);
    return histogram;
}

template<typename T>
std::vector<size_t> ImageCompositor::create_rgb_histogram(QImage &image, float clip_limit) {
    std::vector<size_t> histogram(std::numeric_limits<T>::max()+1);
    T *bits = (T *)image.bits();

    for (size_t i = 0; i < (size_t)image.width() * (size_t)image.height(); i += 3) {
        histogram[bits[i*4+0]]++;
        histogram[bits[i*4+1]]++;
        histogram[bits[i*4+2]]++;
    }

    clip_histogram(histogram, clip_limit);
    return histogram;
}

template<typename T, size_t A, size_t B>
void ImageCompositor::_equalise(QImage &image, Equalization equalization, std::vector<size_t> histogram) {
    if (equalization == Equalization::None) return;

    size_t max = std::numeric_limits<T>::max();

    // Calculate cumulative frequency
	size_t sum = 0;
    std::vector<size_t> cf(histogram.size());
	for(size_t i = 0; i < cf.size(); i++){
		sum += histogram[i];
		cf[i] = (sum*max) / (image.width()*image.height());
	}

    quint16 *bits = reinterpret_cast<quint16 *>(image.bits());
    switch (equalization) {
        case Histogram: {
            #pragma omp parallel for
            for (size_t i = 0; i < (size_t)image.width()*(size_t)image.height(); i++) {
                bits[i*A + B] = cf[bits[i*A + B]];
            }
            break;
        }
        case Stretch: {
            size_t low = 0, high = max;
            for(size_t i = 0; i < max; i++) {
                if(low == 0 && cf[i] > 0.01f*max) {
                    low = i;
                }
                if(cf[i] > 0.99f*max) {
                    high = i;
                    break;
                }
            }

            // Rescale [low, high] to [0, 65535]
            #pragma omp parallel for
            for (size_t i = 0; i < (size_t)image.width()*(size_t)image.height(); i++) { 
                float val = (static_cast<float>(bits[i*A + B]) - low) * 65535.0f/(high - low);
                bits[i*A + B] = clamp(val, 0.0f, 65535.0f);
            }
            break;
        }
        default:
            throw std::runtime_error("Invalid equalization");
    }
}

void ImageCompositor::clip_histogram(std::vector<size_t>& histogram, float clip_limit) {
    if (clip_limit >= 1.0f) return;

    size_t clipLimit = *std::max_element(histogram.begin(), histogram.end()) * clip_limit;
    size_t clipped = 0;

    for (size_t i = 0; i < histogram.size(); i++) {
        if (histogram[i] > clipLimit) {
            clipped += histogram[i] - clipLimit;
            histogram[i] = clipLimit;
        }
    }

    // Redistribute clipped pixels
    size_t redistBatch = clipped / histogram.size();
    size_t residual = clipped - redistBatch * histogram.size();

    #pragma omp parallel for
    for (size_t i = 0; i < histogram.size(); i++) {
        histogram[i] += redistBatch;
    }

    if (residual != 0) {
        size_t residualStep = std::max(histogram.size() / residual, (size_t)1);
        for (size_t i = 0; i < histogram.size() && residual > 0; i += residualStep, residual--) {
            histogram[i]++;
        }
    }
}

void ImageCompositor::equalise(QImage &image, Equalization equalization, float clipLimit, bool brightness_only) {
    switch (image.format()) {
        case QImage::Format_RGBX64:
            if (brightness_only) {
                std::vector<size_t> histogram = create_rgb_histogram<uint16_t>(image, clipLimit);
                _equalise<uint16_t, 4, 0>(image, equalization, histogram);
                _equalise<uint16_t, 4, 1>(image, equalization, histogram);
                _equalise<uint16_t, 4, 2>(image, equalization, histogram);
            } else {
                _equalise<uint16_t, 4, 0>(image, equalization, create_histogram<uint16_t, 4, 0>(image, clipLimit));
                _equalise<uint16_t, 4, 1>(image, equalization, create_histogram<uint16_t, 4, 1>(image, clipLimit));
                _equalise<uint16_t, 4, 2>(image, equalization, create_histogram<uint16_t, 4, 2>(image, clipLimit));
            }
            break;
        case QImage::Format_Grayscale16:
            _equalise<uint16_t, 1, 0>(image, equalization, create_histogram<uint16_t, 1, 0>(image, clipLimit));
            break;
        default:
            throw new std::runtime_error("Unimplemented");
    }
}
