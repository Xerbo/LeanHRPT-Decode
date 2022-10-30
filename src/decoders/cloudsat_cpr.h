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

#ifndef LEANHRPT_DECODERS_CLOUDSAT_CPR_H_
#define LEANHRPT_DECODERS_CLOUDSAT_CPR_H_

#include "decoder.h"
#include "protocol/deframer.h"

class CloudSatCPRDecoder : public Decoder {
   public:
    CloudSatCPRDecoder() : deframer(3, true) { images[Imager::CPR] = new RawImage(125, 1); }

   private:
    uint8_t buffer[402];
    uint8_t frame[402];
    ArbitraryDeframer<uint32_t, 0b00000011100100010101111011010011, 32, 3216> deframer;

    void work(std::istream &stream);
};

#endif
