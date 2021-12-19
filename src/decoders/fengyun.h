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

#ifndef LEANHRPT_DECODERS_FENGYUN_H
#define LEANHRPT_DECODERS_FENGYUN_H

#include <cstdint>

#include "decoder.h"
#include "generic/deframer.h"

class FengyunDecoder : public Decoder {
    public:
        FengyunDecoder(SatID sat) : virrDeframer(8, false) {
            frame = new uint8_t[1024];
            line = new uint8_t[208400 / 8];
            images[Imager::VIRR] = new RawImage(2048, 10);

            switch (sat) {
                case SatID::FengYun3A: launch_timestamp = 1289347200.0; break;
                case SatID::FengYun3B: launch_timestamp = 1289347200.0; break;
                case SatID::FengYun3C: launch_timestamp = 1522368000.0; break;
                default: throw std::runtime_error("Non FY SatID passed to FengyunDecoder");
            }
        }
        ~FengyunDecoder() {
            delete[] frame;
            delete[] line;
        }
    private:
        uint8_t *frame, *line;
        ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 208400> virrDeframer;
        double launch_timestamp;

        void work(std::istream &stream) {
            if (d_filetype == FileType::CADU) {
                stream.read(reinterpret_cast<char *>(frame), 1024);
                frame_work(frame);
            } else if (d_filetype == FileType::VCDU) {
                stream.read((char *)&frame[4], 892);
                frame_work(frame);
            }
        }

        void frame_work(uint8_t *ptr) {
            uint8_t VCID = ptr[5] & 0b111111;
            if (VCID == 5) {
                if (virrDeframer.work(&ptr[14], line, 882)) {
                    // Stolen from the power of @Aang254's SatDump
                    uint8_t timestamp[8];
                    timestamp[0] = (line[26041] & 0b111111) << 2 | line[26042] >> 6;
                    timestamp[1] = (line[26042] & 0b111111) << 2 | line[26043] >> 6;
                    timestamp[2] = (line[26043] & 0b111111) << 2 | line[26044] >> 6;
                    timestamp[3] = (line[26044] & 0b111111) << 2 | line[26045] >> 6;
                    timestamp[4] = (line[26045] & 0b111111) << 2 | line[26046] >> 6;
                    timestamp[6] = (line[26046] & 0b111111) << 2 | line[26047] >> 6;
                    timestamp[7] = (line[26047] & 0b111111) << 2 | line[26048] >> 6;

                    uint16_t days = (timestamp[1] & 0b11) << 10 | timestamp[2] << 2 | timestamp[3] >> 6;
                    uint32_t ms = (timestamp[3] & 0b11) << 24 | timestamp[4] << 16 | timestamp[6] << 8 | timestamp[7];
                    timestamps[Imager::VIRR].push_back(launch_timestamp + days*86400.0 + ms/1000.0 + 12.0*3600.0);

                    images[Imager::VIRR]->push10Bit(line, 349);
                }
            }
        }
};

#endif
