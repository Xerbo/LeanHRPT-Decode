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

#ifndef LEANHRPT_FINGERPRINT_H_
#define LEANHRPT_FINGERPRINT_H_

#include <atomic>
#include <fstream>
#include <set>
#include <string>

#include "decoders/decoder.h"
#include "satinfo.h"

enum class Suggestion { Automatic, POESHRPT, POESGAC, POESDSB, MeteorHRPT, MeteorLRPT, MetOpHRPT, FengYunHRPT };

class Fingerprint {
   public:
    Fingerprint() : is_running(true) {}

    /**
     * Start fingerprinting a file
     *
     * @returns The detected satellite, filetype and protocol
     */
    std::tuple<SatID, FileType, Protocol> file(std::string filename, Suggestion suggestion);

    /**
     * Stop fingerprinting a file
     *
     * This is a only used when running in a thread
     */
    void stop() { is_running = false; }

   private:
    SatID fingerprint_ccsds(std::istream &stream, FileType type);
    SatID fingerprint_noaa(std::istream &stream, FileType type);
    SatID fingerprint_meteor(std::istream &stream, FileType type);
    SatID fingerprint_gac(std::istream &stream, bool reverse);
    SatID fingerprint_dsb(std::istream &stream);

    Protocol fingerprint_raw(std::istream &stream, Suggestion suggestion);
    static std::set<Protocol> ccsds_downlinks(SatID id);

    static FileType id_magic(std::istream &stream) {
        uint8_t header[4];
        stream.read((char *)&header, 4);
        stream.seekg(stream.beg);
        if (header[0] == 0x1A && header[1] == 0xCF && header[2] == 0xFC && header[3] == 0x1D) return FileType::CADU;
        if (header[0] == 0x84 && header[1] == 0x02 && header[2] && 0x6F && header[3] == 0x01) return FileType::raw16;
        if (header[0] == 0x02 && header[1] == 0x84 && header[2] && 0x01 && header[3] == 0x6F) return FileType::HRP;
        if (header[0] == 0xED && header[1] == 0xE2) return FileType::TIP;
        return FileType::Unknown;
    }

    std::atomic<bool> is_running;
};

#endif
