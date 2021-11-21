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

#ifndef MAPSETTINGS_H
#define MAPSETTINGS_H

#include "satinfo.h"

#include <QDialog>
#include <QColorDialog>
#include <QProcess>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MapSettings; }
QT_END_NAMESPACE

class MapSettings : public QDialog {
    Q_OBJECT
    public:
        MapSettings(QWidget *parent = nullptr);
        ~MapSettings();

        void start(size_t width, size_t height);
        QColor color = QColor(255,255,255);
    private:
        Ui::MapSettings *ui;

        QString shapefileFilename;
        QString gcpFilename;
        QTimer *timer;
        QString history;
        QProcess *process;
        bool warping;
        size_t _width;
        size_t _height;

        QColorDialog *color_dialog;

        void set_enabled(bool enabled);
        void createGcps();
    signals:
        void prepareGcps();
        void finished();
    private slots:
        void on_startButton_clicked();
        void on_gcp_clicked();
        void on_shapefile_clicked();
        void on_color_clicked() {
            color_dialog->setCurrentColor(color);
            color_dialog->open();
        }
};

#endif
