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
        MetOpDecoder() : demux(882) {
            image = new RawImage(2048, 5);
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
        ccsds::Demuxer demux;

        void work(std::istream &stream) {
            if (is_ccsds_frames) {
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
                    image->push10Bit(&line[20], 11*5);
                }
            }
        }
};

#endif
