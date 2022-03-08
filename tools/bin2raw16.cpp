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

#include "protocol/deframer.h"
#include "protocol/repack.h"
#include "cli.h"

int main(int argc, char *argv[]) {
    std::ifstream in;
    std::ofstream out;
    interface("input.bin", "output.raw16", in, out, argc, argv);

    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> deframer(8, true);
    uint8_t buffer[1024];
    uint8_t frame[13863];
    uint16_t repacked[11090];
    while (!in.eof()) {
        in.read((char *)buffer, 1024);

        if (deframer.work(buffer, frame, 1024)) {
            repack10(frame, repacked, 11090);
            out.write((char *)repacked, 11090*sizeof(uint16_t));
        }
    }
}
