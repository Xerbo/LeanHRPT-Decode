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

#include "jpeg.h"

#include <cmath>
#include <cstddef>

namespace jpeg {
// clang-format off
	/// IJG standard (Q=50) quantization table
	const int jpeg_qtable[8][8] = {
		{ 16, 11, 10, 16, 24,  40,  51,  61  },
		{ 12, 12, 14, 19, 26,  58,  60,  55  },
		{ 14, 13, 16, 24, 40,  57,  69,  56  },
		{ 14, 17, 22, 29, 51,  87,  80,  62  },
		{ 18, 22, 37, 56, 68,  109, 103, 77  },
		{ 24, 35, 55, 64, 81,  104, 113, 92  },
		{ 49, 64, 78, 87, 103, 121, 120, 101 },
		{ 72, 92, 95, 98, 112, 100, 103, 99  }
	};

	/// 8x8 JPEG zigzag pattern
	const int jpeg_zigzag[8][8] = {
		{  0,  1,  5,  6, 14, 15, 27, 28 },
		{  2,  4,  7, 13, 16, 26, 29, 42 },
		{  3,  8, 12, 17, 25, 30, 41, 43 },
		{  9, 11, 18, 24, 31, 40, 44, 53 },
		{ 10, 19, 23, 32, 39, 45, 52, 54 },
		{ 20, 22, 33, 38, 46, 51, 55, 60 },
		{ 21, 34, 37, 47, 50, 56, 59, 61 },
		{ 35, 36, 48, 49, 57, 58, 62, 63 }
	};

	/**
	 * Look up table for values of cosine needed for the 8x8 IDCT, generated with:
     *
	 * ```
	 * void init_cosine_lut() {
	 *     for (size_t y = 0; y < 8; y++) {
	 *         for (size_t x = 0; x < 8; x++) {
	 *             // https://en.wikipedia.org/wiki/JPEG#Decoding
	 *             cosine_lut[y][x] = cosf(((2.0f*y + 1.0f) * x * M_PI) / 16.0f);
	 *         }
     *         
	 *         // Normalizing factor
	 *         cosine_lut[y][0] /= sqrtf(2.0);
	 *     }
	 * }
	 * ```
	 */
	const float cosine_lut[8][8] = {
		{ 0.707107f,  0.980785f, 0.92388f,   0.83147f,   0.707107f,  0.55557f,   0.382683f,  0.19509f  },
		{ 0.707107f,  0.83147f,  0.382683f, -0.19509f,  -0.707107f, -0.980785f, -0.92388f,  -0.55557f, },
		{ 0.707107f,  0.55557f, -0.382683f, -0.980785f, -0.707107f,  0.19509f,   0.92388f,   0.83147f  },
		{ 0.707107f,  0.19509f, -0.92388f,  -0.55557f,   0.707107f,  0.83147f,  -0.382683f, -0.980785f },
		{ 0.707107f, -0.19509f, -0.92388f,   0.55557f,   0.707107f, -0.83147f,  -0.382684f,  0.980785f },
		{ 0.707107f, -0.55557f, -0.382684f,  0.980785f, -0.707107f, -0.19509f,   0.92388f,  -0.83147f  },
		{ 0.707107f, -0.83147f,  0.382684f,  0.195091f, -0.707107f,  0.980785f, -0.923879f,  0.55557f  },
		{ 0.707107f, -0.980785f, 0.92388f,  -0.83147f,   0.707107f, -0.55557f,   0.382684f, -0.19509f  }
	};
// clang-format on

static void unzigzag(const std::array<int16_t, 64> &in, jpeg::block<int16_t> &out) {
    for (size_t y = 0; y < 8; y++) {
        for (size_t x = 0; x < 8; x++) {
            int pos = jpeg_zigzag[y][x];
            out[y][x] = in[pos];
        }
    }
}

// See https://web.archive.org/web/20150223223556/http://meteor.robonuka.ru/for-experts/soft/
static int qfactor(size_t x, size_t y, uint8_t q) {
    float f;
    if (q < 50) {
        f = 5000.0f / (float)q;
    } else {
        f = 200.0f - 2.0f * (float)q;
    }

    int ptk = roundf(f / 100.0f * (float)jpeg_qtable[y][x]);
    if (ptk > 1) {
        return ptk;
    } else {
        return 1;
    }
}

static void dequantize(jpeg::block<int16_t> &block, uint8_t q) {
    for (size_t y = 0; y < 8; y++) {
        for (size_t x = 0; x < 8; x++) {
            block[y][x] *= qfactor(x, y, q);
        }
    }
}

/// 2D 8x8 inverse DCT
static void idct(const jpeg::block<int16_t> &in, jpeg::block<uint8_t> &out) {
    for (size_t y = 0; y < 8; y++) {
        for (size_t x = 0; x < 8; x++) {
            float sum = 0.0;

            for (size_t i = 0; i < 8; i++) {
                for (size_t j = 0; j < 8; j++) {
                    sum += in[j][i] * cosine_lut[x][i] * cosine_lut[y][j];
                }
            }

            sum = sum / 4.0f + 128.0f;
            out[y][x] = std::min(std::max(sum, 0.0f), 255.0f);
        }
    }
}

void decode_block(const std::array<int16_t, 64> &in, jpeg::block<uint8_t> &out, uint8_t q) {
    if (q < 20 || q > 100) {
        return;
    }

    jpeg::block<int16_t> tmp;
    unzigzag(in, tmp);
    dequantize(tmp, q);
    idct(tmp, out);
}
}  // namespace jpeg
