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

#include "huffman.h"

#include <bitset>
#include <stdexcept>

/// Provides bit level access to an array
class BitArray {
   public:
    BitArray(const uint8_t *data, size_t size = SIZE_MAX) : d_data(data), d_size(size * 8) {}

    /// Get `n` bits
    uint32_t peek(size_t n) {
        if (d_pos + n > d_size) {
            throw std::out_of_range("BitArray: read past array boundary");
        }
        uint32_t result = 0;
        for (size_t i = 0; i < n; i++) {
            size_t x = d_pos + i;

            bool bit = std::bitset<8>(d_data[x / 8]).test(7 - x % 8);
            result = result << 1 | bit;
        }
        return result;
    }

    /// Seek `n` bits forward
    void advance(size_t n) { d_pos += n; }

    /// Get `n` bits and see forward
    uint32_t fetch(size_t n) {
        uint32_t result = peek(n);
        advance(n);
        return result;
    }

   private:
    const uint8_t *d_data;
    const size_t d_size;
    size_t d_pos = 0;
};

// clang-format off
const uint8_t dc_category_len[12] = {
    2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9
};
const uint8_t ac_table_size[17] = {
    0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125
};
const uint8_t ac_table[] = {
	1, 2,
	3,
	0, 4, 17,
	5, 18, 33,
	49, 65,
	6, 19, 81, 97,
	7, 34, 113,
	20, 50, 129, 145, 161,
	8, 35, 66, 177, 193,
	21, 82, 209, 240,
	36, 51, 98, 114,
	130,
	9, 10, 22, 23, 24, 25, 26, 37, 38, 39, 40, 41, 42, 52, 53, 54, 55, 56, 57,
	58, 67, 68, 69, 70, 71, 72, 73, 74, 83, 84, 85, 86, 87, 88, 89, 90, 99, 100,
	101, 102, 103, 104, 105, 106, 115, 116, 117, 118, 119, 120, 121, 122, 131,
	132, 133, 134, 135, 136, 137, 138, 146, 147, 148, 149, 150, 151, 152, 153,
	154, 162, 163, 164, 165, 166, 167, 168, 169, 170, 178, 179, 180, 181, 182,
	183, 184, 185, 186, 194, 195, 196, 197, 198, 199, 200, 201, 202, 210, 211,
	212, 213, 214, 215, 216, 217, 218, 225, 226, 227, 228, 229, 230, 231, 232,
	233, 234, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250
};
// clang-format on

int16_t apply_sign(uint16_t x, uint8_t category) {
    int16_t max = (1 << category) - 1;

    if (std::bitset<16>(x)[category - 1]) {
        return x;
    } else {
        return (int16_t)x - max;
    }
}

int get_dc_category(uint16_t word) {
    if (word >> 14 == 0x0) return 0;
    switch (word >> 13) {
        case 0x2:
            return 1;
        case 0x3:
            return 2;
        case 0x4:
            return 3;
        case 0x5:
            return 4;
        case 0x6:
            return 5;
    }
    if (word >> 12 == 0xE) return 6;
    if (word >> 11 == 0x1E) return 7;
    if (word >> 10 == 0x3E) return 8;
    if (word >> 9 == 0x7E) return 9;
    if (word >> 8 == 0xFE) return 10;
    if (word >> 7 == 0x1FE) return 11;
    return -1;
}

#define DECOMPRESS(x) apply_sign(b.fetch(x), x)

// TODO: exit if reading past the boundary of `in`
bool huffman_decode(const uint8_t *in, std::array<std::array<int16_t, 64>, MCU_PER_PACKET> &out, size_t n, size_t size) {
    BitArray b(in, size);
    int16_t dc = 0;

    for (size_t i = 0; i < n; i++) {
        // Extract the DC category
        int dc_category = get_dc_category(b.peek(16));
        if (dc_category == -1) return false;
        b.advance(dc_category_len[dc_category]);

        // Decompress the DC coefficient
        dc += DECOMPRESS(dc_category);
        out[i][0] = dc;

        // Decompress the AC coefficients
        for (size_t j = 1; j < 64; j++) {
            uint8_t ac_len = 1;
            uint8_t ac_run = 0;
            uint8_t ac_category = 0;

            // Extract the AC code
            // This is adapted from https://github.com/dbdexter-dev/meteor_decode/blob/master/jpeg/huffman.c#L89-L100
            {
                uint16_t first_coeff = 0;
                size_t ac_idx = 0;
                uint32_t ac_buf = b.peek(32);
                for (; ac_len < 17; ac_len++) {
                    uint32_t word = ac_buf >> (32 - ac_len);

                    // If the coefficient belongs to this range, decompress it
                    if (word - first_coeff < ac_table_size[ac_len]) {
                        uint8_t ac_info = ac_table[ac_idx + word - first_coeff];
                        ac_run = ac_info >> 4;
                        ac_category = ac_info & 0x0F;
                        break;
                    }

                    first_coeff = (first_coeff + ac_table_size[ac_len]) << 1;
                    ac_idx += ac_table_size[ac_len];
                }
            }

            b.advance(ac_len);

            if (ac_run == 0 && ac_category == 0) {
                // Fill the rest of this block
                for (; j < 64; j++) {
                    out[i][j] = 0;
                }
            } else {
                // Sanity check
                if (j + ac_run >= 64) return false;

                // The actual decompression
                for (size_t x = 0; x < ac_run; x++) {
                    out[i][j++] = 0;
                }
                out[i][j] = DECOMPRESS(ac_category);
            }
        }
    }

    return true;
}
