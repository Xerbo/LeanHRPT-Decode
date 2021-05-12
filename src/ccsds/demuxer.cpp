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

#include "demuxer.h"

#include <iostream>

namespace ccsds {
    // Constructor
    Demuxer::Demuxer(unsigned int packetDataLength)
        : dataLength(packetDataLength) { }

    // See https://lucasteske.dev/2016/11/goes-satellite-hunt-part-4-packet-demuxer/ for information about how this works
    // NOTICE: this will only work with CPPDUs that are spread over multiple VCDUs
    std::vector<uint8_t> Demuxer::work(uint8_t *in) {
        packet.clear();

        // Parse FHP
        uint16_t fhp = (in[12] << 8 | in[13]) & 0x07ff; // 0b11111111111

        // MPDU with just data, no CPPDU header
        if (writingData && fhp > dataLength - 6) {
            packetBuffer.insert(packetBuffer.end(), &in[14], &in[14+dataLength]);
        }
        // End of a CPPDU frame
        if (writingData && fhp <= dataLength - 6) {
            packetBuffer.insert(packetBuffer.end(), &in[14], &in[14+fhp]);
            writingData = false;
            packet = packetBuffer;
            packetBuffer.clear();
        }
        // A new CPPDU frame
        if (!writingData && fhp <= dataLength - 6) {
            packetBuffer.insert(packetBuffer.end(), &in[14+fhp], &in[14+dataLength]);
            writingData = true;
        }

        return packet;
    }
}
