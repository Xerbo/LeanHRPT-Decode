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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QFutureWatcher>
#include <QShortcut>

#include "imagecompositor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum Satellite {
    Satellite_Unknown,
    Satellite_NOAA,
    Satellite_Meteor,
    Satellite_Fengyun,
    Satellite_MetOp
};

class MainWindow : public QMainWindow {
    Q_OBJECT
    private:
        Ui::MainWindow *ui;

        // Shit related to images
        ImageCompositor *compositor;
        QFutureWatcher<void> *decodeWatcher;
        QImage channel, composite, ndvi;
        int selectedChannel = 1, selectedComposite[3] = { 2, 2, 1 };
        Equalization selectedEqualization = None;
        QString imagerName;
        QGraphicsScene *graphicsScene;

        QShortcut *flip;
        QShortcut *zoomIn;
        QShortcut *zoomOut;
        void incrementZoom(int amount);

        int previousTabIndex = 0;
        void setEnabled(bool enabled);

        void startDecode(Satellite satellite, std::string filename);
        void decodeFinished();
        void reloadImage();
        void populateChannelSelectors(int channels);
        void displayQImage(QImage *image);
        void updateEqualization(Equalization type);
        void saveAllChannels(QString dirname);
        void saveCurrentImage(QString filename);

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private slots:
        // menuFile
        void on_actionOpen_triggered();
        void on_actionSave_Current_Image_triggered();
        void on_actionSave_All_Channels_triggered();
        // menuHelp
        void on_actionDocumentation_triggered();
        void on_actionIssue_Tracker_triggered();
        void on_actionAbout_LeanHRPT_triggered();
        void on_actionAbout_Qt_triggered();
        // menuOptions
        void on_actionFlip_triggered();
        // Channel selectors
        void on_channelSelector_activated(int index);
        void on_redSelector_activated(int index);
        void on_greenSelector_activated(int index);
        void on_blueSelector_activated(int index);
        // Equalisation
        void on_equalisationNone_clicked();
        void on_equalisationStretch_clicked();
        void on_equalisationHistogram_clicked();
        // Zoom selector
        void on_zoomSelector_activated(int index);

        void on_imageTabs_currentChanged(int index);
};

#endif // MAINWINDOW_H
