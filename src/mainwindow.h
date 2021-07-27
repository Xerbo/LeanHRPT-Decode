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
#include <QUrl>
#include <QMessageBox>
#include <QDesktopServices>

#include <QGraphicsView>
#include <QFutureWatcher>
#include <QShortcut>
#include <QString>

#include "decoders/decoder.h"
#include "imagecompositor.h"
#include "preset.h"
#include "satinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum WindowState {
    Idle,
    Decoding,
    Finished
};

#define ABOUT_TEXT "<h2>LeanHRPT Decode</h2>\
A high quality, easy to use HRPT decoder\
<ul>\
<li><code>Ctrl++</code> Zoom in</li>\
<li><code>Ctrl+-</code> Zoom out</li>\
<li><code>Ctrl+F</code> Flip image</li>\
</ul>\
Licensed under GPL-3.0.\
<p>This program uses <a href=\"https://github.com/mcmtroffaes/inipp\">inipp</a> and <a href=\"https://github.com/beltoforion/muparser\">muParser</a> which are licensed under the MIT and BSD 2-Clause \"Simplified\" license respectively.</p>"

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
    private:
        Ui::MainWindow *ui;
        void setState(WindowState state);

        // Shortcuts
        QShortcut *flip;
        QShortcut *zoomIn;
        QShortcut *zoomOut;

        QImage channel;
        QImage composite;
        QImage preset;

        std::map<std::string, Preset> selected_presets;

        // unassorted shit
        PresetManager manager;
        Satellite sat;
        ImageCompositor *compositor;
        QFutureWatcher<void> *decodeWatcher;
        int selectedChannel = 1;
        int selectedComposite[3] = { 2, 2, 1 };
        Equalization selectedEqualization = None;
        QString imagerName;
        int previousTabIndex = 0;
        QGraphicsScene *graphicsScene;
        Decoder *decoder = nullptr;
        QLabel *status;

        void incrementZoom(int amount);
        void startDecode(Satellite satellite, std::string filename);
        void decodeFinished();
        void reloadImage();
        void populateChannelSelectors(size_t channels);
        void displayQImage(QImage *image);

        // Settings
        void setEqualization(Equalization type);
        void setChannel(int sensor_channel);
        void setCompositeChannel(int channel, int sensor_channel);

        // Image saving
        void saveAllChannels(QString directory);
        void writeCurrentImage(QString filename, bool corrected);
        void saveCurrentImage(bool corrected);
        //void saveCurrentImage(QString filename);
    private slots:
        // menuFile
        void on_actionOpen_triggered();
        void on_actionSave_Current_Image_triggered()           { saveCurrentImage(false); };
        void on_actionSave_Current_Image_Corrected_triggered() { saveCurrentImage(true); };
        void on_actionSave_All_Channels_triggered();

        // menuOptions
        void on_actionFlip_triggered();

        // Other shit
        void on_zoomSelector_activated(int index);
        void on_imageTabs_currentChanged(int index);

        // menuHelp
        void on_actionDocumentation_triggered()  { QDesktopServices::openUrl(QUrl("https://github.com/Xerbo/LeanHRPT-Decode/wiki")); };
        void on_actionIssue_Tracker_triggered()  { QDesktopServices::openUrl(QUrl("https://github.com/Xerbo/LeanHRPT-Decode/issues")); };
        void on_actionAbout_LeanHRPT_triggered() { QMessageBox::about(this, "About LeanHRPT", ABOUT_TEXT); };
        void on_actionAbout_Qt_triggered()       { QMessageBox::aboutQt(this, "About Qt"); };

        // Channel selectors
        void on_channelSelector_activated(int index) { setChannel(index); };
        void on_redSelector_activated(int index)     { setCompositeChannel(0, index); };
        void on_greenSelector_activated(int index)   { setCompositeChannel(1, index); };
        void on_blueSelector_activated(int index)    { setCompositeChannel(2, index); };

        // Equalization
        void on_equalisationNone_clicked()      { setEqualization(Equalization::None); };
        void on_equalisationStretch_clicked()   { setEqualization(Equalization::Stretch); };
        void on_equalisationHistogram_clicked() { setEqualization(Equalization::Histogram); };

        void on_presetSelector_activated(QString text);
        void on_presetReload_clicked() { manager.reload(); };
        void on_contrastLimit_valueChanged(int value) { compositor->setClipLimit(value/100.0f); setEqualization(selectedEqualization); };
};

#endif // MAINWINDOW_H
