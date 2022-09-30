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

#ifndef LEANHRPT_DECODERS_COMMON_AMSU_H_
#define LEANHRPT_DECODERS_COMMON_AMSU_H_

#define AMSU_FILL_WORD 0x0001

#include <map>

#include "protocol/rawimage.h"
#include "protocol/repack.h"
#include "satinfo.h"

class AMSUDecoder {
   public:
    void work(std::map<Imager, RawImage *> &images, const uint8_t *amsua1, const uint8_t *amsua2) {
        uint16_t scene[30 * 15];
        uint16_t words[40 * 26];

        removefill(amsua1, words, 40 * 26);
        for (size_t x = 0; x < 30; x++) {
            for (size_t c = 0; c < 13; c++) {
                scene[x * 15 + c + 2] = words[8 + x * 17 + c];
            }
        }

        removefill(amsua2, words, 40 * 14);
        for (size_t x = 0; x < 30; x++) {
            for (size_t c = 0; c < 2; c++) {
                scene[x * 15 + c] = words[6 + x * 4 + c];
            }
        }

        images[Imager::AMSUA]->push16Bit(scene, 0);
    }

    /// Removes fill words from AMSU frames
    void removefill(const uint8_t *in, uint16_t *out, size_t n) {
        size_t j = 0;
        for (size_t i = 0; i < n; i++) {
            uint16_t word = in[i * 2] << 8 | in[i * 2 + 1];

            if (word != AMSU_FILL_WORD) {
                out[j++] = word;
            }
        }
    }

   private:
};

#endif
