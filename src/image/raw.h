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

#ifndef LEANHRPT_IMAGE_RAW_H_
#define LEANHRPT_IMAGE_RAW_H_

#include <cstdint>
#include <stdexcept>
#include <vector>

class RawImage {
   public:
    RawImage(size_t width, size_t channels, size_t chunk_size = 1);

    void push10Bit(const uint8_t *data, int offset = 0);
    void push16Bit(const uint16_t *data, int offset = 0, int multiplier = 1);

    uint16_t *getChannel(size_t channel) {
        if (channel < m_channels) {
            return image_buffer[channel].data();
        }
        throw std::runtime_error("Channel index out of range");
    }

    size_t width() { return m_width; }
    size_t channels() { return m_channels; }
    size_t rows() { return m_rows; }
    void set_height(size_t new_height) {
        m_rows = new_height;

        for (auto &channel : image_buffer) {
            if (channel.size() > m_rows * m_width) continue;
            channel.resize((m_rows + 2000) * m_width);
        }
    }

   private:
    std::vector<uint16_t> row_buffer;
    std::vector<std::vector<uint16_t>> image_buffer;

    void process_line(const uint16_t *data, size_t ch, int offset, int multiplier);
    void process_line_chunked(const uint16_t *data, size_t ch, int offset, int multiplier);

    size_t m_width;
    size_t m_channels;
    size_t m_chunk_size;
    size_t m_rows;
};

#endif
