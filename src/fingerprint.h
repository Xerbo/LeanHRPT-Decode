#ifndef LEANHRPT_FINGERPRINT_H
#define LEANHRPT_FINGERPRINT_H

#include "satinfo.h"
#include <fstream>
#include <string>
#include <atomic>

class Fingerprint {
    public:
        Fingerprint() : is_running(true) { }
        SatID file(std::string filename);

        void stop() {
            is_running = false;
        }
    private:
        SatID fingerprint_ccsds(std::istream &stream);
        SatID fingerprint_ccsds_raw(std::istream &stream);
        bool is_noaa(std::istream &stream);
        SatID id_noaa_raw(std::istream &stream);
        SatID id_noaa(std::istream &stream);

        static bool is_ccsds(std::istream &stream) {
            uint8_t header[4];
            stream.read((char *)&header, 4);
            stream.seekg(stream.beg);
            return (header[0] == 0x1A && header[1] == 0xCF && header[2] == 0xFC && header[3] == 0x1D);  
        };
        static bool is_raw16(std::istream &stream) {
            uint8_t header[4];
            stream.read((char *)&header, 4);
            stream.seekg(stream.beg);
            return (header[0] == 0x84 && header[1] == 0x02 && header[2] && 0x6F && header[3] == 0x01);  
        };

        std::atomic<bool> is_running;
};

#endif
