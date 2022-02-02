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

#include "decoder.h"
#include "generic/deframer.h"

class NOAAHRPTDecoder : public Decoder {
    public:
        NOAAHRPTDecoder() : deframer(8, true) {
            frame = new uint8_t[(11090*10) / 8];
            repacked = new uint16_t[11090];
            images[Imager::AVHRR] = new RawImage(2048, 5);
            images[Imager::MHS] = new RawImage(90, 6);
            images[Imager::HIRS] = new RawImage(56, 20);

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
        ~NOAAHRPTDecoder() {
            delete[] frame;
            delete[] repacked;
        }
    private:
        uint8_t *frame;
        uint16_t *repacked;
        uint8_t mhsline[80*50];
        ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> deframer;

        void work(std::istream &stream);
        void frame_work(uint16_t *ptr);
        void cal_data(uint16_t *ptr);
        void tip_work(const uint8_t *frame);
        void aip_work(const uint8_t *frame);
};

#endif
