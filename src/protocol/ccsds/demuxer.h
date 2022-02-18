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

#ifndef LEANHRPT_CCSDS_DEMUXER_H
#define LEANHRPT_CCSDS_DEMUXER_H

#include <cstdint>
#include <vector>
#include <cstddef>

namespace ccsds {
    struct CPPDUHeader {
        uint16_t apid;
        uint8_t sequence_flag;
        uint16_t counter;
        uint16_t length;

        CPPDUHeader(const uint8_t *header) {
            apid = (header[0] << 8 | header[1]) & 0b11111111111;
            sequence_flag = (header[2] << 8 | header[3]) >> 14;
            counter = (header[2] << 8 | header[3]) & 0b11111111111111;
            length = (header[4] << 8 | header[5]) + 1;
        }
        CPPDUHeader(const std::vector<uint8_t> &header) : CPPDUHeader(header.data()) { }
    };

    // A (fast) demuxer that can only handle one packet per frame
    class SimpleDemuxer {
        public:
            SimpleDemuxer(bool insert_zone = true)
                : fhp_offset(insert_zone ? 12 : 10),
                  mpdu_size(insert_zone ? 882 : 884) { }
            std::vector<uint8_t> work(const uint8_t *in);
        private:
            const size_t fhp_offset;
            const size_t mpdu_size;

            bool writingData = false;
            std::vector<uint8_t> packetBuffer;
    };

    enum DemuxerState {
        IDLE, HEADER, DATA
    };
    enum DemuxerStatus {
        PROCEED, FRAGMENT, PARSED
    };

    // Demuxer that can handle an arbitrary amount of packets per frame
    class Demuxer {
        public:
            Demuxer(bool insert_zone = true)
                : fhp_offset(insert_zone ? 12 : 10),
                  mpdu_size(insert_zone ? 882 : 884),
                  packet(65536) { }
            std::vector<std::vector<uint8_t>> work(const uint8_t *in);
        private:
            const size_t fhp_offset;
            const size_t mpdu_size;

            DemuxerState state = IDLE;
            uint16_t offset = 0;
            uint16_t frag_offset = 0;
            std::vector<uint8_t> packet;

            DemuxerStatus internal_work(const uint8_t *in);
    };
}

#endif
