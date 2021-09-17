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

#ifndef LEANHRPT_DECODERS_DECODER_H
#define LEANHRPT_DECODERS_DECODER_H

#include <istream>
#include <fstream>

#include "satinfo.h"
#include "generic/rawimage.h"

#define BUFFER_SIZE 1024

struct Data {
    std::map<Imager, RawImage *> imagers;
    std::map<std::string, double> coeffs;
};

class Decoder {
    public:
        Decoder() {
            buffer = new uint8_t[BUFFER_SIZE];
        }
        virtual ~Decoder() {
            for (auto image : images) {
                delete image.second;
            }
            delete[] buffer;
        }

        bool decodeFile(std::string filename) {
            std::filebuf file;
            if (!file.open(filename, std::ios::in | std::ios::binary)) {
                return false;
            }
            std::istream stream(&file);
            read_meta(stream);

            while (!stream.eof() && is_running) {
                work(stream);
                read = stream.tellg();
            }

            file.close();
            return true;
        }
        float progress() {
            return static_cast<float>(read) / static_cast<float>(filesize);
        }
        void stop() {
            is_running = false;
        }

        Data get() {
            return { images, coeffs };
        }

    protected:
        uint8_t *buffer;
        std::map<Imager, RawImage *> images;
        std::map<std::string, double> coeffs;
        virtual void work(std::istream &stream)=0;
        bool is_ccsds_frames = false;
        bool is_raw16 = false;

    private:
        bool is_running = true;
        size_t read = 0;
        size_t filesize = 1;

        void read_meta(std::istream &stream) {
            // Determine weather the file is CCSDS frames or not
            uint8_t header[4];
            stream.read(reinterpret_cast<char *>(header), 4);
            is_ccsds_frames = (header[0] == 0x1A && header[1] == 0xCF && header[2] && 0xFC && header[3] == 0x1D);
            is_raw16        = (header[0] == 0x84 && header[1] == 0x02 && header[2] && 0x6F && header[3] == 0x01);

            // Get filesize
            stream.seekg(0, std::ios::end);
            filesize = stream.tellg();

            // Reset
            stream.seekg(stream.beg);
        }
};

#endif
