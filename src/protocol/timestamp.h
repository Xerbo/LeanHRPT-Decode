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

#ifndef LEANHRPT_PROTOCOL_TIMESTAMP_H_
#define LEANHRPT_PROTOCOL_TIMESTAMP_H_

#include <vector>

inline std::vector<double> filter_timestamps(std::vector<double> in) {
    if (in.size() < 20) {
        return {};
    }

    // Main filtering
    std::vector<double> copy = in;
    for (size_t i = 9; i < in.size() - 9; i++) {
        double average = 0.0;
        for (int j = 0; j < 19; j++) {
            average += copy[i + (j - 9)];
        }
        average /= 19.0;

        if (fabs(copy[i] - average) > 2.0) {
            in[i] = 0;
        }
    }

    // Remove the first and last 9 readings because these aren't filtered above
    for (size_t i = 0; i < 9; i++) {
        in[i] = 0;
    }
    for (size_t i = in.size() - 9; i < in.size(); i++) {
        in[i] = 0;
    }

    // Find first and last valid timestamp
    int first = -1, last = -1;
    for (size_t i = 0; i < in.size(); i++) {
        if (in[i] != 0.0 && first == -1) {
            first = i;
        }
        if (in[i] != 0.0) {
            last = i;
        }
    }

    // No valid timestamps
    if (first < 0 || last < 0) {
        return {};
    }

    // Average seconds per line over the pass
    double spl = (in[last] - in[first]) / (last - first);

    // Fill in missing timestamps
    for (size_t i = 1; i < in.size(); i++) {
        if (in[i] == 0.0 && in[i - 1] != 0.0) {
            in[i] = in[i - 1] + spl;
        }
    }
    for (int i = in.size() - 2; i >= 0; i--) {
        if (in[i] == 0.0 && in[i + 1] != 0.0) {
            in[i] = in[i + 1] - spl;
        }
    }

    return in;
}

#endif
