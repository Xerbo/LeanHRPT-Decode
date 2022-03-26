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
#include "map.h"

#include <QFileDialog>
#include <fstream>

ProjectDialog::ProjectDialog(QWidget *parent) : QDialog(parent) {
    ui = new Ui::ProjectDialog;
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->projectionPreview->setScene(scene);
}

ProjectDialog::~ProjectDialog() {
    delete ui;
}

QImage ProjectDialog::render(size_t resolution) {
    QImage image = map::project(get_viewport(), get_points(31), 31, resolution);

    if (map_enable()) {
        std::vector<QLineF> map = map::read_shapefile(map_shapefile().toStdString());
        map::add_overlay(image, map, map_color());
    }

    if(ui->targetProjection->currentText() == "North Pole Azimuthal Equidistant") {
        image = map::warp_to_azimuthal_equidistant(image, false);
    }
    if(ui->targetProjection->currentText() == "South Pole Azimuthal Equidistant") {
        image = map::warp_to_azimuthal_equidistant(image, true);
    }

    return image;
}

void ProjectDialog::on_preview_clicked() {
    QImage image = render(1000);
    scene->clear();
    scene->setSceneRect(0, 0, image.width(), image.height());
    scene->addPixmap(QPixmap::fromImage(image, Qt::NoFormatConversion));
    ui->projectionPreview->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void ProjectDialog::on_render_clicked() {
    QImage image = render(10000);
    image.save("map.png");
}

void ProjectDialog::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    ui->projectionPreview->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}
