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

#include <cstring>

#include "meteor.h"
#include "generic/deframer.h"
#include "ccsds/deframer.h"

/*
 * See http://planet.iitp.ru/index.php?lang=en&page_type=spacecraft&page=meteor_m_n2_structure_1
 * for more information
 */

MeteorDecoder::MeteorDecoder() {
    image = new RawImage(1572, 6, 4);
}

bool MeteorDecoder::decodeFile(std::string filename) {
	QFile fileSource(filename.c_str());
    if (!fileSource.open(QIODevice::ReadOnly))
        return false;
    QDataStream dataStream(&fileSource);

    ccsds::Deframer deframer;
    ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 11850 * 8> MSUMRDeframer(10, false);
    
    uint8_t *buffer = new uint8_t[BUFFER_SIZE];
    uint8_t *frame = new uint8_t[1024];
    uint8_t *msumrBuffer = new uint8_t[948];
    uint8_t *msumrFrame = new uint8_t[11850];

    while (dataStream.readRawData((char *)buffer, BUFFER_SIZE)) {
        if (deframer.work(buffer, frame, BUFFER_SIZE)) {
            // See Table 1 - Structure of a transport frame
            std::memcpy(&msumrBuffer[0    ], &frame[23-1 ], 238);
            std::memcpy(&msumrBuffer[238  ], &frame[279-1], 238);
            std::memcpy(&msumrBuffer[238*2], &frame[535-1], 238);
            std::memcpy(&msumrBuffer[238*3], &frame[791-1], 234);

            if (MSUMRDeframer.work(msumrBuffer, msumrFrame, 948)) {
                image->push10Bit(&msumrFrame[50], 0);
            }
        }
    }

    delete[] buffer;
    delete[] frame;
    delete[] msumrBuffer;
    delete[] msumrFrame;

    fileSource.close();
    return true;
}
