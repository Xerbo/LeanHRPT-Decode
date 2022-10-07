/*
 * LeanHRPT Decode
 * Copyright (C) 2021-2022 Xerbo
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

#include "compositor.h"

#include <muParser.h>
#include <omp.h>

#include <QLocale>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>

#include "calibration.h"
#include "config/config.h"
#include "geometry.h"
#include "util.h"

void ImageCompositor::import(RawImage *image, SatID satellite, Imager sensor, std::map<std::string, double> caldata,
                             double reverse) {
    m_width = image->width();
    m_height = image->rows();
    m_channels = image->channels();
    m_satellite = satellite;
    m_sensor = sensor;
    m_isFlipped = false;
    d_caldata = caldata;

    rawChannels.clear();
    rawChannels.resize(m_channels);
    for (size_t i = 0; i < m_channels; i++) {
        rawChannels[i] = QImage(m_width, m_height, QImage::Format_Grayscale16);
        for (size_t y = 0; y < m_height; y++) {
            std::memcpy(rawChannels[i].scanLine(y), &image->getChannel(i)[y * m_width], m_width * sizeof(uint16_t));
        }
    }

    if (m_sensor == Imager::MSUMR) {
        rawChannels[3].invertPixels();
        rawChannels[4].invertPixels();
        rawChannels[5].invertPixels();
    }

    Calibrator(d_caldata, ch3a).calibrate(satellite, sensor, rawChannels);

    if (sensor == Imager::MHS || sensor == Imager::HIRS || sensor == Imager::AMSUA) {
        for (size_t i = 0; i < m_channels; i++) {
            rawChannels[i] = rawChannels[i].mirrored(true, false);
        }
    }
    if (reverse) {
        for (size_t i = 0; i < m_channels; i++) {
            rawChannels[i] = rawChannels[i].mirrored(false, true);
        }
    }
}

void ImageCompositor::postprocess(QImage &image, bool correct) {
    if (image.format() == QImage::Format_Grayscale16 && stops.size() > 1) {
        QImage copy(image);
        image = QImage(image.width(), image.height(), QImage::Format_RGBX64);

        for (size_t i = 0; i < m_height; i++) {
            uint16_t *bits = (uint16_t *)copy.scanLine(i);
            QRgba64 *color_bits = (QRgba64 *)image.scanLine(i);

#pragma omp parallel for
            for (size_t j = 0; j < m_width; j++) {
                double x = (double)bits[j] / (double)UINT16_MAX * (stops.size() - 1);

                QColor color = lerp(stops[floor(x)], stops[ceil(x)], fmod(x, 1.0));
                color_bits[j] = color.rgba64();
            }
        }
    }

    if (ir_blend) {
        QImage copy(m_sensor == Imager::MSUMR ? rawChannels[4] : rawChannels[3]);
        equalise(copy, Equalization::Histogram, 0.7f, false);

        for (size_t i = 0; i < m_height; i++) {
            uint16_t *ir = (uint16_t *)copy.scanLine(i);
            std::tuple<QRgba64 *, quint16 *> bits =
                std::make_tuple(reinterpret_cast<QRgba64 *>(image.scanLine(i)), reinterpret_cast<quint16 *>(image.scanLine(i)));

#pragma omp parallel for
            for (size_t j = 0; j < m_width; j++) {
                float _sunz = sunz[i * m_width + j];
                float x = clamp(_sunz * 10.0f - 14.8f, 0.0f, 1.0f);

                if (image.format() == QImage::Format_RGBX64) {
                    std::get<0>(bits)[j] = lerp(std::get<0>(bits)[j], QRgba64::fromRgba64(ir[j], ir[j], ir[j], UINT16_MAX), x);
                } else {
                    std::get<1>(bits)[j] = lerp<float>(std::get<1>(bits)[j], ir[j], x);
                }
            }
        }
    }

    if (correct) {
        image = correct_geometry(image, m_satellite, m_sensor, m_width);
    }

    if (enable_map) {
        auto _overlay = overlay;
        if (correct) {
            correct_lines(_overlay, m_satellite, m_sensor, m_width);
        }

        image = image.convertToFormat(QImage::Format_RGBX64);
        QPainter painter(&image);
        painter.setPen(map_color);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawLines(_overlay.data(), _overlay.size());
    }

    if (m_isFlipped) {
        image = image.mirrored(true, true);
    }

    if (enable_landmarks) {
        auto _landmarks = landmarks;
        if (correct) {
            std::vector<QPointF> points(_landmarks.size());
            for (size_t i = 0; i < points.size(); i++) {
                points[i] = _landmarks[i].geo;
            }
            correct_points(points, m_satellite, m_sensor, m_width);
            for (size_t i = 0; i < points.size(); i++) {
                _landmarks[i].geo = points[i];
            }
        }

        image = image.convertToFormat(QImage::Format_RGBX64);
        QPainter painter(&image);
        painter.setBrush(QBrush(landmark_color, Qt::SolidPattern));
        painter.setPen(landmark_color);
        painter.setRenderHint(QPainter::Antialiasing);
        QFont font = painter.font();
        font.setPixelSize(image.width() / 130);
        painter.setFont(font);

        for (const Landmark &landmark : _landmarks) {
            QPointF point = landmark.geo;
            if (m_isFlipped) {
                point.rx() = image.width() - point.x();
                point.ry() = image.height() - point.y();
            }
            painter.drawEllipse(point, image.width() / 500, image.width() / 500);
            painter.drawText(point.x() - 500, point.y() + 2.5, 1000, 250, Qt::AlignHCenter, landmark.text);
        }
    }
}

void ImageCompositor::getChannel(QImage &image, size_t channel) { image = rawChannels[channel - 1]; }

void ImageCompositor::getComposite(QImage &image, std::array<size_t, 3> chs) {
    if (image.format() != QImage::Format_RGBX64) {
        image = QImage(image.width(), image.height(), QImage::Format_RGBX64);
    }

    for (size_t i = 0; i < m_height; i++) {
        QRgba64 *line = (QRgba64 *)image.scanLine(i);
        const uint16_t *r = (const uint16_t *)rawChannels[chs[0] - 1].constScanLine(i);
        const uint16_t *g = (const uint16_t *)rawChannels[chs[1] - 1].constScanLine(i);
        const uint16_t *b = (const uint16_t *)rawChannels[chs[2] - 1].constScanLine(i);

#pragma omp parallel for
        for (size_t x = 0; x < m_width; x++) {
            line[x] = QRgba64::fromRgba64(r[x], g[x], b[x], UINT16_MAX);
        }
    }
}

void ImageCompositor::getExpression(QImage &image, std::string expression) {
    std::vector<double> ch(m_channels);
    double sunz_val = 0.0;
    double scan = 0.0;
    mu::Parser p;

    for (size_t i = 0; i < m_channels; i++) {
        p.DefineVar("ch" + std::to_string(i + 1), &ch[i]);
    }

    double swir, mwir;
    p.DefineVar("SWIR", &swir);
    p.DefineVar("MWIR", &mwir);
    p.DefineVar("NIR", &ch[1]);
    p.DefineVar("RED", &ch[0]);
    p.DefineVar("sunz", &sunz_val);
    p.DefineVar("scan", &scan);
    p.SetExpr(expression);

    int channels;
    p.Eval(channels);
    QImage::Format format = channels == 1 ? QImage::Format_Grayscale16 : QImage::Format_RGBX64;
    if (image.format() != format) {
        image = QImage(image.width(), image.height(), format);
    }

    try {
        for (size_t y = 0; y < m_height; y++) {
            std::vector<quint16 *> rawbits(m_channels);
            for (size_t i = 0; i < m_channels; i++) {
                rawbits[i] = (quint16 *)rawChannels[i].scanLine(y);
            }
            std::tuple<QRgba64 *, quint16 *> bits =
                std::make_tuple(reinterpret_cast<QRgba64 *>(image.scanLine(y)), reinterpret_cast<quint16 *>(image.scanLine(y)));

            for (size_t x = 0; x < m_width; x++) {
                scan = (double)x / (double)m_width;
                for (size_t i = 0; i < m_channels; i++) {
                    ch[i] = (double)rawbits[i][x] / (double)UINT16_MAX;
                }
                if (sunz.size() != 0) sunz_val = sunz[y * m_width + x];

                mwir = swir = 0.0;
                if (m_sensor == Imager::AVHRR) {
                    if (ch3a[y]) {
                        swir = ch[2];
                    } else {
                        mwir = ch[2];
                    }
                } else {
                    if (m_sensor == Imager::VIRR) {
                        swir = ch[5];
                    } else {
                        swir = ch[2];
                    }
                }

                double *rgb = p.Eval(channels);
                if (channels == 1) {
                    std::get<1>(bits)[x] = clamp(rgb[0], 0.0, 1.0) * (double)UINT16_MAX;
                } else {
                    std::get<0>(bits)[x] = QRgba64::fromRgba64(clamp(rgb[0], 0.0, 1.0) * (double)UINT16_MAX,
                                                               clamp(rgb[1], 0.0, 1.0) * (double)UINT16_MAX,
                                                               clamp(rgb[2], 0.0, 1.0) * (double)UINT16_MAX, UINT16_MAX);
                }
            }
        }
    } catch (mu::ParserError &e) {
        std::cout << e.GetMsg() << std::endl;
        return;
    }
}

void ImageCompositor::setFlipped(bool state) { m_isFlipped = state; }

template <typename T, size_t A, size_t B>
std::vector<size_t> ImageCompositor::create_histogram(QImage &image, float clip_limit) {
    std::vector<size_t> histogram(std::numeric_limits<T>::max() + 1);

    // Skip every other pixel for speed (this is a bodge but helps a lot)
    for (size_t y = 0; y < (size_t)image.height(); y++) {
        quint16 *line = reinterpret_cast<quint16 *>(image.scanLine(y));

        for (size_t x = 0; x < (size_t)image.width(); x++) {
            if (line[x * A + B] != 0) histogram[line[x * A + B]]++;
        }
    }

    clip_histogram(histogram, clip_limit);
    return histogram;
}

template <typename T>
std::vector<size_t> ImageCompositor::create_rgb_histogram(QImage &image, float clip_limit) {
    std::vector<size_t> histogram(std::numeric_limits<T>::max() + 1);

    for (size_t y = 0; y < (size_t)image.height(); y++) {
        quint16 *line = reinterpret_cast<quint16 *>(image.scanLine(y));

        for (size_t x = 0; x < (size_t)image.width(); x++) {
            if (std::min({line[x * 4 + 0], line[x * 4 + 1], line[x * 4 + 2]}) != 0) {
                histogram[line[x * 4 + 0]]++;
                histogram[line[x * 4 + 1]]++;
                histogram[line[x * 4 + 2]]++;
            }
        }
    }

    clip_histogram(histogram, clip_limit);
    return histogram;
}

template <typename T, size_t A, size_t B>
void ImageCompositor::_equalise(QImage &image, Equalization equalization, std::vector<size_t> histogram) {
    if (equalization == Equalization::None) return;

    size_t max = std::numeric_limits<T>::max();
    size_t histogram_count = std::accumulate(histogram.begin(), histogram.end(), 0) + 1;

    // Calculate cumulative frequency
    size_t sum = 0;
    std::vector<size_t> cf(histogram.size());
    for (size_t i = 0; i < cf.size(); i++) {
        sum += histogram[i];
        cf[i] = (sum * max) / histogram_count;
    }

    switch (equalization) {
        case Equalization::Histogram: {
#pragma omp parallel for
            for (size_t y = 0; y < (size_t)image.height(); y++) {
                quint16 *line = reinterpret_cast<quint16 *>(image.scanLine(y));

                for (size_t x = 0; x < (size_t)image.width(); x++) {
                    line[x * A + B] = cf[line[x * A + B]];
                }
            }
            break;
        }
        case Equalization::Stretch: {
            size_t low = 0, high = max;
            for (size_t i = 0; i < max; i++) {
                if (low == 0 && cf[i] > 0.01f * max) {
                    low = i;
                }
                if (cf[i] > 0.99f * max) {
                    high = i;
                    break;
                }
            }

// Rescale [low, high] to [0, 65535]
#pragma omp parallel for
            for (size_t y = 0; y < (size_t)image.height(); y++) {
                quint16 *line = reinterpret_cast<quint16 *>(image.scanLine(y));

                for (size_t x = 0; x < (size_t)image.width(); x++) {
                    float val = (static_cast<float>(line[x * A + B]) - low) * 65535.0f / (high - low);
                    line[x * A + B] = clamp(val, 0.0f, 65535.0f);
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Invalid equalization");
    }
}

void ImageCompositor::clip_histogram(std::vector<size_t> &histogram, float clip_limit) {
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
