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

#include <fstream>
#include <inipp.h>
#include <iostream>

// Load a config file, first try looking in the current directory and then the config path
class Config : public inipp::Ini<char> {
    public:
        Config(std::string filename) {
            std::filebuf file;

            if (file.open(filename, std::ios::in)) {
                std::istream stream(&file);
                parse(stream);
                file.close();
                return;
            }

            if (file.open(get_config_prefix() + filename, std::ios::in)) {
                std::istream stream(&file);
                parse(stream);
                file.close();
                return;
            }

            std::cerr << "Could not open " << filename << std::endl;
        }
    private:
        static std::string get_config_prefix() {
#ifdef _WIN32
            std::string home = std::getenv("APPDATA");
            return home + "\\LeanHRPT\\";
#else
            std::string home = std::getenv("HOME");
            return home + "/.config/leanhrpt/";
#endif
        }
};

#endif
