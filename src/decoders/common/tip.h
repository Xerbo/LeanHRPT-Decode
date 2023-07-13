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

#include <bitset>
#include <map>
#include <iostream>
#include <iomanip>
#include "image/raw.h"
#include "protocol/repack.h"
#include "satinfo.h"

#ifndef LEANHRPT_DECODERS_COMMON_TIP_H_
#define LEANHRPT_DECODERS_COMMON_TIP_H_

/**
 * Check parity of a TIP frame
 *
 * @return If the parity was correct
 */
inline bool tip_parity(const uint8_t *frame) {
    bool ok = true;

    for (size_t i = 0; i < 6; i++) {
        size_t parity = 0;
        for (size_t j = 0; j < 17; j++) {
            parity += std::bitset<8>(frame[2 + i * 17 + j]).count();
        }

        // Remove last bit on block 5
        if (i == 5) {
            parity -= std::bitset<8>(frame[103]).test(0);
        }

        if ((parity % 2) != std::bitset<8>(frame[103]).test(5 - i)) {
            ok = false;
        }
    }

    return ok;
}

class TIPDecoder {
   public:
    /**
     * Decode a HIRS Line
     *
     * @return If a line was decoded
     */
    bool hirs_work(std::map<Imager, RawImage *> &images, const uint8_t *frame) {
	for (int i = 0; i < 104; ++i)
	    std::cout << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)(unsigned char)frame[i] << " ";
	std::cout << std::endl;
	// These are taken from the NOAA KLM Users Guide
        const size_t offsets[36] = {16, 17, 22, 23, 26, 27, 30, 31, 34, 35, 38, 39, 42, 43, 54, 55, 58, 59,
                                    62, 63, 66, 67, 70, 71, 74, 75, 78, 79, 82, 83, 84, 85, 88, 89, 92, 93};
        const size_t channels[20] = {1, 17, 2, 3, 13, 4, 18, 11, 19, 7, 8, 20, 10, 14, 6, 5, 15, 12, 16, 9};

        // Extract HIRS/4 data
        uint8_t packet[36];
        for (size_t i = 0; i < 36; i++) {
            packet[i] = frame[offsets[i]];
        }

        // Repack into 13 bit words
        uint16_t words[22];
        arbitrary_repack<uint16_t, 13>(packet, words, 22);

        uint8_t element_number = (words[1] >> 1) & 0b111111;
        if (element_number > 55) return false;

        for (size_t j = 0; j < 20; j++) {
            unsigned short *channel = images[Imager::HIRS]->getChannel(channels[j] - 1);

            bool sign_bit = words[j + 2] >> 12;
            int16_t val = words[j + 2] & 0b111111111111;
            val = sign_bit ? val : -val;

            channel[images[Imager::HIRS]->rows() * images[Imager::HIRS]->width() + element_number] = val * 8 + 32768;
        }

        // New line
        if (element_number == 55) {
            images[Imager::HIRS]->set_height(images[Imager::HIRS]->rows() + 1);
            return true;
        }
        return false;
    }

   private:
};

#endif
