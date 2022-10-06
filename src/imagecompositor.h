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

#ifndef LEANHRPT_IMAGECOMPOSITOR_H_
#define LEANHRPT_IMAGECOMPOSITOR_H_

#include <QImage>
#include <QPainter>
#include <array>
#include <cmath>
#include <vector>

#include "map.h"
#include "protocol/rawimage.h"
#include "satinfo.h"
#include "util.h"

enum class Equalization { None, Histogram, Stretch };

class ImageCompositor {
   public:
    /**
     * Loads raw data from a Decoder
     *
     * @param image The actual image data itself
     * @param satellite The satellite
     * @param sensor The sensor
     * @param caldata A map of data containing information used for dynamic calibration
     * @param reverse Flips the image vertically, used for reverse transmissions
     */
    void import(RawImage *image, SatID satellite, Imager sensor, std::map<std::string, double> caldata, double reverse = false);

    /// Get a channel and write the result into `image`
    void getChannel(QImage &image, size_t channel);
    /// Create a composite and write the result into `image`
    void getComposite(QImage &image, std::array<size_t, 3> chs);
    /// @copydoc getComposite
    void getComposite(QImage &image, size_t r, size_t g, size_t b) { getComposite(image, {r, g, b}); }
    /// Evaluate an expression and write the result into `image`
    void getExpression(QImage &image, std::string expression);

    /**
     * Adds overlays and final effects to an image
     *
     * - Geometry correction
     * - Map overlays
     * - Landmark overlays
     * - Flipping
     * - IR Blend
     */
    void postprocess(QImage &image, bool correct = false);
    /**
     * Equalises an image
     *
     * @param image The image to equalise
     * @param equalization The type of equalization
     * @param clipLimit Clips the histogram, multiplier of the maximum histogram value
     * @param brightness_only Only change the brightness of the image
     */
    static void equalise(QImage &image, Equalization equalization, float clipLimit, bool brightness_only);

    /// Gets the width of currenty loaded image
    size_t width() { return m_width; };
    /// Gets the height of currenty loaded image
    size_t height() { return m_height; };
    /// Gets the number of channels in the currenty loaded image
    size_t channels() { return m_channels; };
    /// If the image is currently flipped
    bool flipped() { return m_isFlipped; }

    /// Set weather the image is flipped or not
    void setFlipped(bool state);
    /**
     * Set weather the image has a thermal overlay
     *
     * Channel 5 on meteor, 4 on all other satellites
     */
    void enableIRBlend(bool enable) { ir_blend = enable; }

    std::vector<QLineF> overlay;
    bool enable_map = false;
    QColor map_color;
    std::vector<float> sunz;
    std::vector<QColor> stops;
    std::vector<bool> ch3a;
    bool has_ch3a = false;

    bool enable_landmarks = false;
    QColor landmark_color;
    std::vector<Landmark> landmarks;

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

    template <typename T, size_t A, size_t B>
    static std::vector<size_t> create_histogram(QImage &image, float clip_limit = 1.0f);
    template <typename T>
    static std::vector<size_t> create_rgb_histogram(QImage &image, float clip_limit = 1.0f);

    template <typename T, size_t A, size_t B>
    static void _equalise(QImage &image, Equalization equalization, std::vector<size_t> histogram);
    static void clip_histogram(std::vector<size_t> &histogram, float clip_limit);
};

#endif
