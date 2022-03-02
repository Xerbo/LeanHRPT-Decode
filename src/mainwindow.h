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

#include <cmath>

#include <QMainWindow>
#include <QUrl>
#include <QMessageBox>
#include <QDesktopServices>
#include <QGraphicsView>
#include <QFutureWatcher>
#include <QShortcut>
#include <QString>
#include <QActionGroup>
#include <QColorDialog>

#include "projectdialog.h"
#include "fingerprint.h"
#include "decoders/decoder.h"
#include "imagecompositor.h"
#include "config/preset.h"
#include "config/gradient.h"
#include "satinfo.h"
#include "network.h"
#include "projection.h"

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
<p>This program uses <a href=\"https://github.com/mcmtroffaes/inipp\">inipp</a>, <a href=\"https://github.com/beltoforion/muparser\">muparser</a>, <a href=\"https://github.com/la1k/libpredict\">libpredict</a> and parts of <a href=\"https://github.com/Digitelektro/MeteorDemod\">MeteorDemod</a> which are licensed under the MIT, BSD 2-Clause \"Simplified\", GPL-2.0 and MIT license respectively.</p>"

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
    private:
        void closeEvent(QCloseEvent *event);

        Ui::MainWindow *ui;
        void setState(WindowState state);

        // Shortcuts
        QShortcut *zoomIn;
        QShortcut *zoomOut;

        std::map<Imager, ImageCompositor *> compositors;
        QImage display;
        QGraphicsScene *scene;

        // Presets
        void reloadPresets();
        PresetManager manager;
        std::map<std::string, Preset> selected_presets;

        // User settings
        float clip_limit = 1.0f;
        size_t selectedChannel = 1;
        std::array<size_t, 3> selectedComposite;
        Equalization selectedEqualization = None;

        // Satellite meta information
        SatID sat;
        int previousTabIndex = 0;

        // Orbit information
        ProjectDialog *project_diag;
        TLEManager tle_manager;
        Projector *proj;
        bool have_tles;
        QColorDialog *color_dialog;
        QColor map_color = QColor(255,255,0);
        QString map_shapefile;

        // Sensor selection
        Imager sensor;
        Imager default_sensor;
        QActionGroup *sensor_select;
        std::map<std::string, QAction *> sensor_actions;

        // Decoding
        QFutureWatcher<void> *decodeWatcher;
        Decoder *decoder = nullptr;
        Fingerprint *fingerprinter = nullptr;
        QLabel *status;
        QPushButton *cancel_button;
        bool clean_up = false;

        // Gradients
        GradientManager *gradient_manager;

        // Internal
        void incrementZoom(int amount);
        void startDecode(std::string filename);
        void decodeFinished();
        void updateDisplay();
        void populateChannelSelectors(size_t channels);

        // Sets the contents of a QGraphicsScene to a QImage
        static void displayQImage(QGraphicsScene *scene, QImage &image) {
            scene->clear();
            scene->setSceneRect(0, 0, image.width(), image.height());
            scene->addPixmap(QPixmap::fromImage(image, Qt::NoFormatConversion));
        };

        // Channel
        void setChannel(int sensor_channel);

        // Composite
        void setCompositeChannel(int channel, int sensor_channel);
        void setComposite(std::array<size_t, 3> channels);

        // Equalization
        void setEqualization(Equalization type);

        // Image saving
        bool savingImage = false;
        void saveAllChannels();
        void saveCurrentImage(bool corrected);

        // GCP Saving
        void save_gcp();
        std::map<Imager, std::vector<double>> timestamps;
        double pass_timestamp;
    protected:
        void dragEnterEvent(QDragEnterEvent *e);
        void dropEvent(QDropEvent *e);
    private slots:
        // menuFile
        void on_actionOpen_triggered();
        void on_actionSave_Current_Image_triggered()           { saveCurrentImage(false); };
        void on_actionSave_Current_Image_Corrected_triggered() { saveCurrentImage(true); };
        void on_actionSave_All_Channels_triggered()            { saveAllChannels(); };
        void on_actionSave_GCP_File_triggered()                { save_gcp(); };
        // menuGeo
        void on_actionProjector_triggered() { project_diag->show(); };
        void on_actionMap_Shapefile_triggered();
        void on_actionMap_Color_triggered() { color_dialog->show(); };
        void on_actionEnable_Map_triggered();
        // menuOptions
        void on_actionFlip_triggered();
        void on_actionIR_Blend_triggered();
        // menuHelp
        void on_actionDocumentation_triggered()  { QDesktopServices::openUrl(QUrl("https://github.com/Xerbo/LeanHRPT-Decode/wiki")); };
        void on_actionIssue_Tracker_triggered()  { QDesktopServices::openUrl(QUrl("https://github.com/Xerbo/LeanHRPT-Decode/issues")); };
        void on_actionAbout_LeanHRPT_triggered() { QMessageBox::about(this, "About LeanHRPT", QString("%1\nVersion: %2").arg(ABOUT_TEXT).arg(VERSION)); };
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

        void on_zoomSelector_activated(int index);
        void on_imageTabs_currentChanged(int index);
        void on_presetSelector_activated(QString text);
        void on_presetReload_clicked() { manager.reload(); reloadPresets(); };
        void on_gradient_textActivated(const QString &text);

        // https://www.desmos.com/calculator/ercsdr9hrq
        void on_contrastLimit_valueChanged(int value) {
            clip_limit = std::log10(value/100.0f*0.9 + 0.1)+1.0f;
            setEqualization(selectedEqualization);
        };
        void on_brightnessOnly_stateChanged() {
            setEqualization(selectedEqualization);
        }
};

#endif // MAINWINDOW_H
