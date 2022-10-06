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

int main(int argc, char *argv[]) {
    std::ifstream in;
    std::ofstream out;
    interface("input.vcdu", "output.cadu", in, out, argc, argv);

    uint8_t frame[1024] = {0x1A, 0xCF, 0xFC, 0x1D};
    while (!in.eof()) {
        in.read((char *)&frame[4], 892);
        out.write((char *)frame, 1024);
    }
}
