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

#include "mapsettings.h"
#include "qt/ui_mapsettings.h"

#include "projectdialog.h"

#include <fstream>
#include <QFileDialog>
#include <iostream>
#include <sstream>

MapSettings::MapSettings(QWidget *parent) : QDialog(parent) {
    ui = new Ui::MapSettings;
    ui->setupUi(this);

    timer = new QTimer(this);
    QTimer::connect(timer, &QTimer::timeout, this, [this]() {
        QByteArray data = process->read(128);
        if (data.size() != 0) {
            history.append(QString(data));
            ui->logWindow->setPlainText(history);
        }

        if (process->state() == QProcess::NotRunning) {
            if (warping) {
#ifdef _WIN32
                QString program = "C:\\OSGeo4W\\bin\\gdal_rasterize.exe";
#else
                QString program = "gdal_rasterize";
#endif
                QStringList arguments;
                arguments << "-ot" << "Byte" << "-burn" << "255" << "-ts" << QString::number(_width) << QString::number(_height) << "-te" << "0" << QString::number(_height) << QString::number(_width) << "0";
                arguments << (QString::fromStdString(get_temp_dir() + "/map.shp")) << (QString::fromStdString(get_temp_dir() + "/map.tif"));
                history += "Command: " + program + " " + arguments.join(" ") + "\n";
                process->start(program, arguments, QIODevice::ReadOnly);
                warping = false;
            } else {
                timer->stop();
                set_enabled(true);
                finished();
            }
            return;
        }
    });

    color_dialog = new QColorDialog(this);
    QColorDialog::connect(color_dialog, &QColorDialog::colorSelected, this, [this](QColor _color) {
        color = _color;
        ui->color->setStyleSheet("background-color: rgb(" + QString::number(color.red()) + ", " + QString::number(color.green()) + ", " + QString::number(color.blue()) + ");");
    });
}

MapSettings::~MapSettings() {
    delete ui;
}

void MapSettings::set_enabled(bool enabled) {
    ui->gcp->setEnabled(enabled);
    ui->shapefile->setEnabled(enabled);
    ui->startButton->setEnabled(enabled);
}

void MapSettings::on_shapefile_clicked() {
    QString _shapefileFilename = QFileDialog::getOpenFileName(this, "Select Shapefile", "", "Shapefile (*.shp)");
    if (!_shapefileFilename.isEmpty()) {
        shapefileFilename = _shapefileFilename;

        ui->shapefile->setText(shapefileFilename);
        ui->startButton->setEnabled(true);
    }
}

void MapSettings::on_gcp_clicked() {
    gcpFilename = QFileDialog::getOpenFileName(this, "Select GCP File", "", "GCP (*.gcp)");
    if (gcpFilename.isEmpty()) {
        ui->gcp->setText("(From TLE)");
    } else {
        ui->gcp->setText(gcpFilename);
    }
}

void MapSettings::on_startButton_clicked() {
    prepareGcps();
}

void MapSettings::start(size_t width, size_t height) {
#ifdef _WIN32
    QString program = "C:\\OSGeo4W\\bin\\ogr2ogr.exe";
#else
    QString program = "ogr2ogr";
#endif
    QStringList arguments;
    arguments << "-tps" << (QString::fromStdString(get_temp_dir()) + "/map.shp") << shapefileFilename;
    history = "Command: " + program + " " + arguments.join(" ") + "\n";
    ui->logWindow->setPlainText(history);
    createGcps(arguments);

    _width = width;
    _height = height;

    warping = true;
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(program, arguments, QIODevice::ReadOnly);

    set_enabled(false);
    timer->start(1000.0f/30.0f);
}

std::string get_tag(std::string line, std::string tag) {
    size_t start = line.find(tag + "=") + tag.length() + 2;
    line = line.substr(start);
    size_t end = line.find("\"");
    line = line.substr(0, end);
    return line;
}

static double str2double(std::string str) {
    QLocale l(QLocale::C);
    return l.toDouble(QString::fromStdString(str));
}

void MapSettings::createGcps(QStringList &list) {
    std::filebuf in;
    if (gcpFilename.isEmpty()) {
        in.open(get_temp_dir() + "/image.gcp", std::ios::in);
    } else {
        in.open(get_temp_dir() + gcpFilename.toStdString(), std::ios::in);
    }
    std::istream stream(&in);

    //std::ostringstream out;

    for (std::string str; std::getline(stream, str); ) {
        if (str.substr(0, 5) == "<GCP ") {
            double line = str2double(get_tag(str, "Line"));
            double pixel = str2double(get_tag(str, "Pixel"));
            double x = str2double(get_tag(str, "X"));
            double y = str2double(get_tag(str, "Y"));

            list << "-gcp";
            list << QString::number(x);
            list << QString::number(y);
            list << QString::number(pixel);
            list << QString::number(line);
        }
    }

    //return out.str();
}
