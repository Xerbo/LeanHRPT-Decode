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

#pragma once

#include <cstdint>
#include <vector>

namespace ccsds {
    class Demuxer {
        public:
            Demuxer(unsigned int packetDataLength = 882);
            std::vector<uint8_t> work(uint8_t *in);
        private:
            unsigned int dataLength;
            bool writingData = false;

            std::vector<uint8_t> packetBuffer;
            std::vector<uint8_t> packet;
    };
}
