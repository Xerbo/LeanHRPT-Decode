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

#ifndef LEANHRPT_DECODERS_NOAA_H
#define LEANHRPT_DECODERS_NOAA_H

#include <string>
#include <cstdint>
#include <bitset>

#include "decoder.h"
#include "generic/deframer.h"

class NOAADecoder : public Decoder {
    public:
        NOAADecoder() : deframer(8, true) {
            frame = new uint8_t[(11090*10) / 8];
            repacked = new uint16_t[11090];
            images[Imager::AVHRR] = new RawImage(2048, 5);
            images[Imager::MHS] = new RawImage(90, 6);

            caldata["prt"] = 0.0;
            caldata["ptrn"] = 0.0;
            caldata["ch1_space"] = 0.0;
            caldata["ch2_space"] = 0.0;
            caldata["ch3_space"] = 0.0;
            caldata["ch4_space"] = 0.0;
            caldata["ch5_space"] = 0.0;
            caldata["ch3_cal"] = 0.0;
            caldata["ch4_cal"] = 0.0;
            caldata["ch5_cal"] = 0.0;
        }
        ~NOAADecoder() {
            delete[] frame;
            delete[] repacked;
        }
        void work(std::istream &stream) {
            if (d_filetype == FileType::raw16) {
                stream.read(reinterpret_cast<char *>(repacked), 11090*2);
                frame_work(repacked);
            } else {
                stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
                if(deframer.work(buffer, frame, BUFFER_SIZE)) {
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

        void frame_work(uint16_t *ptr) {
            uint16_t *data_words = &ptr[103];

            uint8_t frame_type = (ptr[6] >> 7) & 0b11;                
            if (frame_type == 3) { // AIS Frame
                for (size_t i = 0; i < 5; i++) {
                    uint8_t ais_frame[104];
                    bool parity_ok = true;

                    for (size_t j = 0; j < 104; j++) {
                        uint16_t word = data_words[104*i + j];
                        ais_frame[j] = word >> 2;

                        // Parity check
                        bool parity = std::bitset<8>(ais_frame[j]).count() % 2;
                        if (parity != std::bitset<16>(word).test(1)) {
                            parity_ok = false;
                            break;
                        }
                    }

                    if (parity_ok) {
                        uint8_t mhs_status = ais_frame[7];

                        std::memcpy(&mhsline[mhs_status*50], &ais_frame[48], 50);
                        if (mhs_status == 79) {
                            images[Imager::MHS]->push16Bit((const uint16_t *)&mhsline[  98], 0);
                            images[Imager::MHS]->push16Bit((const uint16_t *)&mhsline[1432], 0);
                            images[Imager::MHS]->push16Bit((const uint16_t *)&mhsline[2764], 0);
                            std::memset(mhsline, 0, 80*50);
                        }
                    }
                }
            }

            uint16_t days = repacked[8] >> 1;
            uint32_t ms = (repacked[9] & 0b1111111) << 20 | repacked[10] << 10 | repacked[11];

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

            double timestamp = 1609459200.0 - 86400.0;
            timestamp += days*86400.0 + ms/1000.0;
            timestamps[Imager::AVHRR].push_back(timestamp);

            for (size_t i = 0; i < 11090; i++) {
                ptr[i] *= 64;
            }
            images[Imager::AVHRR]->push16Bit(ptr, 750);
        }
    private:
        uint8_t *frame;
        uint16_t *repacked;
        uint8_t mhsline[80*50];
        ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> deframer;
};

#endif
