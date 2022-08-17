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

#include <map>

#include "protocol/rawimage.h"
#include "protocol/repack.h"
#include "satinfo.h"

#ifndef AIP_H
#define AIP_H

#include <cstring>

class AIPDecoder {
   public:
    bool work(std::map<Imager, RawImage *> &images, const uint8_t *frame) {
        uint8_t mhs_status = frame[7];
        if (mhs_status >= 80) return false;

        std::memcpy(&mhsline[mhs_status * 50], &frame[48], 50);
        if (mhs_status == 79) {
            images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[98], 0);
            images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[1432], 0);
            images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[2764], 0);
            std::memset(mhsline, 0, 80 * 50);
            return true;
        }
        return false;
    }

   private:
    uint8_t mhsline[80 * 50];
};

#endif
