/*
 * LeanHRPT Decode
 * Copyright (C) 2021-2022 Xerbo
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

// Load a config file, first try looking in the current directory and then config paths
class Config : public inipp::Ini<char> {
   public:
    Config(std::string filename) {
        // Local config
        if (try_load(filename)) return;

        // User config
        QString config = QStandardPaths::locate(QStandardPaths::ConfigLocation, QString::fromStdString("leanhrpt/" + filename));
        if (!config.isEmpty() && try_load(config.toStdString())) return;

#ifndef _WIN32
        // Internal AppImage config
        std::string here = std::getenv("HERE") ? std::getenv("HERE") : "";
        if (!here.empty() && try_load(here + "/usr/share/leanhrpt/" + filename)) return;
#endif

        // System config
        config = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromStdString("leanhrpt/" + filename));
        if (!config.isEmpty() && try_load(config.toStdString())) return;

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
