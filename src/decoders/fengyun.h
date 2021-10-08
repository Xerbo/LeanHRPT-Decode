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
        FengyunDecoder() : virrDeframer(8, false) {
            frame = new uint8_t[1024];
            line = new uint8_t[208400 / 8];
            images[Imager::VIRR] = new RawImage(2048, 10);
        }
        ~FengyunDecoder() {
            delete[] frame;
            delete[] line;
        }
    private:
        uint8_t *frame, *line;
        ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 208400> virrDeframer;
        ccsds::Deframer deframer;
        ccsds::Derand derand;
        SatHelper::ReedSolomon reedSolomon;

        void work(std::istream &stream) {
            if (d_filetype == FileType::CADU) {
                stream.read(reinterpret_cast<char *>(frame), 1024);
                frame_work(frame);
            } else {
                stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
                if (deframer.work(buffer, frame, BUFFER_SIZE)) {
                    derand.work(frame, 1024);
                    reedSolomon.decode_intreleaved_ccsds(frame);
                    frame_work(frame);
                }
            }
        }

        void frame_work(uint8_t *ptr) {
            uint8_t VCID = ptr[5] & 0x3f; // 0b111111
            if (VCID == 5) {
                if (virrDeframer.work(&ptr[14], line, 882)) {
                    images[Imager::VIRR]->push10Bit(line, 349);
                }
            }
        }
};

#endif
