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

#ifndef LEANHRPT_DECODERS_METOP_H
#define LEANHRPT_DECODERS_METOP_H

#include <cstdint>

#include "decoder.h"
#include "protocol/ccsds/demuxer.h"

class MetopHRPTDecoder : public Decoder {
    public:
        MetopHRPTDecoder() {
            images[Imager::AVHRR] = new RawImage(2048, 5);
            images[Imager::MHS] = new RawImage(90, 6);
            frame = new uint8_t[1024];
        }
        ~MetopHRPTDecoder() {
            delete[] frame;
        }
    private:
        uint8_t *frame;
        ccsds::SimpleDemuxer demux, mhs_demux;

        void work(std::istream &stream);
        void frame_work(uint8_t *ptr);
};

#endif
