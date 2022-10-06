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

#ifndef LEANHRPT_CALIBRATION_H_
#define LEANHRPT_CALIBRATION_H_

#include <QImage>

#include "config/config.h"
#include "satinfo.h"

class Calibrator {
   public:
    Calibrator(std::map<std::string, double> caldata, std::vector<bool> ch3a = {})
        : config("calibration.ini"), d_caldata(caldata), d_ch3a(ch3a){};
    void calibrate(SatID id, Imager imager, std::vector<QImage> &channel);

   private:
    Config config;
    std::map<std::string, double> d_caldata;
    std::vector<bool> d_ch3a;

    void calibrate_linear(size_t ch, QImage &image, double a, double b);
    void calibrate_split_linear(size_t ch, QImage &image, double a1, double b1, double a2, double b2, double c);
    void calibrate_ir(size_t ch, QImage &image, double Ns, double b0, double b1, double b2, double Vc, double A, double B);
};

#endif
