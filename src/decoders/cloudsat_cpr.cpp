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

#include "cloudsat_cpr.h"

#include "protocol/repack.h"
#include "util.h"

void CloudSatCPRDecoder::work(std::istream &stream) {
    stream.read(reinterpret_cast<char *>(buffer), 402);

    if (deframer.work(buffer, frame, 402)) {
        uint32_t line[126];
        arbitrary_repack<uint32_t, 20>(&frame[83], line, 126);

        uint16_t line16[126];
        for (size_t i = 0; i < 126; i++) {
            bool sign = line[i] >> 19;
            int val = line[i] & 0b1111111111111111111;
            val = sign ? (val - 524288) : val;
            line16[i] = (val + 524288) / 16;
        }

        images[Imager::CPR]->push16Bit(line16, 1);
    }
}
