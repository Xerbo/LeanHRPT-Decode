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

#include "calibration.h"

#include <QLocale>

#include "util.h"

static double str2double(std::string str) {
    QLocale l(QLocale::C);
    return l.toDouble(QString::fromStdString(str));
}

void Calibrator::calibrate(SatID id, Imager imager, std::vector<QImage> &channels) {
    // MSU-MR Calibration
#if 0
    if (imager == Imager::MSUMR && d_caldata.count("wl1_sum")) {
        for (size_t i = 0; i < 3; i++) {
            double wl = d_caldata["wl" + std::to_string(i + 1) + "_sum"] / d_caldata["n"] * 4.0;
            double bl = d_caldata["bl" + std::to_string(i + 1) + "_sum"] / d_caldata["n"] * 4.0;

            // Solve for a linear equasion which maps bl to 0 and wl to 100
            double a = (100.0 - 0.0) / (wl - bl);
            double b = 0.0 - bl * a;
            calibrate_linear(i + 1, channels[i], a, b);
        }
    }
#endif

    for (size_t i = 0; i < channels.size(); i++) {
        std::string name = satellite_info.at(id).name + "_" + sensor_info.at(imager).name + "/" + std::to_string(i + 1);
        if (!config.sections.count(name)) continue;

        std::map<std::string, std::string> coefficients = config.sections.at(name);
        std::string type = "linear";
        if (coefficients.count("type")) {
            type = coefficients.at("type");
        }

        if (type == "linear") {
            double a = str2double(coefficients.at("a"));
            double b = str2double(coefficients.at("b"));
            calibrate_linear(i + 1, channels[i], a, b);
        } else if (type == "split_linear") {
            double a1 = str2double(coefficients.at("a1"));
            double b1 = str2double(coefficients.at("b1"));
            double a2 = str2double(coefficients.at("a2"));
            double b2 = str2double(coefficients.at("b2"));
            double c = str2double(coefficients.at("c"));
            calibrate_split_linear(i + 1, channels[i], a1, b1, a2, b2, c);
        } else if (type == "radiance") {
            double ns = str2double(coefficients.at("ns"));
            double b0 = str2double(coefficients.at("b0"));
            double b1 = str2double(coefficients.at("b1"));
            double b2 = str2double(coefficients.at("b2"));
            double vc = str2double(coefficients.at("vc"));
            double a = str2double(coefficients.at("a"));
            double b = str2double(coefficients.at("b"));
            calibrate_ir(i + 1, channels[i], ns, b0, b1, b2, vc, a, b);
        }
    }
}

void Calibrator::calibrate_linear(size_t ch, QImage &image, double a, double b) {
    (void)ch;

    for (int y = 0; y < image.height(); y++) {
        quint16 *line = reinterpret_cast<quint16 *>(image.scanLine(y));

        for (int x = 0; x < image.width(); x++) {
            double count = line[x] / 64;
            count = a * count + b;
            line[x] = clamp(count / 100.0, 0.0, 1.0) * UINT16_MAX;
        }
    }
}

void Calibrator::calibrate_split_linear(size_t ch, QImage &image, double a1, double b1, double a2, double b2, double c) {
    for (int y = 0; y < image.height(); y++) {
        quint16 *line = reinterpret_cast<quint16 *>(image.scanLine(y));
        if (d_ch3a.size() != 0 && !d_ch3a[y] && ch == 3) continue;

        for (int x = 0; x < image.width(); x++) {
            double count = line[x] / 64;
            if (count < c) {
                count = a1 * count + b1;
            } else {
                count = a2 * count + b2;
            }
            line[x] = clamp(count / 100.0, 0.0, 1.0) * UINT16_MAX;
        }
    }
}

void Calibrator::calibrate_ir(size_t ch, QImage &image, double Ns, double b0, double b1, double b2, double Vc, double A,
                              double B) {
    double Tbb = d_caldata["blackbody_temperature_sum"] / d_caldata["n"];  // Blackbody temperature
    double Tbbstar = A + B * Tbb;                                          // Effective blackbody temperature

    const double c1 = 1.1910427e-5;                                 // mW/(m^2-sr-cm^-4)
    const double c2 = 1.4387752;                                    // cm-K
    double Nbb = c1 * pow(Vc, 3) / (exp(c2 * Vc / Tbbstar) - 1.0);  // Blackbody radiance

    double Cs = d_caldata["ch" + std::to_string(ch) + "_space"] / d_caldata["n"];  // Average space count
    double Cbb = d_caldata["ch" + std::to_string(ch) + "_cal"] / d_caldata["n"];   // Average backscan count

    for (int y = 0; y < image.height(); y++) {
        quint16 *line = reinterpret_cast<quint16 *>(image.scanLine(y));

        for (int x = 0; x < image.width(); x++) {
            double Ce = line[x] / 64;                                // Earth count
            double Nlin = Ns + (Nbb - Ns) * (Cs - Ce) / (Cs - Cbb);  // Linear radiance estimate
            double Ncor = b0 + b1 * Nlin + b2 * pow(Nlin, 2);        // Non-linear correction
            double Ne = Nlin + Ncor;                                 // Radiance

            double Testar = c2 * Vc / log(c1 * pow(Vc, 3) / Ne + 1.0);  // Equivalent black body temperature
            double Te = (Testar - A) / B;                               // Temperature (kelvin)

            // Convert to celsius
            Te -= 273.15;

            Te = (60.0 - Te) / 160.0 * (double)UINT16_MAX;
            line[x] = clamp(Te, 0.0, (double)UINT16_MAX);
        }
    }
}
