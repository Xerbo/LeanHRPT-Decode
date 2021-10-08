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

#ifndef LEANHRPT_DECODERS_METOP_H
#define LEANHRPT_DECODERS_METOP_H

#include <cstdint>

#include "decoder.h"
#include "ccsds/deframer.h"
#include "ccsds/derand.h"
#include "ccsds/reedsolomon.h"
#include "ccsds/demuxer.h"

class MetOpDecoder : public Decoder {
    public:
        MetOpDecoder() : demux(882), mhs_demux(882) {
            images[Imager::AVHRR] = new RawImage(2048, 5);
            images[Imager::MHS] = new RawImage(90, 6);
            frame = new uint8_t[1024];
        }
        ~MetOpDecoder() {
            delete[] frame;
        }
    private:
        uint8_t *frame;
        SatHelper::ReedSolomon reedSolomon;
        ccsds::Deframer deframer;
        ccsds::Derand derand;
        ccsds::Demuxer demux, mhs_demux;

        void work(std::istream &stream) {
            if (d_filetype == FileType::CADU) {
                stream.read(reinterpret_cast<char *>(frame), 1024);
                frame_work(frame);
            } else {
                stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
                if (deframer.work(buffer, frame, BUFFER_SIZE)) {
                    derand.work(frame, 1024);
                    reedSolomon.decode_intreleaved_ccsds(frame);
                    frame_work(frame);
                }
            }
        }

        void frame_work(uint8_t *ptr) {
            uint8_t VCID = ptr[5] & 0x3f; // 0b111111
            if (VCID == 9) {
                std::vector<uint8_t> line = demux.work(ptr);

                // The only thing that VCID 9 will ever contain is AVHRR data so no need for APID filtering
                if(line.size() == 12966) {
                    images[Imager::AVHRR]->push10Bit(&line[20], 11*5);

                    // Days since 01/01/2000
                    uint16_t days = line[6] << 8 | line[7];
                    // Milliseconds since start of the day
                    uint32_t ms = line[8] << 24 | line[9] << 16 | line[10] << 8 | line[11];

                    double timestamp = 946684800.0 + days*86400.0 + ms/1000.0;
                    timestamps[Imager::AVHRR].push_back(timestamp);
                }
            } else if (VCID == 12) {
                std::vector<uint8_t> line = mhs_demux.work(ptr);

                if (line.size() == 1308) {
                    images[Imager::MHS]->push16Bit((uint16_t *)&line[70], 0);

                    // Days since 01/01/2000
                    uint16_t days = line[6] << 8 | line[7];
                    // Milliseconds since start of the day
                    uint32_t ms = line[8] << 24 | line[9] << 16 | line[10] << 8 | line[11];

                    double timestamp = 946684800.0 + days*86400.0 + ms/1000.0;
                    timestamps[Imager::MHS].push_back(timestamp);                    
                }
            }
        }
};

#endif
