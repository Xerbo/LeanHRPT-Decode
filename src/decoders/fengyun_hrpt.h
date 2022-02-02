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
#include "protocol/deframer.h"

class FengyunHRPTDecoder : public Decoder {
    public:
        FengyunHRPTDecoder(SatID sat) : virrDeframer(8, false) {
            frame = new uint8_t[1024];
            line = new uint8_t[208400 / 8];
            images[Imager::VIRR] = new RawImage(2048, 10);

            switch (sat) {
                case SatID::FengYun3A: launch_timestamp = 1289347200.0; break;
                case SatID::FengYun3B: launch_timestamp = 1289347200.0; break;
                case SatID::FengYun3C: launch_timestamp = 1522368000.0; break;
                default: throw std::runtime_error("Non FY SatID passed to FengyunHRPTDecoder");
            }
        }
        ~FengyunHRPTDecoder() {
            delete[] frame;
            delete[] line;
        }
    private:
        uint8_t *frame, *line;
        ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 208400> virrDeframer;
        double launch_timestamp;

        void work(std::istream &stream);
        void frame_work(uint8_t *ptr);
};

#endif
