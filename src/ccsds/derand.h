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

#ifndef LEANHRPT_CCSDS_DERAND_H
#define LEANHRPT_CCSDS_DERAND_H

#include <cstdint>
#include <cstddef>

namespace ccsds {
    class Derand {
        public:
            Derand();
            void work(uint8_t *data, size_t len);
        private:
            uint8_t randomTable[1024];
            void generateRandomTable();
    };
}

#endif
