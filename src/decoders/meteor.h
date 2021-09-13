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

#include <iostream>

// http://planet.iitp.ru/index.php?lang=en&page_type=spacecraft&page=meteor_m_n2_structure_1
class MeteorDecoder : public Decoder {
    public:
        MeteorDecoder() : MSUMRDeframer(10, false) {
            frame = new uint8_t[1024];
            msumrBuffer = new uint8_t[948];
            msumrFrame = new uint8_t[11850];
            images[Imager::MSUMR] = new RawImage(1572, 6, 4);
        }
        ~MeteorDecoder() {
            delete[] frame;
            delete[] msumrBuffer;
            delete[] msumrFrame;
        }
    private:
        uint8_t *frame, *msumrBuffer, *msumrFrame;
        ccsds::Deframer deframer;
        ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 11850 * 8> MSUMRDeframer;

        void work(std::istream &stream) {
            if (is_ccsds_frames) {
                stream.read(reinterpret_cast<char *>(frame), 1024);
                frame_work(frame);
            } else {
                stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
                if (deframer.work(buffer, frame, BUFFER_SIZE)) {
                    frame_work(frame);
                }
            }
        }

        void frame_work(uint8_t *ptr) {
            // See Table 1 - Structure of a transport frame
            std::memcpy(&msumrBuffer[238*0], &ptr[ 23-1], 238);
            std::memcpy(&msumrBuffer[238*1], &ptr[279-1], 238);
            std::memcpy(&msumrBuffer[238*2], &ptr[535-1], 238);
            std::memcpy(&msumrBuffer[238*3], &ptr[791-1], 234);

            if (MSUMRDeframer.work(msumrBuffer, msumrFrame, 948)) {
                images[Imager::MSUMR]->push10Bit(&msumrFrame[50], 0);
            }
        }
};

#endif
