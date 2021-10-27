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

#include "satinfo.h"

#include <QDialog>
#include <QProcess>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class ProjectDialog; }
QT_END_NAMESPACE

static std::string get_temp_dir() {
#ifdef _WIN32
    return std::getenv("TEMP");
#else
    return "/tmp";
#endif
}

class ProjectDialog : public QDialog {
    Q_OBJECT
    public:
        ProjectDialog(QWidget *parent = nullptr);
        ~ProjectDialog();

        void start(Imager sensor);
    private:
        Ui::ProjectDialog *ui;

        QString outputFilename;
        QString gcpFilename;
        QTimer *timer;
        QString history;
        QProcess *process;

        void set_enabled(bool enabled);
        void createVrt(Imager sensor);
    private slots:
        void on_startButton_clicked();
        void on_gcp_clicked();
        void on_output_clicked();
    signals:
        void prepareImage(bool viewport, bool createGcp);
};

#endif
