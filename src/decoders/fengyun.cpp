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

#include "fengyun.h"
#include "generic/deframer.h"
#include "generic/reedsolomon.h"
#include "ccsds/derand.h"
#include "ccsds/deframer.h"

FengyunDecoder::FengyunDecoder() {
    // 2048 pixels wide, maximum 7000 rows, 10 channels, interleaved as single pixels
    image = new RawImage(2048, 10);
}

bool FengyunDecoder::decodeFile(std::string filename) {
    QFile fileSource(filename.c_str());
    if (!fileSource.open(QIODevice::ReadOnly))
        return false;
    QDataStream dataStream(&fileSource);

    ccsds::Deframer deframer;
    ccsds::Derand derand;
    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 208400> virrDeframer(10, false);

    // Reed Solomon (from libsathelper)
    SatHelper::ReedSolomon reedSolomon;
    uint8_t rsWorkBuffer[255];

    uint8_t *buffer = new uint8_t[BUFFER_SIZE];
    uint8_t *frame = new uint8_t[1024];
    uint8_t *line = new uint8_t[208400 / 8];

    while (dataStream.readRawData((char *)buffer, BUFFER_SIZE)) {
        if (deframer.work(buffer, frame, BUFFER_SIZE)) {
            derand.work(frame, 1024);

            for (int i = 0; i < 4; i++) {
                reedSolomon.deinterleave(&frame[4], rsWorkBuffer, i, 4);
                reedSolomon.decode_ccsds(rsWorkBuffer);
                reedSolomon.interleave(rsWorkBuffer, &frame[4], i, 4);
            }

            uint8_t VCID = frame[5] & 0b111111;
            if (VCID == 5) {
                if (virrDeframer.work(&frame[14], line, 882)) {
                    image->push10Bit(line, 349);
                }
            }
        }
    }

    delete[] buffer;
    delete[] frame;
    delete[] line;

    fileSource.close();
    return 0;
}
