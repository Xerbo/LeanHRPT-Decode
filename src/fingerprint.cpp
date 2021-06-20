#include "fingerprint.h"

#include <fstream>
#include <cstring>

#include "ccsds/deframer.h"
#include "ccsds/derand.h"
#include "generic/deframer.h"

Satellite fingerprint_ccsds_frames(std::istream &stream) {
    uint8_t frame[1024];
    size_t virtual_channels[64] = { 0 };

    // Check if the file is CCSDS frames
    for (size_t i = 0; i < 50; i++) {
        stream.read(reinterpret_cast<char *>(frame), 1024);
        if (frame[0] != 0x1A || frame[1] != 0xCF || frame[2] != 0xFC || frame[3] != 0x1D) {
            return Satellite::Unknown;
        }
        uint8_t VCID = frame[5] & 0x3f; // 0b111111
        virtual_channels[VCID]++;
    }

    // Perform virtual channel analysis
    if (virtual_channels[5] > 12) {
        return Satellite::FengYun;
    } else if (virtual_channels[9] > 8) {
        return Satellite::MetOp;
    } else {
        return Satellite::Meteor;
    }
}

Satellite fingerprint_ccsds_data(std::istream &stream) {
    size_t frames = 0;
    size_t virtual_channels[64] = { 0 };

    uint8_t buffer[1024];
    uint8_t frame[1024];

    ccsds::Deframer deframer;
    ccsds::Derand derand;
    for (size_t i = 0; i < 5000; i++) {
        stream.read(reinterpret_cast<char *>(buffer), 1024);
        if (deframer.work(buffer, frame, 1024)) {
            if (frame[10] == 0 && frame[11] == 0 && frame[12] == 0 && frame[13] == 0) {
                return Satellite::Meteor;
            }

            frames++;
            derand.work(frame, 1024);
            uint8_t VCID = frame[5] & 0x3f; // 0b111111
            virtual_channels[VCID]++;
        }
    }

    if (frames > 50) {
        if (virtual_channels[5] > frames/10) {
            return Satellite::FengYun;
        } else if (virtual_channels[9] > frames/10) {
            return Satellite::MetOp;
        }
    }

    return Satellite::Unknown;
}

bool is_noaa(std::istream &stream) {
    ArbitraryDeframer<uint64_t, 0xA116FD719D8CC950, 64, 11090 * 10> deframer2;
    uint8_t *line = new uint8_t[110900 / 8];
    uint8_t buffer[1024];
    size_t frames = 0;

    for (size_t i = 0; i < 5000; i++) {
        stream.read(reinterpret_cast<char *>(buffer), 1024);
        if (deframer2.work(buffer, line, 1024)) {
            frames++;
            if (frames > 30) {
                delete[] line;
                return true;
            }
        }
    }

    delete[] line;
    return false;
}

Satellite fingerprint(std::string filename) {
    std::filebuf file = std::filebuf();
    if (!file.open(filename, std::ios::in | std::ios::binary)) {
        return Satellite::Unknown;
    }
    std::istream stream(&file);

    Satellite satellite = fingerprint_ccsds_frames(stream);
    if (satellite != Satellite::Unknown) {
        file.close();
        return satellite;
    }

    stream.seekg(stream.beg);
    satellite = fingerprint_ccsds_data(stream);
    if (satellite != Satellite::Unknown) {
        file.close();
        return satellite;
    }

    stream.seekg(stream.beg);
    if (is_noaa(stream)) {
        file.close();
        return Satellite::NOAA;
    }

    file.close();
    return Satellite::Unknown;
}
