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

#include "meteor_lrpt.h"

#include <cmath>
#include <cstring>

#include "protocol/lrpt/packet.h"
#include "protocol/repack.h"

void MeteorLRPTDecoder::work(std::istream &stream) {
    if (d_filetype == FileType::CADU) {
        stream.read(reinterpret_cast<char *>(frame), 1024);
        frame_work();
    } else if (d_filetype == FileType::VCDU) {
        stream.read((char *)&frame[4], 892);
        frame_work();
    }
}

void MeteorLRPTDecoder::frame_work() {
    uint8_t VCID = frame[5] & 0b111111;
    if (VCID != 5) return;

    auto packets = demux.work(frame);
    for (const std::vector<uint8_t> &packet : packets) {
        ccsds::CPPDUHeader header(packet);
        if (start_offset == 0) {
            if (header.apid == 70) {
                start_offset = header.counter + 1;
            }
            continue;
        }
        if (header.apid == 70 && header.length == 58) {
            const uint8_t *data = &packet[14];

            uint16_t out[12];
            repack10(&data[35], out, 12);
            caldata["wl1_sum"] += out[0];
            caldata["bl1_sum"] += out[1];
            caldata["wl2_sum"] += out[2];
            caldata["bl2_sum"] += out[3];
            caldata["wl3_sum"] += out[4];
            caldata["bl3_sum"] += out[5];
            caldata["n"] += 1.0;
        }
        if (header.apid < 64 || header.apid > 69) continue;

        // Sanity check
        uint8_t seq = packet[6 + 8];
        if (seq > 196) continue;

        // Current day (always zero)
        // uint16_t day = packet[6+0] << 8 | packet[6+1];
        // Millisecond of day
        uint32_t timestamp = packet[6 + 2] << 24 | packet[6 + 3] << 16 | packet[6 + 4] << 8 | packet[6 + 5];

        // JPEG decoding
        std::array<jpeg::block<uint8_t>, MCU_PER_PACKET> pixels;
        uint8_t q = packet[6 + 13];
        if (!lrpt::decode_packet(pixels, &packet[6 + 14], q, header.length - 6)) continue;

        // Handle counter overflow
        if (last_counter > (size_t)header.counter + 8192) {
            counter_offset += 16384;  // 2^14
        }

        time_t day = created / 86400 * 86400;

        size_t counter = header.counter + counter_offset - start_offset;
        for (size_t j = 0; j < MCU_PER_PACKET; j++) {
            for (size_t y = 0; y < 8; y++) {
                for (size_t x = 0; x < 8; x++) {
                    size_t x1 = x + (j + seq) * 8;
                    size_t y1 = counter / (14 * 3 + 1) * 8 + y;

                    if (y == 0) {
                        timestamps[Imager::MSUMR].resize(y1 + 8);
                    }
                    timestamps[Imager::MSUMR][y1] = (double)day + (double)timestamp / 1000.0 - 10800.0 + y * 0.205;

                    images[Imager::MSUMR]->set_height(y1 + 1);
                    unsigned short *ch = images[Imager::MSUMR]->getChannel(header.apid - 64);

                    ch[y1 * 1568 + x1] = pixels[j][y][x] * 256;
                }
            }
        }

        last_counter = header.counter;
    }
}
