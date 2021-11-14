#include "fingerprint.h"

#include <algorithm>
#include <vector>
#include <cstring>

#include "ccsds/deframer.h"
#include "ccsds/derand.h"
#include "generic/deframer.h"

class CCSDSFingerprint {
    public:
        CCSDSFingerprint(bool raw) : _raw(raw), telemetry_deframer(9, false) { };
        SatID processFrame(uint8_t *frame) {
            uint8_t telemetry_buffer[8];
            uint8_t telemetry_frame[74];
            std::memcpy(&telemetry_buffer[0], &frame[  5-1], 2);
            std::memcpy(&telemetry_buffer[2], &frame[261-1], 2);
            std::memcpy(&telemetry_buffer[4], &frame[517-1], 2);
            std::memcpy(&telemetry_buffer[6], &frame[773-1], 2);

            if (telemetry_deframer.work(telemetry_buffer, telemetry_frame, 8)) {
                sats[SatID::MeteorM22]++;
            } else {
                if (_raw) derand.work(frame, 1024);

                // List of SCIDs here (excluding FY) https://sanaregistry.org/r/spacecraftid/
                uint8_t SCID = ((uint16_t)frame[4] << 8 | frame[5]) >> 6;
                switch (SCID) {
                    case 0x0B: sats[SatID::MetOpB]++;    break; // MetOp 1
                    case 0x0C: sats[SatID::MetOpA]++;    break; // MetOp 2
                    case 0x0D: sats[SatID::MetOpC]++;    break; // MetOp 3
                    case 0x31: sats[SatID::FengYun3C]++; break;
                    case 0x32: sats[SatID::FengYun3B]++; break;
                    case 0x33: sats[SatID::FengYun3C]++; break;
                    default:                             break;
                }
            }

            auto pair = std::max_element(sats.begin(), sats.end(), [](const std::pair<SatID, size_t>& p1, const std::pair<SatID, size_t>& p2) {
                return p1.second < p2.second;
            });
            if (pair->second > 100 && sats.size() != 0) {
                return pair->first;
            }

            return SatID::Unknown;
        }
    private:
        const bool _raw;
        ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 74 * 8> telemetry_deframer;

        ccsds::Derand derand;
        std::map<SatID, size_t> sats;
};

SatID Fingerprint::fingerprint_ccsds(std::istream &stream) {
    CCSDSFingerprint fingerprint(false);
    uint8_t frame[1024];

    while (is_running && !stream.eof()) {
        stream.read(reinterpret_cast<char *>(frame), 1024);

        SatID id = fingerprint.processFrame(frame);
        if (id != SatID::Unknown) {
            return id;
        }
    }

    return SatID::Unknown;
}

SatID Fingerprint::fingerprint_vcdu(std::istream &stream) {
    CCSDSFingerprint fingerprint(false);
    uint8_t frame[1024];

    while (is_running && !stream.eof()) {
        stream.read((char *)&frame[4], 892);

        SatID id = fingerprint.processFrame(frame);
        if (id != SatID::Unknown) {
            return id;
        }
    }

    return SatID::Unknown;
}

SatID Fingerprint::fingerprint_ccsds_raw(std::istream &stream) {
    ccsds::Deframer deframer;
    CCSDSFingerprint fingerprint(true);
    uint8_t frame[1024];
    uint8_t buffer[1024];

    while (is_running && !stream.eof()) {
        stream.read(reinterpret_cast<char *>(buffer), 1024);

        if (deframer.work(buffer, frame, 1024)) {
            SatID id = fingerprint.processFrame(frame);
            if (id != SatID::Unknown) {
                return id;
            }
        }
    }

    return SatID::Unknown;
}

bool Fingerprint::is_noaa(std::istream &stream) {
    uint8_t buffer[1024];
    ccsds::Deframer deframer;
    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> deframer2(8, true);
    std::vector<uint8_t> ccsds_frame(1024);
    std::vector<uint8_t> noaa_frame((11090*10) / 8);

    size_t noaa_frames = 0;
    size_t ccsds_frames = 0;

    while (is_running && !stream.eof()) {
        stream.read(reinterpret_cast<char *>(buffer), 1024);
        if (deframer.work(buffer, ccsds_frame.data(), 1024)) {
            if (ccsds_frames++ > 200) {
                return false;
            }
        }
        if (deframer2.work(buffer, noaa_frame.data(), 1024)) {
            if (noaa_frames++ > 10) {
                return true;
            }
        }
    }

    return false;
}

SatID Fingerprint::id_noaa(std::istream &stream, bool hrp) {
    std::vector<uint16_t> repacked(11090);
    std::map<SatID, size_t> sats;
    
    while (is_running && !stream.eof()) {
        stream.read(reinterpret_cast<char *>(repacked.data()), 11090*2);

        if (hrp) {
            for (size_t i = 0; i < 11090; i++) {
                uint16_t x = repacked[i];
                repacked[i] = (x & 0xFF) << 8 | (x >> 8);
            }
        }

        uint8_t address = ((repacked[6] & 0x078) >> 3) & 0x000F;
        switch (address) {
            case 7:  sats[SatID::NOAA15]++; break;
            case 13: sats[SatID::NOAA18]++; break;
            case 15: sats[SatID::NOAA19]++; break;
            default:                        break;
        }

        auto pair = std::max_element(sats.begin(), sats.end(), [](const std::pair<SatID, size_t>& p1, const std::pair<SatID, size_t>& p2) {
            return p1.second < p2.second;
        });
        if (pair->second > 20) {
            return pair->first;
        }
    }

    return SatID::Unknown;
}

SatID Fingerprint::id_noaa_raw(std::istream &stream) {
    uint8_t buffer[1024];
    std::vector<uint8_t> frame((11090*10) / 8);
    std::vector<uint16_t> repacked(11090);
    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> deframer(8, true);

    std::map<SatID, size_t> sats;

    while (is_running && !stream.eof()) {
        stream.read(reinterpret_cast<char *>(buffer), 1024);
        if (deframer.work(buffer, frame.data(), 1024)) {
            size_t j = 0;
            for (size_t i = 0; i < 11090-3; i += 4) {
                repacked[i + 0] =  (frame[j + 0] << 2)       | (frame[j + 1] >> 6);
                repacked[i + 1] = ((frame[j + 1] % 64) << 4) | (frame[j + 2] >> 4);
                repacked[i + 2] = ((frame[j + 2] % 16) << 6) | (frame[j + 3] >> 2);
                repacked[i + 3] = ((frame[j + 3] % 4 ) << 8) |  frame[j + 4];
                j += 5;
            }

            uint8_t address = ((repacked[6] & 0x078) >> 3) & 0x000F;
            switch (address) {
                case 7:  sats[SatID::NOAA15]++; break;
                case 13: sats[SatID::NOAA18]++; break;
                case 15: sats[SatID::NOAA19]++; break;
                default:                        break;
            }

            auto pair = std::max_element(sats.begin(), sats.end(), [](const std::pair<SatID, size_t>& p1, const std::pair<SatID, size_t>& p2) {
                return p1.second < p2.second;
            });
            if (pair->second > 20) {
                return pair->first;
            }
        }
    }

    return SatID::Unknown;
}

std::pair<SatID, FileType> Fingerprint::file(std::string filename) {
    std::filebuf file;
    if (!file.open(filename, std::ios::in | std::ios::binary)) {
        return {SatID::Unknown, FileType::Unknown};
    }
    std::istream stream(&file);

    std::string extension = filename.substr(filename.find(".")+1);
    if (extension == "cadu") {
        SatID id = fingerprint_ccsds(stream);
        file.close();
        return {id, FileType::CADU};
    } else if (extension == "vcdu") {
        SatID id = fingerprint_vcdu(stream);
        file.close();
        return {id, FileType::VCDU};
    } else if (extension == "raw16") {
        SatID id = id_noaa(stream);
        file.close();
        return {id, FileType::raw16};
    } else if (extension == "hrp") {
        SatID id = id_noaa(stream, true);
        file.close();
        return {id, FileType::HRP};
    }

    if (is_ccsds(stream)) {
        SatID id = fingerprint_ccsds(stream);
        file.close();
        return {id, FileType::CADU};
    }

    if (is_raw16(stream)) {
        SatID id = id_noaa(stream);
        file.close();
        return {id, FileType::raw16};
    }

    if (is_hrp(stream)) {
        SatID id = id_noaa(stream, true);
        file.close();
        return {id, FileType::HRP};
    }

    if (is_noaa(stream)) {
        SatID id = id_noaa_raw(stream);
        file.close();
        return {id, FileType::Raw};
    }

    SatID id = fingerprint_ccsds_raw(stream);
    file.close();
    return {id, FileType::Raw};
}
