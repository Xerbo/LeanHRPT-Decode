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

#include "packet.h"

#include <cstdint>
#include "huffman.h"

namespace lrpt {
	bool decode_packet(std::array<jpeg::block<uint8_t>, MCU_PER_PACKET> &out, const uint8_t *data, uint8_t q, size_t n) {
		if (n == 0) {
			return false;
		}

		// Huffman decompression
		std::array<std::array<int16_t, 64>, MCU_PER_PACKET> decompressed;
		if (!huffman_decode(data, decompressed, MCU_PER_PACKET, n)) {
			return false;
		}

		// Decode the blocks
		for (size_t i = 0; i < MCU_PER_PACKET; i++) {
			jpeg::decode_block(decompressed[i], out[i], q);
		}

		return true;
	}
}