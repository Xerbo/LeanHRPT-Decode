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

#ifndef LEANHRPT_GENERIC_RAWIMAGE_H
#define LEANHRPT_GENERIC_RAWIMAGE_H

#include <cstdint>
#include <stdexcept>
#include <vector>

class RawImage {
   public:
    RawImage(size_t width, size_t channels, size_t interleaving_size = 1);
    ~RawImage();

    void push10Bit(const uint8_t *data, int offset);
    void push16Bit(const uint16_t *data, int offset);

    unsigned short *getChannel(size_t channel) {
        if (channel < m_channels) {
            return imageBuffer[channel].data();
        }
        throw std::runtime_error("Channel index out of range");
    }

    size_t width() { return m_width; }
    size_t channels() { return m_channels; }
    size_t rows() { return m_rows; }
    void set_height(size_t new_height) {
        m_rows = new_height;

        for (auto &channel : imageBuffer) {
            if (channel.size() <= m_rows * m_width) {
                channel.resize((m_rows + 1000) * m_width);
            }
        }
    }

   private:
    unsigned short *rowBuffer;
    std::vector<std::vector<unsigned short>> imageBuffer;

    size_t m_width;
    size_t m_channels;
    size_t m_interleavingSize;
    size_t m_rows;
};

#endif
