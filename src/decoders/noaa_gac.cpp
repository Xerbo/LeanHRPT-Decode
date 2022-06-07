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

#include "noaa_gac.h"
#include "common/tip.h"
#include <bitset>

// Contains 1023 bits of data, last bit is zero
uint8_t pattern[128] = {
	0x07, 0x40, 0xc9, 0x29, 0x42, 0x5e, 0xee, 0xa8, 0xee, 0x2c, 0x2f, 0xb1,
	0xf5, 0x24, 0x21, 0xc8, 0xe6, 0x60, 0x36, 0x6c, 0x5c, 0x79, 0x6f, 0x04,
	0x9c, 0xed, 0x36, 0xaa, 0xfd, 0x2a, 0x58, 0xdb, 0xa2, 0x77, 0x92, 0xd6,
	0x45, 0x20, 0x07, 0xc4, 0x08, 0xb4, 0x98, 0xcb, 0x3a, 0x44, 0x29, 0x84,
	0xff, 0xbd, 0x9f, 0x31, 0x12, 0xb5, 0x15, 0x89, 0x9c, 0x2b, 0x97, 0xf9,
	0xca, 0xf5, 0x66, 0x41, 0x06, 0x0b, 0x2a, 0xdc, 0x1a, 0x3f, 0xad, 0x07,
	0x02, 0xa9, 0xe7, 0xaf, 0x14, 0x04, 0xde, 0x8d, 0xf8, 0x47, 0xb7, 0xc0,
	0x2e, 0xb8, 0x76, 0x1f, 0x94, 0xe3, 0x4f, 0xb9, 0xb9, 0x3d, 0xfc, 0x61,
	0xbb, 0x2e, 0xfa, 0x16, 0xd1, 0x79, 0xa9, 0xa5, 0xcf, 0xda, 0xe9, 0x94,
	0x67, 0x8e, 0x24, 0x63, 0xa8, 0x28, 0x8d, 0x7c, 0x86, 0x2a, 0x1a, 0xbb,
	0x6c, 0x9a, 0xd8, 0x3c, 0x33, 0x43, 0xd3, 0xac
};

void NOAAGACDecoder::init_xor() {
    uint8_t buf = 0;
	size_t offset = 60;
	size_t bit = offset%8;
	size_t byte = offset/8;

	for (size_t i = 0; i < 33270-offset; i++) {
		size_t j = i % 1023;
		buf = buf << 1 | std::bitset<8>(pattern[j/8]).test(7 - (j%8));
		bit++;

		if (bit == 8) {
			xor_table[byte++] = buf;
			bit = 0;
		}
	}
}

void NOAAGACDecoder::work(std::istream &stream) {
    if (d_filetype == FileType::Raw) {
        stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
        if (deframer.work(buffer, frame, BUFFER_SIZE)) {
            for (size_t i = 0; i < 4159; i++) {
				frame[i] ^= xor_table[i];
			}

            repack10(frame, repacked, 3327-3);
            frame_work(repacked);
        }
    }
}

void NOAAGACDecoder::frame_work(uint16_t *ptr) {
    uint16_t *data = &ptr[103];
    bool line_ok = true;

    // Parse TIP/AIP frames
    for (size_t i = 0; i < 10; i++) {
        uint8_t frame[104];
        bool parity_ok = true;

        for (size_t j = 0; j < 104; j++) {
            uint16_t word = data[104*i + j];
            frame[j] = word >> 2;

            // Parity check
            bool parity = std::bitset<8>(frame[j]).count() % 2;
            bool expected_parity = std::bitset<16>(word).test(1);
            if (parity != expected_parity) {
                parity_ok = false;
            }
        }

        if (!parity_ok) {
            line_ok = false;
            continue;
        }

        if (i < 5) {
            if (tip_work(images, frame)) {
                timestamps[Imager::HIRS].push_back(timestamp);
            }
        } else {
            if (aip_decoder.work(images, frame)) {
                timestamps[Imager::MHS].push_back(0);
                timestamps[Imager::MHS].push_back(0);
                timestamps[Imager::MHS].push_back(timestamp);
            }
        }
    }

    // Calculate the timestamp of the start of the year
    int _year = QDateTime::fromSecsSinceEpoch(created).date().year();
    double year = QDate(_year, 1, 1).startOfDay(Qt::UTC).toSecsSinceEpoch() - 86400.0;

    // Parse timestamp
    uint16_t days = repacked[8] >> 1;
    uint32_t ms = (repacked[9] & 0b1111111) << 20 | repacked[10] << 10 | repacked[11];
    timestamp = (double)year + (double)days*86400.0 + (double)ms/1000.0;
    timestamps[Imager::AVHRR].push_back(line_ok ? timestamp : 0.0);

    for (size_t i = 0; i < 3327; i++) {
        ptr[i] *= 64;
    }
    images[Imager::AVHRR]->push16Bit(ptr, 1182);
    ch3a.push_back(false);
}
