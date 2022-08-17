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

#ifndef LEANHRPT_CONFIG_H
#define LEANHRPT_CONFIG_H

#include <config/inipp.h>

#include <fstream>
#include <iostream>

inline std::string get_temp_dir() {
#ifdef _WIN32
    return std::getenv("TEMP");
#else
    return "/tmp";
#endif
}

// Load a config file, first try looking in the current directory and then the config path
class Config : public inipp::Ini<char> {
   public:
    Config(std::string filename) {
        std::filebuf file;

        if (file.open(filename, std::ios::in)) {
            std::istream stream(&file);
            parse(stream);
            interpolate();
            file.close();
            return;
        }

        if (file.open(get_local_prefix() + filename, std::ios::in)) {
            std::istream stream(&file);
            parse(stream);
            interpolate();
            file.close();
            return;
        }

#ifndef _WIN32
        if (file.open(get_system_prefix() + "/share/leanhrpt/" + filename, std::ios::in)) {
            std::istream stream(&file);
            parse(stream);
            interpolate();
            file.close();
            return;
        }
#endif

        std::cerr << "Could not open " << filename << std::endl;
    }

   private:
    static std::string get_local_prefix() {
#ifdef _WIN32
        std::string home = std::getenv("APPDATA");
        return home + "\\LeanHRPT\\";
#else
        std::string home = std::getenv("HOME");
        return home + "/.config/leanhrpt/";
#endif
    }

#ifndef _WIN32
    static std::string get_system_prefix() {
        std::string here = std::getenv("HERE") ? std::getenv("HERE") : "";
        return here.empty() ? "/usr" : (here + "/usr");
    }
#endif
};

#endif
