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

#ifndef LEANHRPT_DECODERS_METEOR_H
#define LEANHRPT_DECODERS_METEOR_H

#include <cstdint>
#include <cstring>

#include "decoder.h"
#include "ccsds/deframer.h"
#include "generic/deframer.h"

#include <ctime>

// http://planet.iitp.ru/index.php?lang=en&page_type=spacecraft&page=meteor_m_n2_structure_1
class MeteorHRPTDecoder : public Decoder {
    public:
        MeteorHRPTDecoder() : MSUMRDeframer(9, false), mtvza_deframer(4, false) {
            frame = new uint8_t[1024];
            msumrBuffer = new uint8_t[948];
            msumrFrame = new uint8_t[11850];
            images[Imager::MSUMR] = new RawImage(1572, 6, 4);
            images[Imager::MTVZA] = new RawImage(50, 29);
        }
        ~MeteorHRPTDecoder() {
            delete[] frame;
            delete[] msumrBuffer;
            delete[] msumrFrame;
        }
    private:
        uint8_t *frame, *msumrBuffer, *msumrFrame;
        ccsds::Deframer deframer;
        ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 11850 * 8> MSUMRDeframer;
        ArbitraryDeframer<uint32_t, 0xFB386A45, 32, 248 * 8> mtvza_deframer;
        double msumr_timestamp = 0.0;

        void work(std::istream &stream) {
            if (d_filetype == FileType::CADU) {
                stream.read(reinterpret_cast<char *>(frame), 1024);
                frame_work(frame);
            } else {
                stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
                if (deframer.work(buffer, frame, BUFFER_SIZE)) {
                    frame_work(frame);
                }
            }
        }

        void frame_work(uint8_t *ptr) {
            // See Table 1 - Structure of a transport frame
            std::memcpy(&msumrBuffer[238*0], &ptr[ 23-1], 238);
            std::memcpy(&msumrBuffer[238*1], &ptr[279-1], 238);
            std::memcpy(&msumrBuffer[238*2], &ptr[535-1], 238);
            std::memcpy(&msumrBuffer[238*3], &ptr[791-1], 234);

            if (MSUMRDeframer.work(msumrBuffer, msumrFrame, 948)) {
                int hours = msumrFrame[8];
                hours = (hours-3) % 24; // Moscow to UTC
                int minutes = msumrFrame[9];
                int seconds = msumrFrame[10];
                int delay = msumrFrame[10];

                if (hours < 24 || minutes < 60 || seconds < 60) {
                    time_t day = created/86400 * 86400;
                    msumr_timestamp = (double)day + (double)hours*3600.0 + (double)minutes*60.0 + (double)seconds + (double)delay/255.0;
                } else {
                    msumr_timestamp = 0.0;
                }
                timestamps[Imager::MSUMR].push_back(msumr_timestamp);

                images[Imager::MSUMR]->push10Bit(&msumrFrame[50], 0);
            }

            uint8_t mtvza_buffer[32];
            uint8_t mtvza_frame[248];
            std::memcpy(&mtvza_buffer[8*0], &ptr[ 15-1], 8);
            std::memcpy(&mtvza_buffer[8*1], &ptr[271-1], 8);
            std::memcpy(&mtvza_buffer[8*2], &ptr[527-1], 8);
            std::memcpy(&mtvza_buffer[8*3], &ptr[783-1], 8);

            if (mtvza_deframer.work(mtvza_buffer, mtvza_frame, 32)) {
                // Frame type, only type 255 contains imagery
                if (mtvza_frame[4] != 255) return;

                // X position
                uint8_t x = mtvza_frame[5]-2;
                if (x > 24) return;

                for (size_t i = 0; i < 29; i++) {
                    uint8_t *data = &mtvza_frame[8];
                    unsigned short *channel = images[Imager::MTVZA]->getChannel(i);

                    int16_t val = data[i*2+1] << 8 | data[i*2];
                    channel[images[Imager::MTVZA]->rows()*images[Imager::MTVZA]->width() + x*2] = val+32768;
                }
                for (size_t i = 0; i < 29; i++) {
                    uint8_t *data = &mtvza_frame[128];
                    unsigned short *channel = images[Imager::MTVZA]->getChannel(i);

                    int16_t val = data[i*2+1] << 8 | data[i*2];
                    channel[images[Imager::MTVZA]->rows()*images[Imager::MTVZA]->width() + x*2 + 1] = val+32768;
                }

                // End of a line
                if (x == 24) {
                    images[Imager::MTVZA]->set_height(images[Imager::MTVZA]->rows() + 1);
                    timestamps[Imager::MTVZA].push_back(msumr_timestamp);
                }
            }
        }
};

#endif
