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

#ifndef LEANHRPT_DECODERS_METEOR_LRPT_H_
#define LEANHRPT_DECODERS_METEOR_LRPT_H_

#include "decoder.h"
#include "protocol/ccsds/demuxer.h"

// http://planet.iitp.ru/retro/index.php?lang=en&page_type=spacecraft&page=meteor_m_n2_structure_2
class MeteorLRPTDecoder : public Decoder {
   public:
    MeteorLRPTDecoder() {
        // Allocate all 6 channels for correct channel labeling
        images[Imager::MSUMR] = new RawImage(1568, 6);
        timestamps[Imager::MSUMR].reserve(10000);
    }

   private:
    uint8_t frame[1024];
    ccsds::Demuxer demux;

    size_t counter_offset = 0;
    size_t start_offset = 0;
    size_t last_counter = 0;

    void work(std::istream &stream);
    void frame_work();
};

#endif
