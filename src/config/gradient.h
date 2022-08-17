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

#ifndef GRADIENT_H
#define GRADIENT_H

#include <QString>
#include <map>
#include <string>

#include "config.h"

struct Gradient {
    std::string author;
    std::vector<QColor> stops;
};

class GradientManager {
   public:
    GradientManager() { reload(); }
    void reload() {
        Config ini("gradients.ini");
        gradients.clear();

        for (auto &i : ini.sections) {
            std::vector<QColor> stops;

            for (const auto &x : QString::fromStdString(i.second["map"]).split(",")) {
                stops.push_back(QColor("#" + x));
            }

            gradients.insert({i.first, Gradient{i.second["author"], stops}});
        }
    }

    std::map<std::string, Gradient> gradients;
};

#endif
