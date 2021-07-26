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

#ifndef LEANHRPT_PRESET_H
#define LEANHRPT_PRESET_H

#include <map>
#include <set>
#include <string>
#include <fstream>
#include <inipp.h>

#include "satinfo.h"

struct Preset {
    std::string description;
    std::string category;
    std::string author;
    std::set<Satellite> satellites;
    std::string expression;
};

class PresetManager {
    public:
        PresetManager(std::string filename) : is(filename) {
            ini.parse(is);
            parse();
        }
        ~PresetManager() {
            is.close();
        }
        void reload() {
            is.seekg(is.beg);
            ini.parse(is);
            parse();
        }

        std::map<std::string, Preset> presets;
    private:
        std::ifstream is;
        inipp::Ini<char> ini;

        void parse() {
            presets.clear();

            for (auto &i : ini.sections) {
                try {
                    Preset preset = {
                        i.second["description"],
                        i.second["category"],
                        i.second["author"],
                        parse_satellites(i.second["satellites"]),
                        i.second["expression"]
                    };
                    presets.insert(std::pair<std::string, Preset>(i.first, preset));
                } catch (std::out_of_range &e) {
                    std::cerr << "Syntax error in preset \"" << i.first << "\"" << std::endl;
                }
            }

            if (presets.size() == 0) {
                Preset preset = { "", "", "", { MetOp, NOAA, FengYun, Meteor }, "bw(0)" };
                presets.insert(std::pair<std::string, Preset>("Unable to load presets", preset));
            }
        }

        std::set<Satellite> parse_satellites(std::string str) {
            std::set<Satellite> satellites;
            std::map<std::string, Satellite> table = {
                {"MetOp",   Satellite::MetOp  },
                {"NOAA",    Satellite::NOAA   },
                {"FengYun", Satellite::FengYun},
                {"Meteor",  Satellite::Meteor }
            };

            std::stringstream stream(str);
            std::string sat;
            while (std::getline(stream, sat, '|')) {
                satellites.insert(table[sat]);
            }

            return satellites;
        } 
};

#endif
