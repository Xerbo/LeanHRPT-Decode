/*
 * LeanHRPT Decode
 * Copyright (C) 2021-2022 Xerbo
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

#include <cstring>
#include <iostream>

#define HEADER_LEN 6

namespace ccsds {
std::vector<uint8_t> SimpleDemuxer::work(const uint8_t *in) {
    std::vector<uint8_t> packet;
    uint16_t fhp = (in[fhp_offset] << 8 | in[fhp_offset + 1]) & 0b11111111111;
    const uint8_t *data = &in[fhp_offset + 2];

    // Exit immediately if the FHP is invalid
    if (fhp > mpdu_size && fhp != 2047) return packet;

    // MPDU with just data, no CPPDU header
    if (writingData && fhp == 2047) {
        packetBuffer.insert(packetBuffer.end(), data, &data[mpdu_size]);
    }
    // End of a CPPDU frame
    if (writingData && fhp != 2047) {
        packetBuffer.insert(packetBuffer.end(), data, &data[fhp]);
        writingData = false;
        packet = packetBuffer;
        packetBuffer.clear();
    }
    // A new CPPDU frame
    if (!writingData && fhp != 2047) {
        packetBuffer.insert(packetBuffer.end(), &data[fhp], &data[mpdu_size]);
        writingData = true;
    }

    return packet;
}

std::vector<std::vector<uint8_t>> Demuxer::work(const uint8_t *in) {
    std::vector<std::vector<uint8_t>> packets;

    while (true) {
        DemuxerStatus state = internal_work(in);

        if (state == DemuxerStatus::PARSED) {
            packets.push_back(packet);
        } else if (state == DemuxerStatus::PROCEED) {
            break;
        }
    }

    return packets;
}
DemuxerStatus Demuxer::internal_work(const uint8_t *in) {
    uint16_t fhp = (in[fhp_offset] << 8 | in[fhp_offset + 1]) & 0b11111111111;
    const uint8_t *data = &in[fhp_offset + 2];

    // Exit immediately if the FHP is invalid
    if (fhp > mpdu_size && fhp != 2047) return DemuxerStatus::PROCEED;

    size_t bytes_left = 0;
    bool jump_idle = (fhp != 2047 && offset == 0);

    switch (state) {
        case DemuxerState::IDLE:
            // Get the pointer to the next header, set that as the new offset
            if (fhp != 2047) {
                offset = fhp;
                frag_offset = 0;
                state = DemuxerState::HEADER;
                return DemuxerStatus::FRAGMENT;
            }
            break;
        case DemuxerState::HEADER:
            bytes_left = HEADER_LEN - frag_offset;

            if (offset + bytes_left < mpdu_size) {
                // The header's end byte is contained in this VCDU: copy bytes
                std::copy(data + offset, data + offset + bytes_left, packet.begin() + frag_offset);
                frag_offset = 0;
                offset += bytes_left;
                state = DemuxerState::DATA;
                return DemuxerStatus::FRAGMENT;
            }

            // The header's end byte is in the next VCDU: copy some bytes and update the fragment offset
            std::copy(data + offset, data + mpdu_size, packet.begin() + frag_offset);
            frag_offset += mpdu_size - offset;
            offset = 0;
            return DemuxerStatus::PROCEED;
        case DemuxerState::DATA:
            bytes_left = CPPDUHeader(packet).length - frag_offset;

            if (offset + bytes_left < mpdu_size) {
                // The end of this data segment is within the VCDU: copy bytes
                std::copy(data + offset, data + offset + bytes_left, packet.begin() + HEADER_LEN + frag_offset);
                frag_offset = 0;
                offset += bytes_left;
                state = jump_idle ? DemuxerState::IDLE : DemuxerState::HEADER;
                return DemuxerStatus::PARSED;
            }

            // The data continues in the next VCDU: copy some bytes and update the fragment offset
            std::copy(data + offset, data + mpdu_size, packet.begin() + HEADER_LEN + frag_offset);
            frag_offset += mpdu_size - offset;
            offset = 0;
            state = jump_idle ? DemuxerState::IDLE : DemuxerState::DATA;
            return jump_idle ? DemuxerStatus::FRAGMENT : DemuxerStatus::PROCEED;
    }

    return DemuxerStatus::PROCEED;
}
}  // namespace ccsds
