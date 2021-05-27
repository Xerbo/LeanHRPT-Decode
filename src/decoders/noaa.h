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

#ifndef LEANHRPT_DECODERS_NOAA_H
#define LEANHRPT_DECODERS_NOAA_H

#include <string>
#include <cstdint>

#include "decoder.h"
#include "generic/deframer.h"

class NOAADecoder : public Decoder {
    public:
        NOAADecoder() : deframer(10, true) {
            frame = new uint8_t[(11090*10) / 8];
            image = new RawImage(2048, 5);
        }
        ~NOAADecoder() {
            delete[] frame;
        }
        void work() {
            if(deframer.work(buffer, frame, BUFFER_SIZE)) {
                image->push10Bit(frame, 750);
            }
        }
        std::string imagerName() {
            return "AVHRR";
        }
    private:
        uint8_t *frame;
        ArbitraryDeframer<uint64_t, 0xA116FD719D8CC950, 64, 11090 * 10> deframer;
};

#endif
