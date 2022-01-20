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

#include "noaa_hrpt.h"

#include <string>
#include <ctime>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <bitset>
#include "protocol/repack.h"

void NOAADecoder::work(std::istream &stream) {
    if (d_filetype == FileType::raw16) {
        stream.read(reinterpret_cast<char *>(repacked), 11090*2);
        frame_work(repacked);
    } else if (d_filetype == FileType::HRP) {
        stream.read(reinterpret_cast<char *>(repacked), 11090*2);
        for (size_t i = 0; i < 11090; i++) {
            uint16_t x = repacked[i];
            repacked[i] = (x << 8) | (x >> 8);
        }
        frame_work(repacked);
    } else if (d_filetype == FileType::Raw) {
        stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
        if (deframer.work(buffer, frame, BUFFER_SIZE)) {
            size_t j = 0;
            for (size_t i = 0; i < 11090-3; i += 4) {
                repacked[i + 0] =  (frame[j + 0] << 2)       | (frame[j + 1] >> 6);
                repacked[i + 1] = ((frame[j + 1] % 64) << 4) | (frame[j + 2] >> 4);
                repacked[i + 2] = ((frame[j + 2] % 16) << 6) | (frame[j + 3] >> 2);
                repacked[i + 3] = ((frame[j + 3] % 4 ) << 8) |  frame[j + 4];
                j += 5;
            }
            frame_work(repacked);
        }
    }
}

void NOAADecoder::frame_work(uint16_t *ptr) {
    uint16_t *data = &ptr[103];
    bool line_ok = true;

    // Parse TIP/AIP frames
    for (size_t i = 0; i < 5; i++) {
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

        uint8_t frame_type = (ptr[6] >> 7) & 0b11;
        switch (frame_type) {
            case 1: tip_work(frame); break;
            case 3: aip_work(frame); break;
            default:                 break;
        }
    }

    // Extract calibration data
    if (ptr[17] == ptr[18] && ptr[18] == ptr[19] && ptr[17] != 0) {
        caldata["prt"] += ptr[17];
        caldata["prtn"] += 1.0;
    }
    for (size_t i = 0; i < 5; i++) {
        double sum = 0.0;
        for (size_t x = 0; x < 10; x++) {
            sum += ptr[52 + x*5 + i];
        }

        caldata["ch" + std::to_string(i+1) + "_space"] += sum/10.0;
    }
    for (size_t i = 0; i < 3; i++) {
        double sum = 0.0;
        for (size_t x = 0; x < 10; x++) {
            sum += ptr[22 + x*3 + i];
        }

        caldata["ch" + std::to_string(i+3) + "_cal"] += sum/10.0;
    }

    // Calculate the timestamp of the start of the year
    int _year = QDateTime::fromSecsSinceEpoch(created).date().year();
    double year = QDate(_year, 1, 1).startOfDay(Qt::UTC).toSecsSinceEpoch() - 86400.0;

    // Parse timestamp
    uint16_t days = repacked[8] >> 1;
    uint32_t ms = (repacked[9] & 0b1111111) << 20 | repacked[10] << 10 | repacked[11];
    double timestamp = (double)year + (double)days*86400.0 + (double)ms/1000.0;
    if (line_ok) {
        timestamps[Imager::AVHRR].push_back(timestamp);
    } else {
        timestamps[Imager::AVHRR].push_back(0.0);
    }

    for (size_t i = 0; i < 11090; i++) {
        ptr[i] *= 64;
    }
    images[Imager::AVHRR]->push16Bit(ptr, 750);
}

void NOAADecoder::aip_work(const uint8_t *frame) {
    uint8_t mhs_status = frame[7];
    if (mhs_status > 80) return;

    std::memcpy(&mhsline[mhs_status*50], &frame[48], 50);
    if (mhs_status == 79) {
        images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[  98], 0);
        images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[1432], 0);
        images[Imager::MHS]->push16Bit((uint16_t *)&mhsline[2764], 0);
        std::memset(mhsline, 0, 80*50);
    }
}

void NOAADecoder::tip_work(const uint8_t *frame) {
    // These are taken from the NOAA KLM Users Guide
    const size_t offsets[36] = { 16, 17, 22, 23, 26, 27, 30, 31, 34, 35, 38, 39, 42, 43, 54, 55, 58, 59, 62, 63, 66, 67, 70, 71, 74, 75, 78, 79, 82, 83, 84, 85, 88, 89, 92, 93 };
    const size_t channels[20] = { 1, 17, 2, 3, 13, 4, 18, 11, 19, 7, 8, 20, 10, 14, 6, 5, 15, 12, 16, 9 };

    // Extract HIRS/4 data
    uint8_t packet[36];
    for (size_t i = 0; i < 36; i++) {
        packet[i] = frame[offsets[i]];
    }

    // Repack into 13 bit words
    uint16_t words[22];
    arbitrary_repack<uint16_t, 13>(packet, words, 22);

    uint8_t element_number = (words[1] >> 1) & 0b111111;
    if (element_number > 55) return;

    for (size_t j = 0; j < 20; j++) {
        unsigned short *channel = images[Imager::HIRS]->getChannel(channels[j]-1);

        bool sign_bit = words[j+2] >> 12;
        int16_t val = words[j+2] & 0b111111111111;
        val = sign_bit ? val : -val;

        channel[images[Imager::HIRS]->rows()*images[Imager::HIRS]->width() + element_number] = val*8 + 32768;
    }

    // New line
    if (element_number == 55) {
        images[Imager::HIRS]->set_height(images[Imager::HIRS]->rows() + 1);
    }
}
