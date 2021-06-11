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

#ifndef LEANHRPT_DECODERS_FENGYUN_H
#define LEANHRPT_DECODERS_FENGYUN_H

#include <cstdint>

#include "decoder.h"
#include "generic/deframer.h"
#include "ccsds/derand.h"
#include "ccsds/reedsolomon.h"
#include "ccsds/deframer.h"

class FengyunDecoder : public Decoder {
    public:
        FengyunDecoder() : virrDeframer(10, false) {
            frame = new uint8_t[1024];
            line = new uint8_t[208400 / 8];
            image = new RawImage(2048, 10);
        }
        ~FengyunDecoder() {
            delete[] frame;
            delete[] line;
        }
        void work() {
            if (buffer[0] == 0x1A && buffer[1] == 0xCF && buffer[2] && 0xFC && buffer[3] == 0x1D) {
                uint8_t VCID = buffer[5] & 0x3f; // 0b111111
                if (VCID == 5) {
                    if (virrDeframer.work(&buffer[14], line, 882)) {
                        image->push10Bit(line, 349);
                    }
                }
            } else {
                if (deframer.work(buffer, frame, BUFFER_SIZE)) {
                    derand.work(frame, 1024);
                    reedSolomon.decode_intreleaved_ccsds(frame);

                    uint8_t VCID = frame[5] & 0x3f; // 0b111111
                    if (VCID == 5) {
                        if (virrDeframer.work(&frame[14], line, 882)) {
                            image->push10Bit(line, 349);
                        }
                    }
                }
            }
        }
        std::string imagerName() {
            return "VIRR";
        }
    private:
        uint8_t *frame, *line;
        ArbitraryDeframer<uint64_t, 0xA116FD719D8CC950, 64, 208400> virrDeframer;
        ccsds::Deframer deframer;
        ccsds::Derand derand;
        SatHelper::ReedSolomon reedSolomon;
};

#endif
