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

#include "generic/rawimage.h"

// DO NOT CHANGE, changing will break the ability to read CCSDS frames
#define BUFFER_SIZE 1024

class Decoder {
    public:
        Decoder() {
            buffer = new uint8_t[BUFFER_SIZE];
        }
        virtual ~Decoder() {
            delete image;
            delete[] buffer;
        }

        bool decodeFile(std::string filename) {
            std::filebuf file = std::filebuf();
            if (!file.open(filename, std::ios::in | std::ios::binary)) {
                return false;
            }
            std::istream stream(&file);

            stream.seekg(stream.end);
            filesize = stream.tellg();
            stream.seekg(stream.beg);

            while (!stream.eof() && is_running) {
                stream.read(reinterpret_cast<char *>(buffer), BUFFER_SIZE);
                read += BUFFER_SIZE;
                work();
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

        virtual std::string imagerName() {
            return "Unknown";
        };
        RawImage *getImage() {
            return image;
        };

    protected:
        uint8_t *buffer;
        RawImage *image;
        virtual void work()=0;

    private:
        bool is_running = true;
        size_t read = 0;
        size_t filesize = 1;
};

#endif
