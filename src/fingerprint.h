#ifndef LEANHRPT_FINGERPRINT_H
#define LEANHRPT_FINGERPRINT_H

#include "satinfo.h"
#include "decoders/decoder.h"
#include <fstream>
#include <string>
#include <atomic>
#include <set>

const std::map<std::string, FileType> known_extensions = {
    { "cadu",  FileType::CADU },
    { "vcdu",  FileType::VCDU },
    { "raw16", FileType::raw16 },
    { "hrp",   FileType::HRP },
    { "tip",   FileType::TIP },
};

class Fingerprint {
    public:
        Fingerprint() : is_running(true) { }
        std::tuple<SatID, FileType, Protocol> file(std::string filename);

        void stop() {
            is_running = false;
        }
    private:
        SatID fingerprint_ccsds(std::istream &stream, FileType type);
        SatID fingerprint_noaa(std::istream &stream, FileType type);
        SatID fingerprint_meteor(std::istream &stream, FileType type);
        SatID fingerprint_gac(std::istream &stream, bool reverse);
        SatID fingerprint_dsb(std::istream &stream);

        Protocol fingerprint_raw(std::istream &stream);
        std::set<Protocol> ccsds_downlinks(SatID id);

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
