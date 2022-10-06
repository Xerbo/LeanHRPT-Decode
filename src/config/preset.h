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

#ifndef LEANHRPT_CONFIG_PRESET_H
#define LEANHRPT_CONFIG_PRESET_H

#include <map>
#include <set>
#include <string>

#include "config.h"
#include "satinfo.h"

struct Preset {
    std::string description;
    std::string category;
    std::string author;
    std::set<Imager> imagers;
    std::string expression;
    std::map<Imager, std::string> overrides;
};

class PresetManager {
   public:
    PresetManager() { reload(); }
    void reload() {
        Config ini("presets.ini");
        presets.clear();

        for (auto &i : ini.sections) {
            try {
                std::map<Imager, std::string> overrides;
                for (const auto &x : i.second) {
                    size_t y = x.first.find(":");
                    if (y != std::string::npos) {
                        overrides.insert(std::pair<Imager, std::string>(sensors.at(x.first.substr(y + 1)), x.second));
                    }
                }

                presets.insert(std::pair<std::string, Preset>(
                    i.first, Preset{i.second["description"], i.second["category"], i.second["author"],
                                    parse_imagers(i.second["imagers"]), i.second["expression"], overrides}));
            } catch (std::out_of_range &e) {
                std::cerr << "Syntax error in preset \"" << i.first << "\"" << std::endl;
            }
        }
    }

    std::map<std::string, Preset> presets;

   private:
    std::set<Imager> parse_imagers(std::string str) {
        std::set<Imager> imagers;

        std::stringstream stream(str);
        std::string imager;
        while (std::getline(stream, imager, '|')) {
            imagers.insert(sensors.at(imager));
        }

        return imagers;
    }
};

#endif
