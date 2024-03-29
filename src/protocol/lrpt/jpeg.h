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

#ifndef LEANHRPT_PROTOCOL_LRPT_JPEG_H_
#define LEANHRPT_PROTOCOL_LRPT_JPEG_H_

#include <array>
#include <cstdint>

namespace jpeg {
template <typename T>
using block = std::array<std::array<T, 8>, 8>;

void decode_block(const std::array<int16_t, 64> &in, jpeg::block<uint8_t> &out, uint8_t q);
}  // namespace jpeg

#endif
