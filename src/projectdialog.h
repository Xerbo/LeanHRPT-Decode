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

#ifndef PROJECTDIALOG_H
#define PROJECTDIALOG_H

#include <QColor>
#include <QDialog>
#include <QFutureWatcher>
#include <QGraphicsScene>
#include <QProcess>
#include <QTimer>

#include "geo/crs.h"
#include "projection.h"
#include "satinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ProjectDialog;
}
QT_END_NAMESPACE

class ProjectDialog : public QDialog {
    Q_OBJECT
   public:
    ProjectDialog(QWidget *parent = nullptr);
    ~ProjectDialog();

   private:
    Ui::ProjectDialog *ui;
    QGraphicsScene *scene;
    QFutureWatcher<void> *render_finished;
    QImage render(QSize dimensions);
    QSize calculate_dimensions(size_t resolution);
    void write_wld_file(QString filename);

    transform::CRS crs = transform::CRS::Equdistant;
    QRectF bounds;
    QRectF target_bounds;
    double pixelsize;

    virtual void resizeEvent(QResizeEvent *event) override;
   private slots:
    void on_preview_clicked();
    void on_render_clicked();
    void on_projection_textActivated(QString text);
   signals:
    QImage get_viewport();
    std::vector<std::pair<xy, Geodetic>> get_points(size_t n);

    QString map_shapefile();
    QColor map_color();
    bool map_enable();
    QString default_filename();
};

#endif
