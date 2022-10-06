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

#include "cli.h"
#include "protocol/ccsds/deframer.h"

int main(int argc, char *argv[]) {
    std::ifstream in;
    std::ofstream out;
    interface("input.bin", "output.cadu", in, out, argc, argv);

    ccsds::Deframer deframer;
    uint8_t buffer[1024];
    uint8_t frame[1024];
    while (!in.eof()) {
        in.read((char *)buffer, 1024);

        if (deframer.work(buffer, frame, 1024)) {
            out.write((char *)frame, 1024);
        }
    }
}
