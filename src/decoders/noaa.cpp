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

#include "noaa.h"
#include "generic/deframer.h"

NOAADecoder::NOAADecoder() {
    image = new RawImage(2048, 5);
}

bool NOAADecoder::decodeFile(std::string filename) {
	QFile fileSource(filename.c_str());
    if (!fileSource.open(QIODevice::ReadOnly))
        return false;
    QDataStream dataStream(&fileSource);

    ArbitraryDeframer<uint64_t, 0xA116FD719D8CC950, 64, 11090 * 10> deframer(10, true);

    uint8_t *buffer = new uint8_t[BUFFER_SIZE];
    uint8_t *frame = new uint8_t[(11090*10) / 8];

    while(dataStream.readRawData(reinterpret_cast<char *>(buffer), BUFFER_SIZE)) {
        if(deframer.work(buffer, frame, BUFFER_SIZE)) {
            image->push10Bit(frame, 750);
        }
    }

    delete[] buffer;
    delete[] frame;

    fileSource.close();
    return true;
}
