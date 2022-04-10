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
#include <QtConcurrent/QtConcurrent>
#include <QFileDialog>

ProjectDialog::ProjectDialog(QWidget *parent) : QDialog(parent) {
    ui = new Ui::ProjectDialog;
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->projectionPreview->setScene(scene);

    render_finished = new QFutureWatcher<void>(this);
    QFutureWatcher<void>::connect(render_finished, &QFutureWatcher<void>::finished, [=]() {
        ui->render->setEnabled(true);
    });
}

ProjectDialog::~ProjectDialog() {
    delete ui;
}

QImage ProjectDialog::render(size_t resolution) {
    QRectF bounds(-180, -90, 360, 180);
    if (ui->bounds->currentText() == "Auto") {
        bounds = map::bounds(get_points(31));
    }

    double scale = EARTH_CIRCUMFRANCE/ui->resolution->value();
    QSize dimensions(bounds.width()/360.0 * scale, bounds.height()/360.0 * scale);
    ui->details->setText(QString("Final size: %1x%2").arg(dimensions.width()).arg(dimensions.height()));

    if (resolution != 0) {
        dimensions = QSize(resolution*2, resolution);
    }

    QImage image = map::project(get_viewport(), get_points(31), 31, dimensions, bounds.width(), bounds.x(), bounds.height(), bounds.y());

    if (map_enable()) {
        std::vector<QLineF> map = map::read_shapefile(map_shapefile().toStdString());
        map::add_overlay(image, map, map_color(), bounds.width(), bounds.x(), bounds.height(), bounds.y());
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
    QString filename = QFileDialog::getSaveFileName(this, "Save Projected Image", QString("%1_%2.png").arg(default_filename()).arg(ui->targetProjection->currentText()), "PNG (*.png);;JPEG (*.jpg *.jpeg);;WEBP (*.webp);;BMP (*.bmp)");
    if (filename.isEmpty()) return;

    QFuture<void> future = QtConcurrent::run([=]() {
        QImage image = render(0);
        image.save(filename);
    });

    render_finished->setFuture(future);
    ui->render->setEnabled(false);
}

void ProjectDialog::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    ui->projectionPreview->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}
