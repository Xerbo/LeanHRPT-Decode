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
#include <cstring>

#define HEADER_LEN 6

namespace ccsds {
    // Constructor
    SimpleDemuxer::SimpleDemuxer(size_t packetDataLength)
        : dataLength(packetDataLength) { }

    // See https://lucasteske.dev/2016/11/goes-satellite-hunt-part-4-packet-demuxer/ for information about how this works
    // NOTICE: this will only work with CPPDUs that are spread over multiple VCDUs
    std::vector<uint8_t> SimpleDemuxer::work(const uint8_t *in) {
        packet.clear();

        // Parse FHP
        uint16_t fhp = (in[12] << 8 | in[13]) & 0b11111111111;

        // MPDU with just data, no CPPDU header
        if (writingData && fhp > dataLength - HEADER_LEN) {
            packetBuffer.insert(packetBuffer.end(), &in[14], &in[14+dataLength]);
        }
        // End of a CPPDU frame
        if (writingData && fhp <= dataLength - HEADER_LEN) {
            packetBuffer.insert(packetBuffer.end(), &in[14], &in[14+fhp]);
            writingData = false;
            packet = packetBuffer;
            packetBuffer.clear();
        }
        // A new CPPDU frame
        if (!writingData && fhp <= dataLength - HEADER_LEN) {
            packetBuffer.insert(packetBuffer.end(), &in[14+fhp], &in[14+dataLength]);
            writingData = true;
        }

        return packet;
    }

    std::vector<std::vector<uint8_t>> Demuxer::work(const uint8_t *in) {
        std::vector<std::vector<uint8_t>> packets;

        while (true) {
            DemuxerStatus state = internal_work(in);

            if (state == PARSED) {
                packets.push_back(packet);
            } else if (state == PROCEED) {
                break;
            }
        }

        return packets;
    }

    DemuxerStatus Demuxer::internal_work(const uint8_t *in) {
        uint16_t fhp = (in[12] << 8 | in[13]) & 0b11111111111;
        const uint8_t *data = &in[14];

        // Exit immediately if the FHP is invalid
        if (fhp > d_mpdu_size && fhp != 2047) return PROCEED;

        size_t bytes_left = 0;
        bool jump_idle = (fhp != 2047 && offset == 0);

        switch(state) {
            case IDLE:
                // Get the pointer to the next header, set that as the new offset
                if (fhp != 2047) {
                    offset = fhp;
                    frag_offset = 0;
                    state = HEADER;
                    return FRAGMENT;
                }
                break;
            case HEADER:
                bytes_left = HEADER_LEN - frag_offset;

                if (offset + bytes_left < d_mpdu_size) {
                    // The header's end byte is contained in this VCDU: copy bytes
                    std::copy(data+offset, data+offset+bytes_left, packet.begin()+frag_offset);
                    frag_offset = 0;
                    offset += bytes_left;
                    state = DATA;
                    return FRAGMENT;
                }

                // The header's end byte is in the next VCDU: copy some bytes and update the fragment offset
                std::copy(data+offset, data+d_mpdu_size, packet.begin()+frag_offset);
                frag_offset += d_mpdu_size - offset;
                offset = 0;
                return PROCEED;
            case DATA:
                bytes_left = CPPDUHeader(packet).length - frag_offset;

                if (offset + bytes_left < d_mpdu_size) {
                    // The end of this data segment is within the VCDU: copy bytes
                    std::copy(data+offset, data+offset+bytes_left, packet.begin()+HEADER_LEN + frag_offset);
                    frag_offset = 0;
                    offset += bytes_left;
                    state = jump_idle ? IDLE : HEADER;
                    return PARSED;
                }

                // The data continues in the next VCDU: copy some bytes and update the fragment offset 
                std::copy(data+offset, data+d_mpdu_size, packet.begin()+HEADER_LEN + frag_offset);
                frag_offset += d_mpdu_size - offset;
                offset = 0;
                state = jump_idle ? IDLE : DATA;
                return jump_idle ? FRAGMENT : PROCEED;
        }

        return PROCEED;
    }
}
