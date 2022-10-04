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

#include "rawimage.h"

#include <cstring>

#include "protocol/repack.h"

RawImage::RawImage(size_t width, size_t channels, size_t interleaving_size)
    : rowBuffer(new unsigned short[width * channels + 100]),
      m_width(width),
      m_channels(channels),
      m_interleavingSize(interleaving_size),
      m_rows(0) {
    imageBuffer.resize(m_channels);
    for (size_t i = 0; i < channels; i++) {
        imageBuffer[i].resize(width * 5000);
    }
}

RawImage::~RawImage() { delete[] rowBuffer; }

void RawImage::push10Bit(const uint8_t *data, int offset) {
    int j = offset / 4 * 5;     // Offset as close as we can get in just bytes
    int pxoffset = offset % 4;  // Numbers of pixels to offset after byte shifting

    repack10(&data[j], rowBuffer, m_width * m_channels + pxoffset);
    push16Bit(rowBuffer, pxoffset, 64);
}
void RawImage::push16Bit(const uint16_t *data, int offset, int multiplier) {
    for (size_t channel = 0; channel < m_channels; channel++) {
        if (m_interleavingSize == 1) {
            for (size_t x = 0; x < m_width; x++) {
                imageBuffer[channel][m_rows * m_width + x] = data[x * m_channels + channel + offset] * multiplier;
            }
            // Meteor just can't be normal can it
        } else {
            for (size_t x = 0; x < m_width; x += m_interleavingSize) {
                for (size_t i = 0; i < m_interleavingSize; i++) {
                    imageBuffer[channel][m_rows * m_width + x + i] =
                        data[x * m_channels + channel * m_interleavingSize + i + offset] * multiplier;
                }
            }
        }
    }

    set_height(m_rows + 1);
}
