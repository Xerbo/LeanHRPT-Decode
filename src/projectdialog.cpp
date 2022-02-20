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

#include "projectdialog.h"
#include "qt/ui_projectdialog.h"

#include <QFileDialog>
#include <fstream>

std::string get_temp_dir() {
#ifdef _WIN32
    return std::getenv("TEMP");
#else
    return "/tmp";
#endif
}

ProjectDialog::ProjectDialog(QWidget *parent) : QDialog(parent) {
    ui = new Ui::ProjectDialog;
    ui->setupUi(this);

    timer = new QTimer(this);
    QTimer::connect(timer, &QTimer::timeout, this, [this]() {
        QByteArray data = process->read(128);
        if (data.size() != 0) {
            history.append(QString(data));
            ui->logWindow->setPlainText(history);
        }

        if (process->state() == QProcess::NotRunning) {
            timer->stop();
            set_enabled(true);
            return;
        }
    });
}

ProjectDialog::~ProjectDialog() {
    delete ui;
}

void ProjectDialog::set_enabled(bool enabled) {
    ui->projection->setEnabled(enabled);
    ui->interpolation->setEnabled(enabled);
    ui->output->setEnabled(enabled);
    ui->startButton->setEnabled(enabled);
}

void ProjectDialog::on_output_clicked() {
    QString _outputFilename = QFileDialog::getSaveFileName(this, "Select Output File", "", "GeoTIFF (*.tif *.tiff)");
    if (!_outputFilename.isEmpty()) {
        outputFilename = _outputFilename;

        ui->output->setText(outputFilename);
        ui->startButton->setEnabled(true);
    }
}

void ProjectDialog::on_gcp_clicked() {
    gcpFilename = QFileDialog::getOpenFileName(this, "Select Output File", "", "GeoTIFF (*.tif *.tiff)");
    if (gcpFilename.isEmpty()) {
        ui->gcp->setText("(From TLE)");
    } else {
        ui->gcp->setText(gcpFilename);
    }
}

void ProjectDialog::on_startButton_clicked() {
    prepareImage(gcpFilename.isEmpty());
}

void ProjectDialog::createVrt() {
    std::filebuf in;
    if (gcpFilename.isEmpty()) {
        in.open(get_temp_dir() + "/image.gcp", std::ios::in);
    } else {
        in.open(gcpFilename.toStdString(), std::ios::in);
    }
    std::filebuf out;
    out.open(get_temp_dir() + "/image.vrt", std::ios::out);
    std::ostream dst(&out);

    QImage image(QString::fromStdString(get_temp_dir()) + "/viewport.png");
    dst << "<VRTDataset rasterXSize=\"" << image.width() << "\" rasterYSize=\"" << image.height() << "\">\n";
    dst << &in;

    size_t nchannels = (image.format() == QImage::Format_Grayscale16) ? 1 : 3;

    for (size_t i = 0; i < nchannels; i++) {
        dst << "<VRTRasterBand dataType=\"UInt16\" band=\"" << (i+1) << "\">\n";
        dst << "  <NoDataValue>0.0</NoDataValue>\n";
        if (nchannels == 1) {
            dst << "  <ColorInterp>Gray</ColorInterp>\n";
        } else {
            std::string channel_names[3] = { "Red", "Green", "Blue" };
            dst << "  <ColorInterp>" << channel_names[i] << "</ColorInterp>\n";
        }
        dst << "  <SimpleSource>\n";
        dst << "    <SourceFilename relativeToVRT=\"1\">" + get_temp_dir() + "/viewport.png</SourceFilename>\n";
        if (nchannels == 3) {
            dst << "    <SourceBand>" << (i+1) << "</SourceBand>\n";
        }
        dst << "  </SimpleSource>\n";
        dst << "</VRTRasterBand>\n";
    }
    dst << "</VRTDataset>\n";

    in.close();
    out.close();
}

void ProjectDialog::start() {
    QString epsg = ui->projection->currentText().split(" ")[0];

    // https://gdal.org/programs/gdalwarp.html#cmdoption-gdalwarp-r
    QString interpolation;
    if (ui->interpolation->currentText() == "Lanczos") {
        interpolation = "lanczos";
    } else if (ui->interpolation->currentText() == "Cubic") {
        interpolation = "cubic";
    } else if (ui->interpolation->currentText() == "Bilinear") {
        interpolation = "bilinear";
    } else if (ui->interpolation->currentText() == "Nearest Neighbor") {
        interpolation = "near";
    }

    createVrt();

#ifdef _WIN32
    QString program = "C:\\OSGeo4W\\bin\\gdalwarp.exe";
#else
    QString program = "gdalwarp";
#endif
    QStringList arguments;
    arguments << "-multi" << "-wm" << "512" << "-wo" << "NUM_THREADS=ALL_CPUS" << "-overwrite" << "-r" << interpolation << "-tps" << "-t_srs" << epsg << (QString::fromStdString(get_temp_dir()) + "/image.vrt") << outputFilename;

    history = "Command: " + program + " " + arguments.join(" ") + "\n";

    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(program, arguments, QIODevice::ReadOnly);

    set_enabled(false);
    timer->start(1000.0f/30.0f);
}
