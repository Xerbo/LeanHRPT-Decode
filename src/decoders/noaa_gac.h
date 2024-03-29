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

#ifndef LEANHRPT_DECODERS_NOAA_GAC_H_
#define LEANHRPT_DECODERS_NOAA_GAC_H_

#include "common/aip.h"
#include "common/tip.h"
#include "decoder.h"
#include "protocol/deframer.h"

class NOAAGACDecoder : public Decoder {
   public:
    NOAAGACDecoder(bool reverse) : d_reverse(reverse), deframer(8, true), deframer_reverse(8, true) {
        images[Imager::AVHRR] = new RawImage(409, 5);
        images[Imager::HIRS] = new RawImage(56, 20);
        images[Imager::MHS] = new RawImage(90, 6);
        images[Imager::AMSUA] = new RawImage(30, 15);
        init_xor();
    }

   private:
    const bool d_reverse;
    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 33270> deframer;
    ArbitraryDeframer<uint64_t, 0b010011001111000011111001001010011011001001001000101010011110, 60, 33270> deframer_reverse;
    double timestamp = 0.0;
    double blackbody_temperature = 290;

    uint8_t frame[4159];
    uint16_t repacked[3327];

    uint8_t xor_table[4159];
    void init_xor();

    void work(std::istream &stream);
    void frame_work(uint16_t *ptr);

    AIPDecoder aip_decoder;
    TIPDecoder tip_decoder;
};

#endif
