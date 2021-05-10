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

// Maybe consider dynamic allocation?
#define MAX_HEIGHT 7000

#include <vector>
#include <cstdint>
#include <stdexcept>

class RawImage {
    public:
        RawImage(size_t width, size_t channels, size_t interleaving_size);
        RawImage(size_t width, size_t channels);
        ~RawImage();

        void push10Bit(const uint8_t *data, int offset);
        void push16Bit(const uint16_t *data, int offset);

        unsigned short *getChannel(size_t channel) {
            if (channel < m_channels) {
                return imageBuffer[channel];
            }
            throw std::runtime_error("Channel index out of range");
        }

        size_t width() { return m_width; }
        size_t channels() { return m_channels; }
        size_t rows() { return m_rows; }
    private:
        unsigned short *rowBuffer;
        std::vector<unsigned short *> imageBuffer;

        size_t m_width;
        size_t m_channels;
        size_t m_interleavingSize;
        size_t m_rows;
};

#endif
