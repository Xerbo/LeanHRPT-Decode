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

#ifndef LEANHRPT_PROTOCOL_LRPT_PACKET_H_
#define LEANHRPT_PROTOCOL_LRPT_PACKET_H_

#include <cstddef>
#include <cstdint>

#include "jpeg.h"

#define MCU_PER_PACKET 14

namespace lrpt {
bool decode_packet(std::array<jpeg::block<uint8_t>, MCU_PER_PACKET> &out, const uint8_t *data, uint8_t q, size_t n);
}

#endif
