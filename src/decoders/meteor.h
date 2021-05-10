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

#include <string>
#include <cstdint>

#include "decoder.h"

class MeteorDecoder : public Decoder {
    public:
        MeteorDecoder();
        bool decodeFile(std::string filename);
        std::string imagerName() {
            return "MSU-MR";
        }
};

#endif
