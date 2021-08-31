#include "fingerprint.h"

#include <algorithm>
#include <bitset>

#include "ccsds/deframer.h"
#include "ccsds/derand.h"
#include "generic/deframer.h"

SatID Fingerprint::fingerprint_ccsds(std::istream &stream) {
    std::array<size_t, 11> SCIDs = { 0 };

    while (!stream.eof()) {
        uint8_t frame[1024];
        stream.read(reinterpret_cast<char *>(frame), 1024);

        if (std::bitset<32>(*(uint32_t *)&frame[10]).count() < 8) {
            SCIDs[SatID::MeteorM22] += 4;
        } else {
            // https://sanaregistry.org/r/spacecraftid/
            uint8_t SCID = ((uint16_t)frame[4] << 8 | frame[5]) >> 6;
            switch (SCID) {
                case 0x0B: SCIDs[SatID::MetOpB]++;    break; // MetOp 1
                case 0x0C: SCIDs[SatID::MetOpA]++;    break; // MetOp 2
                case 0x0D: SCIDs[SatID::MetOpC]++;    break; // MetOp 3
                case 0x31: SCIDs[SatID::FengYun3C]++; break;
                case 0x32: SCIDs[SatID::FengYun3B]++; break;
                case 0x33: SCIDs[SatID::FengYun3C]++; break;
                default:                              break;
            }
        }

        size_t *max = std::max_element(SCIDs.begin(), SCIDs.end());
        if (*max > 100) {
            return (SatID)(max - SCIDs.begin());
        }
    }

    return SatID::Unknown;
}

SatID Fingerprint::fingerprint_ccsds_raw(std::istream &stream) {
    std::array<size_t, 12> SCIDs = { 0 };
    ccsds::Deframer deframer;
    ccsds::Derand derand;

    while (!stream.eof()) {
        uint8_t frame[1024];
        uint8_t buffer[1024];
        stream.read(reinterpret_cast<char *>(buffer), 1024);

        if (deframer.work(buffer, frame, 1024)) {
            if (std::bitset<32>(*(uint32_t *)&frame[10]).count() < 8) {
                SCIDs[SatID::MeteorM22] += 4;
            } else {
                derand.work(frame, 1024);
                // https://sanaregistry.org/r/spacecraftid/
                uint8_t SCID = ((uint16_t)frame[4] << 8 | frame[5]) >> 6;
                switch (SCID) {
                    case 0x0B: SCIDs[SatID::MetOpB]++;    break; // MetOp 1
                    case 0x0C: SCIDs[SatID::MetOpA]++;    break; // MetOp 2
                    case 0x0D: SCIDs[SatID::MetOpC]++;    break; // MetOp 3
                    case 0x31: SCIDs[SatID::FengYun3C]++; break;
                    case 0x32: SCIDs[SatID::FengYun3B]++; break;
                    case 0x33: SCIDs[SatID::FengYun3C]++; break;
                    default:                              break;
                }
            }

            size_t *max = std::max_element(SCIDs.begin(), SCIDs.end());
            if (*max > 1000) {
                return (SatID)(max - SCIDs.begin());
            }
        }
    }

    return SatID::Unknown;
}

bool Fingerprint::is_ccsds(std::istream &stream) {
    uint32_t header;
    stream.read((char *)&header, 4);
    stream.seekg(stream.beg);

    return (header == 0x1DFCCF1A);
}

bool Fingerprint::is_noaa(std::istream &stream) {
    uint8_t buffer[1024];
    uint8_t ccsds[1024];
    uint8_t *noaa = new uint8_t[(11090*10) / 8];
    ccsds::Deframer deframer;
    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> deframer2(10, true);

    size_t i = 0, j = 0;

    while (!stream.eof()) {
        stream.read(reinterpret_cast<char *>(buffer), 1024);
        if (deframer.work(buffer, ccsds, 1024)) {
            i++;
            if (i > 200) {
                stream.seekg(stream.beg);
                delete[] noaa;
                return false;
            }
        }
        if (deframer2.work(buffer, noaa, 1024)) {
            j++;
            if (j > 10) {
                stream.seekg(stream.beg);
                delete[] noaa;
                return true;
            }
        }
    }

    delete[] noaa;
    return false;
}

SatID Fingerprint::id_noaa(std::istream &stream) {
    uint8_t buffer[1024];
    uint8_t *frame = new uint8_t[(11090*10) / 8];
    uint16_t *repacked = new uint16_t[11090];
    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> deframer(10, true);

    std::array<size_t, 12> idfks = { 0 };

    while (!stream.eof()) {
        stream.read(reinterpret_cast<char *>(buffer), 1024);
        if (deframer.work(buffer, frame, 1024)) {
            size_t j = 0;
            for (size_t i = 0; i < 11090; i += 4) {
                repacked[i + 0] =  (frame[j + 0] << 2)       | (frame[j + 1] >> 6);
                repacked[i + 1] = ((frame[j + 1] % 64) << 4) | (frame[j + 2] >> 4);
                repacked[i + 2] = ((frame[j + 2] % 16) << 6) | (frame[j + 3] >> 2);
                repacked[i + 3] = ((frame[j + 3] % 4 ) << 8) |  frame[j + 4];
                j += 5;
            }

            std::map<int, SatID> table = {
                { 7,  SatID::NOAA15 },
                { 13, SatID::NOAA18 },
                { 15, SatID::NOAA19 }
            };
            int address = ((repacked[6] & 0x078) >> 3) & 0x000F;
            if (table.count(address)) {
                idfks[table[address]]++;
            }

            size_t *max = std::max_element(idfks.begin(), idfks.end());
            if (*max > 10) {
                delete[] frame;
                delete[] repacked;
                return (SatID)(max - idfks.begin());
            }
        }
    }

    delete[] frame;
    delete[] repacked;
    return SatID::Unknown;
}

SatID Fingerprint::file(std::string filename) {
    std::filebuf file;
    if (!file.open(filename, std::ios::in | std::ios::binary)) {
        return SatID::Unknown;
    }
    std::istream stream(&file);

    if (is_ccsds(stream)) {
        SatID id = fingerprint_ccsds(stream);
        file.close();
        return id;
    }

    if (is_noaa(stream)) {
        SatID id = id_noaa(stream);
        file.close();
        return id;
    }

    SatID id = fingerprint_ccsds_raw(stream);
    file.close();
    return id;
}
