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

#include "commandline.h"
#include "satinfo.h"
#include "fingerprint.h"
#include "imagecompositor.h"
#include "decoders/meteor.h"
#include "decoders/noaa.h"
#include "decoders/fengyun.h"
#include "decoders/metop.h"

#include <iostream>

int parseCommandLine(QCommandLineParser &parser) {
    QString filename = parser.positionalArguments().first();
    if (filename.isEmpty()) return 1;

    std::cout << "Decoding \"" << filename.toStdString() << "\"" << std::endl;

    std::pair<SatID, FileType> info = Fingerprint().file(filename.toStdString());
    SatID sat = info.first;
    if (sat == SatID::Unknown) {
        std::cout << "Unable to identify satellite" << std::endl;
        return 1;
    } else {
        std::cout << "Satellite is " << satellite_info.at(sat).name << std::endl;
    }
    Mission mission = satellite_info.at(sat).mission;

    Decoder *decoder;
    switch (mission) {
        case Mission::FengYun3: decoder = new FengyunDecoder; break;
        case Mission::MeteorM:  decoder = new MeteorDecoder; break;
        case Mission::MetOp:    decoder = new MetOpDecoder; break;
        case Mission::POES:     decoder = new NOAADecoder; break;
        default: throw std::runtime_error("invalid value in enum `Mission`");
    }

    std::cout << "Starting decode" << std::endl;;
    decoder->decodeFile(filename.toStdString(), info.second);
    Data data = decoder->get();
    std::cout << "Decode finished" << std::endl;

    for (auto imager : data.imagers) {
        std::cout << sensor_info.at(imager.first).name << " lines: " << imager.second->rows() << std::endl;
    }

    ImageCompositor compositor;
    for (auto imager : data.imagers) {
        compositor.import(imager.second, sat, imager.first, data.caldata);
        QImage image(compositor.width(), compositor.height(), QImage::Format_RGBX64);

        for (size_t i = 0; i < compositor.channels(); i++) {
            std::cout << "Saving " << sensor_info.at(imager.first).name << " channel " << i+1 << std::endl;
            compositor.getChannel(image, i+1);
            image.save(QString("%1_%2_%3.png").arg(QString::fromStdString(satellite_info.at(sat).name)).arg(QString::fromStdString(sensor_info.at(imager.first).name)).arg(QString::number(i+1)));
        }
    }

    return 0;
}
