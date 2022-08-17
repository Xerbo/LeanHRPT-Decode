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

#include "metop_hrpt.h"

void MetopHRPTDecoder::work(std::istream &stream) {
    if (d_filetype == FileType::CADU) {
        stream.read(reinterpret_cast<char *>(frame), 1024);
        frame_work(frame);
    } else if (d_filetype == FileType::VCDU) {
        stream.read((char *)&frame[4], 892);
        frame_work(frame);
    }
}

void MetopHRPTDecoder::frame_work(uint8_t *ptr) {
    uint8_t VCID = ptr[5] & 0b111111;
    if (VCID == 9) {
        std::vector<uint8_t> line = demux.work(ptr);

        // The only thing that VCID 9 will ever contain is AVHRR data so no need for APID filtering
        if (line.size() == 12966) {
            images[Imager::AVHRR]->push10Bit(&line[20], 11 * 5);

            // Days since 01/01/2000
            uint16_t days = line[6] << 8 | line[7];
            // Milliseconds since start of the day
            uint32_t ms = line[8] << 24 | line[9] << 16 | line[10] << 8 | line[11];

            double timestamp = 946684800.0 + days * 86400.0 + ms / 1000.0;
            timestamps[Imager::AVHRR].push_back(timestamp);
            ch3a.push_back(ccsds::CPPDUHeader(line).apid == 103);
        }
    } else if (VCID == 12) {
        std::vector<uint8_t> line = mhs_demux.work(ptr);

        if (line.size() == 1308) {
            images[Imager::MHS]->push16Bit((uint16_t *)&line[70], 0);

            // Days since 01/01/2000
            uint16_t days = line[6] << 8 | line[7];
            // Milliseconds since start of the day
            uint32_t ms = line[8] << 24 | line[9] << 16 | line[10] << 8 | line[11];

            double timestamp = 946684800.0 + days * 86400.0 + ms / 1000.0;
            timestamps[Imager::MHS].push_back(timestamp);
        }
    }
}
