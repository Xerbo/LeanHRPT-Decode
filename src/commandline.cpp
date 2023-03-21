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

#include "commandline.h"

#include <QDir>
#include <QLocale>
#include <array>
#include <iostream>

#include "config/preset.h"
#include "decoders/decoder.h"
#include "fingerprint.h"
#include "geometry.h"
#include "image/compositor.h"
#include "map.h"
#include "network.h"
#include "protocol/timestamp.h"
#include "satinfo.h"

static ulong str2ulong(QString str) {
    QLocale l(QLocale::C);
    return l.toULong(str);
}

int parseCommandLine(QCommandLineParser &parser) {
    QString filename = parser.positionalArguments().first();
    if (filename.isEmpty()) return 1;

    QString outdir = parser.value("out");
    if (!outdir.isEmpty() && !QDir(outdir).exists()) {
        std::cout << "Output directory doesn't exist, creating it" << std::endl;
        QDir().mkdir(outdir);
    }

    bool have_map = false;
    bool have_landmarks = false;
    std::array<std::vector<QLineF>, 36 * 18> map_buckets;
    std::vector<Landmark> landmarks;

    QString shapefile_path = parser.value("map");
    if (!shapefile_path.isEmpty()) {
        if (!map::verify_shapefile(shapefile_path.toStdString())) {
            std::cout << "Could not open shapefile" << std::endl;
            return 1;
        }

        map_buckets = map::index_line_segments(map::read_shapefile(shapefile_path.toStdString()));
        have_map = true;
    }

    QString landmark_path = parser.value("landmark");
    if (!landmark_path.isEmpty()) {
        landmarks = map::read_landmarks(landmark_path.toStdString());
        if (landmarks.size() != 0) {
            have_landmarks = true;
        }
    }

    TLEManager *tles;
    if (have_map || have_landmarks) {
        tles = new TLEManager;
    }

    std::cout << "Fingerprinting \"" << filename.toStdString() << "\"" << std::endl;

    SatID sat;
    FileType type;
    Protocol protocol;
    std::tie(sat, type, protocol) = Fingerprint().file(filename.toStdString(),Suggestion::Automatic);

    if (sat == SatID::Unknown) {
        std::cout << "Unable to identify satellite" << std::endl;
        return 1;
    } else {
        std::cout << "Satellite is " << satellite_info.at(sat).name << std::endl;
    }

    Decoder *decoder = Decoder::make(protocol, sat);

    std::cout << "Decoding" << std::endl;

    decoder->decodeFile(filename.toStdString(), type);
    Data data = decoder->get();
    std::cout << "Finished decoding" << std::endl;

    // Timestamp is the middle of the pass on the main imager
    for (auto &sensor : data.timestamps) {
        std::vector<double> &timestamp = sensor.second;

        if (protocol == Protocol::GACReverse) {
            reverse(timestamp);
        }

        timestamp = filter_timestamps(timestamp);
    }

    QDateTime timestamp;
    {
        Imager default_imager = satellite_info.at(sat).default_imager;
        std::vector<double> medianv = data.timestamps[default_imager];
        std::sort(medianv.begin(), medianv.end());
        medianv.erase(std::remove(medianv.begin(), medianv.end(), 0.0), medianv.end());
        timestamp.setSecsSinceEpoch(medianv[medianv.size() / 2]);
    }

    inipp::Ini<char> ini;
    if (parser.isSet("ini")) {
        std::ifstream ifs(parser.value("ini").toStdString());
        if (!ifs.is_open()) {
            std::cout << "Could not open composite definition file" << std::endl;
            return 1;
        }
        ini.parse(ifs);
        ifs.close();
    } else {
        // clang-format off
        std::map<std::string, std::string> settings = {
            { "sensors", "AVHRR|MSU-MR|VIRR|MHS" },
            { "preset", "Automatic" }
        };
        // clang-format on
        ini.sections["{sat}_{time}_{sensor}_Automatic.png"] = settings;
        settings["equalization"] = "histogram";
        ini.sections["{sat}_{time}_{sensor}_Automatic_EQU.png"] = settings;
        settings["equalization"] = "stretch";
        ini.sections["{sat}_{time}_{sensor}_Automatic_CONT.png"] = settings;

        // clang-format off
        settings = {
            { "sensors", "AVHRR|MSU-MR|VIRR" },
            { "preset", "Thermal" }
        };
        // clang-format on
        ini.sections["{sat}_{time}_{sensor}_Thermal.png"] = settings;
        settings["equalization"] = "histogram";
        ini.sections["{sat}_{time}_{sensor}_Thermal_EQU.png"] = settings;
        settings["equalization"] = "stretch";
        ini.sections["{sat}_{time}_{sensor}_Thermal_CONT.png"] = settings;
    }

    PresetManager preset_manager;

    std::map<Imager, ImageCompositor> compositors;
    for (auto &sensor_data : data.imagers) {
        Imager imager = sensor_data.first;
        RawImage *image = sensor_data.second;

        compositors[imager].import(image, sat, imager, data.caldata);
        compositors[imager].ch3a = data.ch3a;

        for (auto &file : ini.sections) {
            // Replace template strings in filename
            QString filename = QString::fromStdString(file.first);
            filename = filename.replace("{sat}", QString::fromStdString(satellite_info.at(sat).name));
            filename = filename.replace("{time}", timestamp.toString("yyyyMMdd-hhmmss"));
            filename = filename.replace("{sensor}", QString::fromStdString(sensor_info.at(imager).name));

            // Parse settings
            std::string sensors;
            if (file.second.count("sensors")) {
                sensors = file.second["sensors"];
            } else {
                std::cout << "Image \"" << file.first << "\" doesn't specify what sensors it works on, skipping" << std::endl;
                continue;
            }
            std::string equalization = file.second.count("equalization") ? file.second["equalization"] : "none";
            std::string brightnessonly = file.second.count("brightnessonly") ? file.second["brightnessonly"] : "true";
            std::string corrected = file.second.count("corrected") ? file.second["corrected"] : "true";

            // Skip if it doesn't work for this imager
            if (sensors.find(sensor_info.at(imager).name) == std::string::npos) {
                continue;
            }

            // Generate the image
            QImage image(compositors[imager].width(), compositors[imager].height(), QImage::Format_RGBX64);
            if (file.second.count("preset")) {
                // Preset
                Preset preset = preset_manager.presets.at(file.second["preset"]);
                std::string expression = preset.expression;
                if (preset.overrides.count(imager)) {
                    expression = preset.overrides.at(imager);
                }
                compositors[imager].getExpression(image, expression);
            } else if (file.second.count("channel")) {
                // Single channel
                compositors[imager].getChannel(image, str2ulong(QString::fromStdString(file.second["channel"])));
            } else if (file.second.count("composite")) {
                // RGB composite
                QStringList _channels = QString::fromStdString(file.second["composite"]).split(",");
                std::array<size_t, 3> channels = {str2ulong(_channels[0]), str2ulong(_channels[1]), str2ulong(_channels[2])};
                compositors[imager].getComposite(image, channels);
            } else {
                std::cout << "Image \"" << file.first << "\" has no source, skipping" << std::endl;
                continue;
            }

            // Equalize the image
            if (equalization == "histogram") {
                ImageCompositor::equalise(image, Equalization::Histogram, 1.0f, brightnessonly == "true");
            } else if (equalization == "stretch") {
                ImageCompositor::equalise(image, Equalization::Stretch, 1.0f, brightnessonly == "true");
            } else if (equalization == "none") {
                ImageCompositor::equalise(image, Equalization::None, 1.0f, brightnessonly == "true");
            } else {
                std::cout << "Image \"" << file.first << "\" uses an unknown equalization, skipping" << std::endl;
                continue;
            }

            if (have_map || have_landmarks) {
                size_t width = compositors[imager].width();
                size_t height = compositors[imager].height();

                Projector projector(tles->catalog_by_norad[(int)sat]);
                std::vector<std::pair<xy, Geodetic>> gcps =
                    projector.calculate_gcps(data.timestamps[imager], (height / width) * 21, 21, imager, sat, width);
                if (gcps.size() == 0) {
                    break;
                }

                if (have_map) {
                    compositors[imager].overlay = map::warp_to_pass(map_buckets, gcps, 21);
                    compositors[imager].enable_map = true;
                    compositors[imager].map_color = QColor(255, 255, 0);
                }
                if (have_landmarks) {
                    compositors[imager].landmarks = map::warp_to_pass(landmarks, gcps, 21);
                    compositors[imager].enable_landmarks = true;
                    compositors[imager].landmark_color = QColor(255, 0, 0);
                }
            }

            compositors[imager].setFlipped(parser.isSet("flip"));
            compositors[imager].postprocess(image, corrected == "true");

            std::cout << "Writing \"" << filename.toStdString() << "\"" << std::endl;
            image.save(QDir(outdir).filePath(filename));
        }
    }

    return 0;
}
