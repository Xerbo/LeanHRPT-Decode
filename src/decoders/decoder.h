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

#ifndef LEANHRPT_DECODERS_DECODER_H_
#define LEANHRPT_DECODERS_DECODER_H_

#include <QDateTime>
#include <QFileInfo>
#include <atomic>
#include <fstream>
#include <istream>

#include "protocol/rawimage.h"
#include "satinfo.h"

#define BUFFER_SIZE 1024

struct Data {
    std::map<Imager, RawImage *> imagers;
    std::map<Imager, std::vector<double>> timestamps;
    std::map<std::string, double> caldata;
    std::vector<bool> ch3a;
};

enum class FileType { Raw, CADU, VCDU, raw16, HRP, TIP, Unknown };

class Decoder {
   public:
    Decoder() : is_running(true) { buffer = new uint8_t[BUFFER_SIZE]; }
    virtual ~Decoder() {
        for (auto image : images) {
            delete image.second;
        }
        delete[] buffer;
    }

    /// Decode a file
    bool decodeFile(std::string filename, FileType filetype) {
        d_filetype = filetype;
        std::filebuf file;
        if (!file.open(filename, std::ios::in | std::ios::binary)) {
            return false;
        }
        std::istream stream(&file);
        get_filesize(stream);
        QDateTime _created = QFileInfo(QString::fromStdString(filename)).birthTime().toUTC();
        if (!_created.isValid()) _created = QFileInfo(QString::fromStdString(filename)).lastModified().toUTC();
        if (!_created.isValid()) _created = QDateTime::currentDateTimeUtc();
        created = _created.toSecsSinceEpoch();

        while (is_running && !stream.eof()) {
            work(stream);
            read = stream.tellg();
        }

        file.close();
        return true;
    }
    /// Current decode progress (0 to 1)
    float progress() { return static_cast<float>(read) / static_cast<float>(filesize); }

    /**
     * Stop the current decode
     *
     * This is only used when the decoder is running on another thread
     */
    void stop() { is_running = false; }

    /// Get the produced data
    Data get() { return {images, timestamps, caldata, ch3a}; }

    /// Automatically make/allocate a decoder given satid
    static Decoder *make(Protocol protocol, SatID sat);

   protected:
    uint8_t *buffer;
    std::map<Imager, RawImage *> images;
    std::map<Imager, std::vector<double>> timestamps;
    std::map<std::string, double> caldata;
    std::vector<bool> ch3a;
    virtual void work(std::istream &stream) = 0;
    FileType d_filetype;
    time_t created;

   private:
    std::atomic<bool> is_running;
    size_t read = 0;
    size_t filesize = 1;

    void get_filesize(std::istream &stream) {
        // Get filesize
        stream.seekg(0, std::ios::end);
        filesize = stream.tellg();

        // Seek back to the beginning
        stream.seekg(stream.beg);
    }
};

#endif
