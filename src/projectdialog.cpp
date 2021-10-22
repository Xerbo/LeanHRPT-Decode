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
#include <iostream>
#include <fcntl.h>

ProjectDialog::ProjectDialog(QWidget *parent) : QDialog(parent) {
    ui = new Ui::ProjectDialog;
    ui->setupUi(this);

    timer = new QTimer(this);
    QTimer::connect(timer, &QTimer::timeout, this, [this]() {
        if (feof(process)) {
            set_enabled(true);
            timer->stop();
            history.clear();
            pclose(process);
        } else {
            // Pretty sure I just reimplemented fread here
            char byte;
            bool changed = false;
            while ((byte = fgetc(process)) != EOF) {
                history.append(byte);
                changed = true;
            }

            if (changed) {
                ui->logWindow->setPlainText(history);
            }
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
    prepareImage();
}

void ProjectDialog::start() {
    std::string epsg = ui->projection->currentText().split(" ")[0].toStdString();

    // https://gdal.org/programs/gdalwarp.html#cmdoption-gdalwarp-r
    std::string interpolation;
    if (ui->interpolation->currentText() == "Lanczos") {
        interpolation = "lanczos";
    } else if (ui->interpolation->currentText() == "Cubic") {
        interpolation = "cubic";
    } else if (ui->interpolation->currentText() == "Bilinear") {
        interpolation = "bilinear";
    } else if (ui->interpolation->currentText() == "Nearest Neighbor") {
        interpolation = "near";
    }

    std::string command = "gdal_translate $(cat /tmp/gcp.gcp) /tmp/input.png /tmp/image.tif 2>&1 &&";
    command.append("gdalwarp -overwrite -r " + interpolation + " -tps -t_srs " + epsg + " /tmp/image.tif " + outputFilename.toStdString() + " 2>&1");

    process = popen(command.c_str(), "r");
    int flags = fcntl(process->_fileno, F_GETFL, 0);
    fcntl(process->_fileno, F_SETFL, flags | O_NONBLOCK);

    set_enabled(false);
    timer->start(1000.0f/30.0f);
}