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

#include "derand.h"

#include <cstring>
#include <bitset>

namespace ccsds {
    Derand::Derand() {
        generateRandomTable();
    }

    // Polynomial is standard CCSDS, 1 + x3 + x5 + x7 + x8
    void Derand::generateRandomTable() {
        uint8_t shiftRegisiter = 0xff;
        std::memset(randomTable, 0, 1024);

        // 1024 bytes in a packet, 4 of which are the ASM
        for (int i = 4; i < 1024; i++) {
            for (int j = 0; j < 8; j++) {
                // Take latest bit from `shiftRegisiter` and put it into `randomTable`
                randomTable[i] = randomTable[i] << 1;
                randomTable[i] |= std::bitset<8>(shiftRegisiter).test(7);

                // Next bit in PN sequence is XOR of all polynomials
                bool next = std::bitset<8>(shiftRegisiter).test(7) ^
                            std::bitset<8>(shiftRegisiter).test(4) ^
                            std::bitset<8>(shiftRegisiter).test(2) ^
                            std::bitset<8>(shiftRegisiter).test(0);

                // Shift the shift regisiter
                shiftRegisiter = shiftRegisiter << 1;
                shiftRegisiter |= next;
            }
        }

#if 0
        // Print randomization table
        for (int i = 0; i < 1024; i++) {
            printf("%i: %X\n", i, randomTable[i]);
        }
#endif
    }

    void Derand::work(uint8_t *data, size_t len) {
        for(size_t i = 0; i < len; i++) {
            data[i] ^= randomTable[i];
        }
    }
}
