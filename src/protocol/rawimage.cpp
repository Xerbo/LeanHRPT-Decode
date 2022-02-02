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

RawImage::RawImage(size_t width, size_t channels, size_t interleaving_size) 
    : rowBuffer(new unsigned short[width*channels + 4]),
      m_width(width),
      m_channels(channels),
      m_interleavingSize(interleaving_size),
      m_rows(0) {
    imageBuffer.resize(m_channels);
    for (size_t i = 0; i < channels; i++) {
        imageBuffer[i].resize(width * 5000);
    }
}
RawImage::RawImage(size_t width, size_t channels)
    : RawImage(width, channels, 1) { }

RawImage::~RawImage() {
    delete[] rowBuffer;
}

void RawImage::push10Bit(const uint8_t *data, int offset) {
    int j = offset/4 * 5; // Offset as close as we can get in just bytes
    int pxoffset = offset % 4; // Numbers of pixels to offset after byte shifting

    for (size_t i = 0; i < m_width*m_channels; i += 4) {
        rowBuffer[i + 0] =  (data[j + 0] << 2)       | (data[j + 1] >> 6);
        rowBuffer[i + 1] = ((data[j + 1] % 64) << 4) | (data[j + 2] >> 4);
        rowBuffer[i + 2] = ((data[j + 2] % 16) << 6) | (data[j + 3] >> 2);
        rowBuffer[i + 3] = ((data[j + 3] % 4 ) << 8) |  data[j + 4];
        j += 5;
    }

    // Scale 10 to 16 bit
    // 2^16 / 2^10 is 64, not 60...
    for (size_t i = 0; i < m_width*m_channels; i++) {
        rowBuffer[i] *= 64;
    }

    push16Bit(rowBuffer, pxoffset);
}
void RawImage::push16Bit(const uint16_t *data, int offset) {
    for (size_t channel = 0; channel < m_channels; channel++) {
        if (m_interleavingSize == 1) { 
            for (size_t x = 0; x < m_width; x++) {
                imageBuffer[channel][m_rows*m_width + x] = data[x*m_channels + channel + offset];
            }
        // Meteor just can't be normal can it
        } else {
            for (size_t x = 0; x < m_width; x += m_interleavingSize) {
                for (size_t i = 0; i < m_interleavingSize; i++) {
                    imageBuffer[channel][m_rows*m_width + x + i] = data[x*m_channels + channel*m_interleavingSize + i + offset];
                }
            }
        }
    }

    set_height(m_rows+1);
}
