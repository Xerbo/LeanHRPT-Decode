#ifndef LEANHRPT_FINGERPRINT_H
#define LEANHRPT_FINGERPRINT_H

#include "satinfo.h"
#include <fstream>
#include <string>

class Fingerprint {
    public:
        static SatID file(std::string filename);
    private:
        static SatID fingerprint_ccsds(std::istream &stream);
        static SatID fingerprint_ccsds_raw(std::istream &stream);
        static bool is_ccsds(std::istream &stream);
        static bool is_noaa(std::istream &stream);
        static SatID id_noaa(std::istream &stream);
};

#endif
