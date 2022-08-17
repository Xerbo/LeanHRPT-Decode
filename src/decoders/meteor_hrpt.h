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

#include "decoder.h"
#include "protocol/ccsds/deframer.h"
#include "protocol/deframer.h"

// http://planet.iitp.ru/retro/index.php?lang=en&page_type=spacecraft&page=meteor_m_n2_structure_1
class MeteorHRPTDecoder : public Decoder {
   public:
    MeteorHRPTDecoder() : MSUMRDeframer(9, false), mtvza_deframer(4, false) {
        frame = new uint8_t[1024];
        msumrBuffer = new uint8_t[948];
        msumrFrame = new uint8_t[11850];
        images[Imager::MSUMR] = new RawImage(1572, 6, 4);
        images[Imager::MTVZA] = new RawImage(200, 30);
    }
    ~MeteorHRPTDecoder() {
        delete[] frame;
        delete[] msumrBuffer;
        delete[] msumrFrame;
    }

   private:
    uint8_t *frame, *msumrBuffer, *msumrFrame;
    ccsds::Deframer deframer;
    ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 11850 * 8> MSUMRDeframer;
    ArbitraryDeframer<uint32_t, 0xFB386A45, 32, 248 * 8> mtvza_deframer;
    double msumr_timestamp = 0.0;

    void work(std::istream &stream);
    void frame_work(uint8_t *ptr);
    void mtvza_work(uint8_t x, size_t offset, uint8_t *ptr);
};

#endif
