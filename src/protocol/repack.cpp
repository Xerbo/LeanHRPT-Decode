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

#include "repack.h"

#include <bitset>

template <typename T, size_t N>
void arbitrary_repack(const uint8_t *in, T *out, size_t n) {
    size_t pos = 0;
    for (size_t i = 0; i < n; i++) {
        out[i] = 0;
        for (size_t j = pos; j < pos + N; j++) {
            bool bit = std::bitset<8>(in[j / 8]).test(7 - j % 8);
            out[i] = out[i] << 1 | bit;
        }
        pos += N;
    }
}

// HIRS
template void arbitrary_repack<uint16_t, 13>(const uint8_t *in, uint16_t *out, size_t n);
// CPR
template void arbitrary_repack<uint32_t, 20>(const uint8_t *in, uint32_t *out, size_t n);

void repack10(const uint8_t *in, uint16_t *out, size_t n) {
    size_t j = 0;
    for (size_t i = 0; i < n; i += 4) {
        // clang-format off
        out[i + 0] =  (in[j + 0] << 2)       | (in[j + 1] >> 6);
        out[i + 1] = ((in[j + 1] % 64) << 4) | (in[j + 2] >> 4);
        out[i + 2] = ((in[j + 2] % 16) << 6) | (in[j + 3] >> 2);
        out[i + 3] = ((in[j + 3] % 4 ) << 8) |  in[j + 4];
        j += 5;
        // clang-format on
    }
}
