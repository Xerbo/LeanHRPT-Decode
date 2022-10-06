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

#include "decoder.h"

#include "fengyun_hrpt.h"
#include "meteor_hrpt.h"
#include "meteor_lrpt.h"
#include "metop_hrpt.h"
#include "noaa_dsb.h"
#include "noaa_gac.h"
#include "noaa_hrpt.h"

Decoder *Decoder::make(Protocol protocol, SatID sat) {
    Decoder *decoder;
    switch (protocol) {
        case Protocol::LRPT:
            decoder = new MeteorLRPTDecoder;
            break;
        case Protocol::HRPT:
            decoder = new NOAAHRPTDecoder;
            break;
        case Protocol::AHRPT:
            decoder = new MetopHRPTDecoder;
            break;
        case Protocol::MeteorHRPT:
            decoder = new MeteorHRPTDecoder;
            break;
        case Protocol::FengYunHRPT:
            decoder = new FengyunHRPTDecoder(sat);
            break;
        case Protocol::GAC:
            decoder = new NOAAGACDecoder(false);
            break;
        case Protocol::GACReverse:
            decoder = new NOAAGACDecoder(true);
            break;
        case Protocol::DSB:
            decoder = new NOAADSBDecoder;
            break;
        default:
            throw std::runtime_error("invalid value in enum `Protocol`");
    }
    return decoder;
}
