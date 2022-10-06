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

#ifndef LEANHRPT_DECODERS_COMMON_AIP_H_
#define LEANHRPT_DECODERS_COMMON_AIP_H_

#include <cstring>
#include <map>

#include "amsu.h"
#include "protocol/rawimage.h"
#include "protocol/repack.h"
#include "satinfo.h"

class AIPDecoder {
   public:
    /**
     * Decode a MHS Line
     *
     * @return If a line was decoded
     */
    bool work(std::map<Imager, RawImage *> &images, const uint8_t *frame) {
        uint8_t frame_counter = frame[4];
        if (frame_counter >= 80) return false;

        std::memcpy(&mhsline[frame_counter * 50], &frame[48], 50);
        std::memcpy(&amsua1_line[frame_counter * 26], &frame[8], 26);
        std::memcpy(&amsua2_line[frame_counter * 14], &frame[34], 14);
        if (frame_counter == 79) {
            images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[198], 0);
            images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[1532], 0);
            images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[2864], 0);
            amsu_decoder.work(images, amsua1_line, amsua2_line);

            std::memset(mhsline, 0, 80 * 50);
            std::memset(amsua1_line, 0, 80 * 26);
            std::memset(amsua2_line, 0, 80 * 14);
            return true;
        }
        return false;
    }

   private:
    uint8_t amsua1_line[80 * 26];
    uint8_t amsua2_line[80 * 14];
    uint8_t mhsline[80 * 50];
    AMSUDecoder amsu_decoder;
};

#endif
