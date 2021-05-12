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

#include <QFile>
#include <QDataStream>

#include "metop.h"
#include "generic/reedsolomon.h"
#include "ccsds/demuxer.h"
#include "ccsds/derand.h"
#include "ccsds/deframer.h"

MetOpDecoder::MetOpDecoder() {
    image = new RawImage(2048, 5);
}

bool MetOpDecoder::decodeFile(std::string filename) {
    QFile fileSource(filename.c_str());
    if (!fileSource.open(QIODevice::ReadOnly))
        return false;
    QDataStream dataStream(&fileSource);

    ccsds::Deframer deframer;
    ccsds::Derand derand;
    ccsds::Demuxer demux(882);

    // Reed Solomon (from libsathelper)
    SatHelper::ReedSolomon reedSolomon;
    uint8_t rsWorkBuffer[255];

    uint8_t *buffer = new uint8_t[BUFFER_SIZE];
    uint8_t *frame = new uint8_t[1024];

    while (dataStream.readRawData(reinterpret_cast<char *>(buffer), BUFFER_SIZE)) {
        if (deframer.work(buffer, frame, BUFFER_SIZE)) {
            derand.work(frame, 1024);

            for (int i = 0; i < 4; i++) {
                reedSolomon.deinterleave(&frame[4], rsWorkBuffer, i, 4);
                reedSolomon.decode_ccsds(rsWorkBuffer);
                reedSolomon.interleave(rsWorkBuffer, &frame[4], i, 4);
            }

            uint8_t VCID = frame[5] & 0x3f; // 0b111111
            if (VCID == 9) {
                std::vector<uint8_t> line = demux.work(frame);
                // The only thing that VCID 9 will ever contain is AVHRR data so no need for APID filtering
                if(line.size() == 12966) {
                    image->push10Bit(&line[20], 11*5);
                }
            }
        }
    }

    delete[] buffer;
    delete[] frame;

    fileSource.close();
    return true;
}
