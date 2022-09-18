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

#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>

#include "map.h"
#include "qt/ui_projectdialog.h"

ProjectDialog::ProjectDialog(QWidget *parent) : QDialog(parent) {
    ui = new Ui::ProjectDialog;
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->projectionPreview->setScene(scene);

    render_finished = new QFutureWatcher<void>(this);
    QFutureWatcher<void>::connect(render_finished, &QFutureWatcher<void>::finished, [=]() { ui->render->setEnabled(true); });

    for (const std::string &crs : transform::CRS_NAMES) {
        ui->projection->addItem(QString::fromStdString(crs));
    }
}

ProjectDialog::~ProjectDialog() { delete ui; }

void ProjectDialog::write_wld_file(QString filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);

    out << pixelsize << "\n";
    out << 0.0 << "\n";
    out << 0.0 << "\n";
    out << -pixelsize << "\n";
    out << bounds.left() << "\n";
    out << bounds.bottom() << "\n";
}

QSize ProjectDialog::calculate_dimensions(size_t resolution) {
    bounds = QRectF(-180, -90, 360, 180);
    target_bounds = QRectF(0, 0, 1, 1);
    if (ui->bounds->currentText() == "Auto") {
        bounds = map::bounds(get_points(31));
        target_bounds = map::bounds_crs(get_points(31), crs);
    }

    // So big it might as well be the entire earth
    if (bounds.width() * bounds.height() > (360 * 180) * 0.95) {
        bounds = QRectF(-180, -90, 360, 180);
        target_bounds = QRectF(0, 0, 1, 1);
    }

    double scale = EARTH_CIRCUMFRANCE / ui->resolution->value();
    pixelsize = 360.0 / scale;
    QSize dimensions(bounds.width() / 360.0 * scale, bounds.height() / 360.0 * scale);
    if (crs != transform::CRS::Equdistant) {
        dimensions.rheight() = dimensions.width() * (double)target_bounds.height() / (double)target_bounds.width();
    }
    ui->details->setText(QString("Final size: %1x%2").arg(dimensions.width()).arg(dimensions.height()));

    if (resolution != 0) {
        dimensions = QSize(resolution * 2, resolution);
    }

    return dimensions;
}

QImage ProjectDialog::render(QSize dimensions) {
    QImage image = map::project(get_viewport(), get_points(31), 31, dimensions, bounds);
    image = map::reproject(image, crs, bounds, target_bounds);

    if (map_enable()) {
        std::vector<QLineF> map = map::read_shapefile(map_shapefile().toStdString());
        map::add_overlay(image, map, map_color(), crs, target_bounds);
    }
    if (landmark_enable()) {
        std::vector<Landmark> landmarks = map::read_landmarks(landmark_file().toStdString());
        map::add_landmarks(image, landmarks, landmark_color(), crs, target_bounds);
    }

    return image;
}

void ProjectDialog::on_preview_clicked() {
    QImage image = render(calculate_dimensions(1000));
    scene->clear();
    scene->setSceneRect(0, 0, image.width(), image.height());
    scene->addPixmap(QPixmap::fromImage(image, Qt::NoFormatConversion));
    ui->projectionPreview->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void ProjectDialog::on_render_clicked() {
    QSize dimensions = calculate_dimensions(0);
    if (dimensions.width() * dimensions.height() > 64000000) {
        QMessageBox confirm;
        confirm.setText(QString("Generating a large (%1x%2) image, this may cause slowdowns/crashes!")
                            .arg(dimensions.width())
                            .arg(dimensions.height()));
        confirm.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        confirm.setDefaultButton(QMessageBox::Cancel);
        confirm.setIcon(QMessageBox::Warning);
        if (confirm.exec() == QMessageBox::Cancel) {
            return;
        }
    }

    QString filename = QFileDialog::getSaveFileName(this, "Save Projected Image", QString("%1.png").arg(default_filename()),
                                                    "PNG (*.png);;JPEG (*.jpg *.jpeg);;WEBP (*.webp);;BMP (*.bmp)");
    if (filename.isEmpty()) return;

    QFuture<void> future = QtConcurrent::run([=]() {
        QImage image = render(dimensions);
        if (crs == transform::CRS::Equdistant) {
            QFileInfo fi(filename);
            write_wld_file(fi.absolutePath() + "/" + fi.completeBaseName() + ".wld");
        }
        image.save(filename);
    });

    render_finished->setFuture(future);
    ui->render->setEnabled(false);
}

void ProjectDialog::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    ui->projectionPreview->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void ProjectDialog::on_projection_textActivated(QString text) {
    for (size_t i = 0; i < transform::CRS_NAMES.size(); i++) {
        if (text.toStdString() == transform::CRS_NAMES[i]) {
            crs = (transform::CRS)i;
            return;
        }
    }
}
