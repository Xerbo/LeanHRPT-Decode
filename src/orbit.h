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

#ifndef LEANHRPT_ORBIT_H_
#define LEANHRPT_ORBIT_H_

#include <predict/predict.h>

#include <locale>
#include <stdexcept>
#include <string>

// A simple C++ wrapper around libpredict
class OrbitPredictor {
   public:
    OrbitPredictor(std::pair<std::string, std::string> tle) {
        std::setlocale(LC_NUMERIC, "C");
        orbital_elements = predict_parse_tle(tle.first.c_str(), tle.second.c_str());
        std::setlocale(LC_NUMERIC, "");
    }
    ~OrbitPredictor() { predict_destroy_orbital_elements(orbital_elements); }

    /**
     * Gets orbital information at the specified timestamp
     *
     * @param timestamp UNIX timestamp
     * @return A predict_position struct
     */
    struct predict_position predict(double timestamp) {
        // UNIX to Julian date
        predict_julian_date_t prediction_time = (timestamp / 86400.0) - 3651.0;

        struct predict_position orbit;
        if (predict_orbit(orbital_elements, &orbit, prediction_time)) {
            throw std::runtime_error("Error predicting orbit");
        }

        return orbit;
    }

   private:
    predict_orbital_elements_t *orbital_elements;
};

#endif
