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

#ifndef LEANHRPT_PROTOCOL_REPACK_H_
#define LEANHRPT_PROTOCOL_REPACK_H_

#include <cstddef>
#include <cstdint>

template <typename T, size_t N>
void arbitrary_repack(const uint8_t *in, T *out, size_t n);

void repack10(const uint8_t *in, uint16_t *out, size_t n);

#endif
