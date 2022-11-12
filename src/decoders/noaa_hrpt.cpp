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

#include "noaa_hrpt.h"

#include <bitset>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>

#include "protocol/repack.h"

void NOAAHRPTDecoder::work(std::istream &stream) {
    if (d_filetype == FileType::raw16) {
        stream.read(reinterpret_cast<char *>(repacked), 11090 * 2);
        frame_work(repacked);
    } else if (d_filetype == FileType::HRP) {
        stream.read(reinterpret_cast<char *>(repacked), 11090 * 2);
        for (size_t i = 0; i < 11090; i++) {
            uint16_t x = repacked[i];
            repacked[i] = (x << 8) | (x >> 8);
        }
        frame_work(repacked);
    } else if (d_filetype == FileType::Raw) {
        stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
        if (deframer.work(buffer, frame, BUFFER_SIZE)) {
            repack10(frame, repacked, 11090 - 3);
            frame_work(repacked);
        }
    }
}

void NOAAHRPTDecoder::frame_work(uint16_t *ptr) {
    uint16_t *data = &ptr[103];
    // bool line_ok = true;

    // Parse TIP/AIP frames
    for (size_t i = 0; i < 5; i++) {
        uint8_t frame[104];
        bool parity_ok = true;

        for (size_t j = 0; j < 104; j++) {
            uint16_t word = data[104 * i + j];
            frame[j] = word >> 2;

            // Parity check
            bool parity = std::bitset<8>(frame[j]).count() % 2;
            bool expected_parity = std::bitset<16>(word).test(1);
            if (parity != expected_parity) {
                parity_ok = false;
            }
        }

        if (!parity_ok) {
            // line_ok = false;
            continue;
        }

        uint8_t frame_type = (ptr[6] >> 7) & 0b11;
        switch (frame_type) {
            case 1: {
                if (tip_decoder.hirs_work(images, frame)) {
                    timestamps[Imager::HIRS].push_back(timestamp);
                }
                break;
            }
            case 3: {
                if (aip_decoder.work(images, frame)) {
                    timestamps[Imager::MHS].push_back(0);
                    timestamps[Imager::MHS].push_back(0);
                    timestamps[Imager::MHS].push_back(timestamp);
                    timestamps[Imager::AMSUA].push_back(timestamp);
                };
                break;
            }
            default:
                break;
        }
    }

    // Extract calibration data
    uint16_t *prt = &ptr[17];
    if (prt[0] != 0) {
        double sum = 0.0;
        for (size_t i = 0; i < 3; i++) {
            // TODO: calibrate each PRT separately
            // Currently this uses the average of all 4 coefficients (excluding d2)
            sum += 276.57465 + prt[i] * 0.050912;
        }

        blackbody_temperature = sum / 3.0;
    }
    caldata["blackbody_temperature_sum"] += blackbody_temperature;

    for (size_t i = 0; i < 5; i++) {
        double sum = 0.0;
        for (size_t x = 0; x < 10; x++) {
            sum += ptr[52 + x * 5 + i];
        }

        caldata["ch" + std::to_string(i + 1) + "_space"] += sum / 10.0;
    }

    for (size_t i = 0; i < 3; i++) {
        double sum = 0.0;
        for (size_t x = 0; x < 10; x++) {
            sum += ptr[22 + x * 3 + i];
        }

        caldata["ch" + std::to_string(i + 3) + "_cal"] += sum / 10.0;
    }
    caldata["n"] += 1.0;

    // Calculate the timestamp of the start of the year
    int _year = QDateTime::fromSecsSinceEpoch(created).date().year();
    double year = QDate(_year, 1, 1).startOfDay(Qt::UTC).toSecsSinceEpoch() - 86400.0;

    // Parse timestamp
    uint16_t days = repacked[8] >> 1;
    uint32_t ms = (repacked[9] & 0b1111111) << 20 | repacked[10] << 10 | repacked[11];
    timestamp = (double)year + (double)days * 86400.0 + (double)ms / 1000.0;
    timestamps[Imager::AVHRR].push_back(timestamp);

    ch3a.push_back(std::bitset<10>(ptr[6]).test(0));

    for (size_t i = 0; i < 11090; i++) {
        ptr[i] *= 64;
    }
    images[Imager::AVHRR]->push16Bit(ptr, 750);
}
