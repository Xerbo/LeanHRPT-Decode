#include "fingerprint.h"

#include <algorithm>
#include <vector>
#include <cstring>
#include <set>

#include "ccsds/deframer.h"
#include "generic/deframer.h"

// Finds the most common value of T
template<typename T>
class Scoreboard {
    public:
        Scoreboard(size_t threshold = 100) : d_threshold(threshold) { }

        void add(T id, size_t weight = 1) {
            ids[id] += weight;
        }
        T max() {
            auto largest = std::max_element(ids.begin(), ids.end(), [](const auto& p1, const auto& p2) { return p1.second < p2.second; });
            if (ids.size() != 0 && largest->second > d_threshold) {
                return largest->first;
            } else {
                return T::Unknown;
            }
        }
    private:
        const size_t d_threshold;
        std::map<T, size_t> ids;
};

std::tuple<SatID, FileType, Protocol> Fingerprint::file(std::string filename) {
    std::filebuf file;
    if (!file.open(filename, std::ios::in | std::ios::binary)) {
        return {SatID::Unknown, FileType::Unknown, Protocol::Unknown};
    }
    std::istream stream(&file);

    std::string extension = QFileInfo(QString::fromStdString(filename)).suffix().toLower().toStdString();
    FileType filetype = FileType::Unknown;

    // Try and guess the filetype based off extension and (if failing that, the) header
    if (known_extensions.count(extension)) {
        filetype = known_extensions.at(extension);
    } else {
        filetype = id_magic(stream);
    }

    switch (filetype) {
        case FileType::CADU:
        case FileType::VCDU: {
            SatID id = fingerprint_ccsds(stream, filetype);
            file.close();
            return {id, filetype, *ccsds_downlinks(id).begin() };
        }
        case FileType::raw16:
        case FileType::HRP: {
            SatID id = fingerprint_noaa(stream, filetype);
            file.close();
            return {id, filetype, Protocol::HRPT};
        }
        default: break;
    }

    // Brute force until we find meteor frames or noaa frames
    switch (fingerprint_raw(stream)) {
        case Protocol::HRPT: {
            SatID id = fingerprint_noaa(stream, FileType::Raw);
            file.close();
            return {id, FileType::Raw, Protocol::HRPT};
        }
        case Protocol::MeteorHRPT: {
            SatID id = fingerprint_meteor(stream, FileType::Raw);
            file.close();
            return {id, FileType::Raw, Protocol::MeteorHRPT};
        }
        default: break;
    }

    file.close();
    return {SatID::Unknown, FileType::Unknown, Protocol::Unknown};
}

SatID Fingerprint::fingerprint_ccsds(std::istream &stream, FileType type) {
    Scoreboard<SatID> s;
    uint8_t frame[1024];

    while (is_running && !stream.eof()) {
        if (type == FileType::VCDU) {
            stream.read((char *)&frame[4], 892);
        } else if (type == FileType::CADU) {
            stream.read(reinterpret_cast<char *>(frame), 1024);
        }

        uint8_t SCID = ((uint16_t)frame[4] << 8 | frame[5]) >> 6;
        switch (SCID) {
            case 0x00: s.add(SatID::MeteorM2);  break; // This is fine until the launch of Meteor-M23
            case 0x0B: s.add(SatID::MetOpB);    break; // MetOp 1
            case 0x0C: s.add(SatID::MetOpA);    break; // MetOp 2
            case 0x0D: s.add(SatID::MetOpC);    break; // MetOp 3
            case 0x31: s.add(SatID::FengYun3A); break;
            case 0x32: s.add(SatID::FengYun3B); break;
            case 0x33: s.add(SatID::FengYun3C); break;
            default:                            break;
        }

        if (s.max() != SatID::Unknown) return s.max();
    }

    return SatID::Unknown;
}

Protocol Fingerprint::fingerprint_raw(std::istream &stream) {
    uint8_t buffer[1024];
    ccsds::Deframer ccsds_deframer;
    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> noaa_deframer(8, true);
    std::vector<uint8_t> out((11090*10) / 8);
    Scoreboard<Protocol> s;

    while (is_running && !stream.eof()) {
        stream.read(reinterpret_cast<char *>(buffer), 1024);
        if (ccsds_deframer.work(buffer, out.data(), 1024)) {
            s.add(Protocol::MeteorHRPT);
        }
        if (noaa_deframer.work(buffer, out.data(), 1024)) {
            s.add(Protocol::HRPT, 14);
        }

        if (s.max() != Protocol::Unknown) return s.max();
    }

    return Protocol::Unknown;
}

SatID Fingerprint::fingerprint_noaa(std::istream &stream, FileType type) {
    std::vector<uint16_t> frame(11090);
    Scoreboard<SatID> s;
    ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900> deframer(8, true);

    while (is_running && !stream.eof()) {
        bool have_frame = false;

        if (type == FileType::raw16) {
            stream.read((char *)frame.data(), 11090 * sizeof(uint16_t));
            have_frame = true;
        } else if (type == FileType::HRP) {
            stream.read((char *)frame.data(), 11090 * sizeof(uint16_t));
            have_frame = true;

            // Flip endianness
            for (size_t i = 0; i < 11090; i++) {
                uint16_t x = frame[i];
                frame[i] = (x & 0xFF) << 8 | (x >> 8);
            }
        } else if (type == FileType::Raw) {
            std::vector<uint8_t> buffer(1024);
            std::vector<uint8_t> raw((11090*10)/8);

            stream.read((char *)buffer.data(), 1024);
            if (deframer.work(buffer.data(), raw.data(), 1024)) {
                size_t j = 0;
                for (size_t i = 0; i < 11090-3; i += 4) {
                    frame[i + 0] =  (raw[j + 0] << 2)       | (raw[j + 1] >> 6);
                    frame[i + 1] = ((raw[j + 1] % 64) << 4) | (raw[j + 2] >> 4);
                    frame[i + 2] = ((raw[j + 2] % 16) << 6) | (raw[j + 3] >> 2);
                    frame[i + 3] = ((raw[j + 3] % 4 ) << 8) |  raw[j + 4];
                    j += 5;
                }

                have_frame = true;
            }
        }

        if (have_frame) {
            uint8_t address = (frame[6] >> 3) & 0b1111;
            switch (address) {
                case 7:  s.add(SatID::NOAA15); break;
                case 13: s.add(SatID::NOAA18); break;
                case 15: s.add(SatID::NOAA19); break;
                default:                       break;
            }

            if (s.max() != SatID::Unknown) return s.max();
        }
    }

    return SatID::Unknown;
}

SatID Fingerprint::fingerprint_meteor(std::istream &stream, FileType type) {
    Scoreboard<SatID> s;
    ccsds::Deframer deframer;
    std::vector<uint8_t> frame(1024);
    ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 11850 * 8> msumr_deframer;

    while (is_running && !stream.eof()) {
        bool have_frame = false;

        if (type == FileType::Raw) {
            std::vector<uint8_t> buffer(1024);
            stream.read((char *)buffer.data(), 1024);

            if (deframer.work(buffer.data(), frame.data(), 1024)) {
                have_frame = true;
            }
        }

        if (have_frame) {
            uint8_t msumr_buffer[948];
            std::vector<uint8_t> msumr_frame(11850);
            std::memcpy(&msumr_buffer[238*0], &frame[ 23-1], 238);
            std::memcpy(&msumr_buffer[238*1], &frame[279-1], 238);
            std::memcpy(&msumr_buffer[238*2], &frame[535-1], 238);
            std::memcpy(&msumr_buffer[238*3], &frame[791-1], 234);
            if (msumr_deframer.work(msumr_buffer, msumr_frame.data(), 948)) {
                switch (msumr_frame[12] >> 4) {
                    case 0: s.add(SatID::MeteorM2);  break;
                    case 2: s.add(SatID::MeteorM22); break;
                    default:                         break;
                }

                if (s.max() != SatID::Unknown) return s.max();
            }
        }
    }

    return SatID::Unknown;
}

std::set<Protocol> Fingerprint::ccsds_downlinks(SatID id) {
    if (satellite_info.count(id) == 0) {
        return { Protocol::Unknown };
    }

    switch (satellite_info.at(id).mission) {
        case POES:     return { Protocol::HRPT };
        case FengYun3: return { Protocol::FengYunHRPT };
        case MetOp:    return { Protocol::AHRPT/*, Protocol::LRPT*/ };
        case MeteorM:  return { /*Protocol::MeteorHRPT,*/ Protocol::LRPT };
        default:       return { Protocol::Unknown };
    }
}
