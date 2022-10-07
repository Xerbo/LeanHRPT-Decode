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

#include "raw.h"

#include <cstring>

#include "protocol/repack.h"

RawImage::RawImage(size_t width, size_t channels, size_t chunk_size)
    : row_buffer(width * channels + 100), m_width(width), m_channels(channels), m_chunk_size(chunk_size), m_rows(0) {
    image_buffer.resize(m_channels);
    for (size_t i = 0; i < channels; i++) {
        image_buffer[i].resize(width * 2000);
    }
}

/// Process interleaved pixels into individual channels
void RawImage::process_line(const uint16_t *data, size_t ch, int offset, int multiplier) {
    for (size_t x = 0; x < m_width; x++) {
        image_buffer[ch][m_rows * m_width + x] = data[x * m_channels + ch + offset] * multiplier;
    }
}

/// Process interleaved chunks of pixels into individual channels
void RawImage::process_line_chunked(const uint16_t *data, size_t ch, int offset, int multiplier) {
    for (size_t x = 0; x < m_width; x += m_chunk_size) {
        for (size_t i = 0; i < m_chunk_size; i++) {
            image_buffer[ch][m_rows * m_width + x + i] = data[x * m_channels + ch * m_chunk_size + i + offset] * multiplier;
        }
    }
}

void RawImage::push10Bit(const uint8_t *data, int offset) {
    int byte_offset = offset / 4 * 5;  // Offset as close as possible by byte shifting
    int pixel_offset = offset % 4;     // Numbers of pixels to offset after byte shifting

    repack10(&data[byte_offset], row_buffer.data(), m_width * m_channels + pixel_offset);
    push16Bit(row_buffer.data(), pixel_offset, 64);
}

void RawImage::push16Bit(const uint16_t *data, int offset, int multiplier) {
    for (size_t ch = 0; ch < m_channels; ch++) {
        if (m_chunk_size == 1) {
            process_line(data, ch, offset, multiplier);
        } else {
            process_line_chunked(data, ch, offset, multiplier);
        }
    }

    set_height(m_rows + 1);
}
