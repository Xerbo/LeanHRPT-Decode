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

#ifndef LEANHRPT_CONFIG_CONFIG_H_
#define LEANHRPT_CONFIG_CONFIG_H_

#include <config/inipp.h>

#include <QStandardPaths>
#include <fstream>
#include <iostream>

inline std::string get_temp_dir() {
#ifdef _WIN32
    return std::getenv("TEMP");
#else
    return "/tmp";
#endif
}

// Load a config file, first try looking in the current directory and then config paths
class Config : public inipp::Ini<char> {
   public:
    Config(std::string filename) {
        // Local (used for development)
        if (try_load(filename)) return;

        // Standard system config directories
        QString config = QStandardPaths::locate(QStandardPaths::ConfigLocation, QString::fromStdString("leanhrpt/" + filename));
        if (try_load(config.toStdString())) return;

#ifndef _WIN32
        // Running as AppImage
        std::string here = std::getenv("HERE") ? std::getenv("HERE") : "";
        if (!here.empty() && try_load(here + "/usr/share/leanhrpt/" + filename)) return;

        if (try_load("/usr/share/leanhrpt/" + filename)) return;
        if (try_load("/usr/local/share/leanhrpt/" + filename)) return;
#endif

        std::cerr << "Could not open " << filename << std::endl;
    }

   private:
    bool try_load(std::string filename) {
        std::filebuf file;
        if (file.open(filename, std::ios::in)) {
            std::istream stream(&file);
            parse(stream);
            interpolate();
            file.close();
            return true;
        }

        return false;
    }
};

#endif
