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

#ifndef LEANHRPT_DECODERS_METEOR_H
#define LEANHRPT_DECODERS_METEOR_H

#include <cstdint>
#include <cstring>

#include "decoder.h"
#include "ccsds/deframer.h"
#include "generic/deframer.h"

class MeteorDecoder : public Decoder {
    public:
        MeteorDecoder() : MSUMRDeframer(10, false) {
            frame = new uint8_t[1024];
            msumrBuffer = new uint8_t[948];
            msumrFrame = new uint8_t[11850];
            image = new RawImage(1572, 6, 4);
        }
        ~MeteorDecoder() {
            delete[] frame;
            delete[] msumrBuffer;
            delete[] msumrFrame;
        }
        void work() {
            if (deframer.work(buffer, frame, BUFFER_SIZE)) {
                // See Table 1 - Structure of a transport frame
                std::memcpy(&msumrBuffer[238*0], &frame[ 23-1], 238);
                std::memcpy(&msumrBuffer[238*1], &frame[279-1], 238);
                std::memcpy(&msumrBuffer[238*2], &frame[535-1], 238);
                std::memcpy(&msumrBuffer[238*3], &frame[791-1], 234);

                if (MSUMRDeframer.work(msumrBuffer, msumrFrame, 948)) {
                    image->push10Bit(&msumrFrame[50], 0);
                }
            }
        }
        std::string imagerName() {
            return "MSU-MR";
        }
    private:
        uint8_t *frame, *msumrBuffer, *msumrFrame;
        ccsds::Deframer deframer;
        ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 11850 * 8> MSUMRDeframer;
};

#endif
