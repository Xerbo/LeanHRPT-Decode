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

#ifndef LEANHRPT_DECODERS_NOAA_DSB_H_
#define LEANHRPT_DECODERS_NOAA_DSB_H_

#include "common/tip.h"
#include "decoder.h"

class NOAADSBDecoder : public Decoder {
   public:
    NOAADSBDecoder() { images[Imager::HIRS] = new RawImage(56, 20); }

   private:
    void work(std::istream &stream) {
        uint8_t frame[104];
        if (d_filetype == FileType::TIP) {
            stream.read((char *)frame, 104);
            tip_work(images, frame);
        }
    }
};

#endif
