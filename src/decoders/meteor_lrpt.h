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

#ifndef DECODERS_METEOR_LRPT_H
#define DECODERS_METEOR_LRPT_H

#include "decoder.h"

#include "ccsds/demuxer.h"
#include "lrpt/packet.h"
#include <cstring>
#include <cmath>

// http://planet.iitp.ru/index.php?lang=en&page_type=spacecraft&page=meteor_m_n2_structure_2
class MeteorLRPTDecoder : public Decoder {
    public:
        MeteorLRPTDecoder() {
            // Allocate all 6 channels for correct channel labeling
            images[Imager::MSUMR] = new RawImage(1568, 6);
            timestamps[Imager::MSUMR].resize(10000);
        }
    private:
        uint8_t frame[1024];
        ccsds::Demuxer demux;

        size_t counter_offset = 0;
        size_t start_offset = 0;
        size_t last_counter = 0;

        void work(std::istream &stream) {
            if (d_filetype == FileType::CADU) {
                stream.read(reinterpret_cast<char *>(frame), 1024);
                frame_work();
            } else if (d_filetype == FileType::VCDU) {
                stream.read((char *)&frame[4], 892);
                frame_work();
            }
        }

        void frame_work() {
            uint8_t VCID = frame[5] & 0b111111;
            if (VCID != 5) return;

            auto packets = demux.work(frame);
            for (const std::vector<uint8_t> &packet : packets) {
                ccsds::CPPDUHeader header(packet);
                if (start_offset == 0) {
                    if (header.apid == 70) {
                        start_offset = header.counter+1;
                    }
                    continue;
                }
                if (header.apid < 64 || header.apid > 69) continue;

                // Sanity check
                uint8_t seq = packet[6+8];
                if (seq > 196) continue;

                // Current day (always zero)
                //uint16_t day = packet[6+0] << 8 | packet[6+1]; 
                // Millisecond of day
                uint32_t timestamp = packet[6+2] << 24 | packet[6+3] << 16 | packet[6+4] << 8 | packet[6+5];

                // JPEG decoding
                std::array<jpeg::block<uint8_t>, MCU_PER_PACKET> pixels;
                uint8_t q = packet[6+13];
                if (!lrpt::decode_packet(pixels, &packet[6+14], q, header.length-6)) continue;

                // Handle counter overflow
                if (last_counter > (size_t)header.counter+8192) {
                    counter_offset += 16384; // 2^14
                }

                size_t counter = header.counter+counter_offset - start_offset;
                for (size_t j = 0; j < MCU_PER_PACKET; j++) {
                    for (size_t y = 0; y < 8; y++) {
                        for (size_t x = 0; x < 8; x++) {
                            size_t x1 = x + (j+seq)*8;
                            size_t y1 = counter/(14*3 + 1) * 8 + y;

                            if (y == 0) {
                                time_t day = created/86400 * 86400;
                                timestamps[Imager::MSUMR][y1] = (double)day + (double)timestamp/1000.0 - 10800.0;
                            }
                            images[Imager::MSUMR]->set_height(y1+1);
                            unsigned short *ch = images[Imager::MSUMR]->getChannel(header.apid - 64);

                            ch[y1*1568 + x1] = pixels[j][y][x] * 256;
                        }
                    }
                }

                last_counter = header.counter;
            }
        }
};

#endif
