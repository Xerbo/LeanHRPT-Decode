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
#include "ui_projectdialog.h"

#include <QFileDialog>
#include <fstream>

ProjectDialog::ProjectDialog(QWidget *parent) : QDialog(parent) {
    ui = new Ui::ProjectDialog;
    ui->setupUi(this);

    timer = new QTimer(this);
    QTimer::connect(timer, &QTimer::timeout, this, [this]() {
        if (process->state() == QProcess::NotRunning) {
            timer->stop();
            set_enabled(true);
            return;
        }

        QByteArray data = process->read(128);
        if (data.size() != 0) {
            history.append(QString(data));
            ui->logWindow->setPlainText(history);
        }
    });
}

ProjectDialog::~ProjectDialog() {
    delete ui;
}

void ProjectDialog::set_enabled(bool enabled) {
    ui->source->setEnabled(enabled);
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

void ProjectDialog::on_startButton_clicked() {
    prepareImage(ui->source->currentText() == "Viewport");
}

void ProjectDialog::createVrt(Imager sensor) {
    std::ifstream src("/tmp/image.gcp");
    std::ofstream dst("/tmp/image.vrt");

    QImage image((ui->source->currentText() == "Viewport") ? "/tmp/viewport.png" : "/tmp/channel-1.png");
    dst << "<VRTDataset rasterXSize=\"" << image.width() << "\" rasterYSize=\"" << image.height() << "\">\n";
    dst << src.rdbuf();

    if (ui->source->currentText() == "Viewport") {
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
            dst << "    <SourceFilename relativeToVRT=\"1\">/tmp/viewport.png</SourceFilename>\n";
            if (nchannels == 3) {
                dst << "    <SourceBand>" << (i+1) << "</SourceBand>\n";
            }
            dst << "  </SimpleSource>\n";
            dst << "</VRTRasterBand>\n";
        }
    } else {
        std::vector<ChannelInfo> ch = channels.at(sensor);
        for (size_t i = 0; i < ch.size(); i++) {
            FormatInfo chinfo = format_info.at(ch[i].format);
            dst << "<VRTRasterBand dataType=\"UInt16\" band=\"" << (i+1) << "\">\n";
            dst << "  <Description>" << ch[i].wavelength << " µm</Description>\n";
            dst << "  <NoDataValue>0.0</NoDataValue>\n";
            dst << "  <Scale>" << chinfo.scale << "</Scale>\n";
            dst << "  <Offset>" << chinfo.offset << "</Offset>\n";
            dst << "  <UnitType>" << chinfo.unit << "</UnitType>\n";
            dst << "  <SimpleSource>\n";
            dst << "    <SourceFilename relativeToVRT=\"1\">/tmp/channel-" << (i+1) << ".png</SourceFilename>\n";
            dst << "  </SimpleSource>\n";
            dst << "</VRTRasterBand>\n";
        }
    }
    dst << "</VRTDataset>\n";

    dst.close();
    src.close();
}

void ProjectDialog::start(Imager sensor) {
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

    createVrt(sensor);

    QString program = "gdalwarp";
    QStringList arguments;
    arguments << "-overwrite" << "-r" << interpolation << "-tps" << "-t_srs" << epsg << "/tmp/image.vrt" << outputFilename;

    history = "Command: " + program + " " + arguments.join(" ") + "\n";

    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(program, arguments, QIODevice::ReadOnly);

    set_enabled(false);
    timer->start(1000.0f/30.0f);
}
