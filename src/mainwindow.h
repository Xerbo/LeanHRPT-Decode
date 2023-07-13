/*
 * LeanHRPT Decode
 * Copyright (C) 2021-2022 Xerbo
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

#ifndef LEANHRPT_MAINWINDOW_H_
#define LEANHRPT_MAINWINDOW_H_

#include <QActionGroup>
#include <QColorDialog>
#include <QDesktopServices>
#include <QFutureWatcher>
#include <QGraphicsView>
#include <QMainWindow>
#include <QMessageBox>
#include <QShortcut>
#include <QString>
#include <QUrl>
#include <array>

#include "config/gradient.h"
#include "config/preset.h"
#include "decoders/decoder.h"
#include "fingerprint.h"
#include "image/compositor.h"
#include "network.h"
#include "projectdialog.h"
#include "projection.h"
#include "satinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum class WindowState { Idle, Decoding, Finished };

#define ABOUT_TEXT \
    "<h2>LeanHRPT Decode</h2>\
A high quality, easy to use HRPT decoder\
<ul>\
<li><code>Ctrl++</code> Zoom in</li>\
<li><code>Ctrl+-</code> Zoom out</li>\
<li><code>Ctrl+F</code> Flip image</li>\
</ul>\
Licensed under GPL-3.0.\
<p>This program uses:</p>\
<ul>\
<li><a href=\"https://github.com/mcmtroffaes/inipp\">inipp</a> - Licensed under MIT</li>\
<li><a href=\"https://github.com/beltoforion/muparser\">muparser</a> - Licensed under BSD 2-Clause \"Simplified\"</li>\
<li><a href=\"https://github.com/la1k/libpredict\">libpredict</a> - Licensed under GPL-2.0</li>\
<li><a href=\"https://github.com/OSGeo/shapelib\">shapelib</a> - Licensed under GPL-2.0</li>\
<li>Parts of <a href=\"https://github.com/Digitelektro/MeteorDemod\">MeteorDemod</a> - Licensed under MIT</li>\
<li>Parts of <a href=\"https://github.com/airbreather/Gavaghan.Geodesy\">Gavaghan.Geodesy</a> - Public domain</li>\
</ul>"

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
    bool corrected = false;
    float clip_limit = 1.0f;
    float set_clip_limit = clip_limit;
    size_t selectedChannel = 1;
    std::array<size_t, 3> selectedComposite;
    Equalization selectedEqualization = Equalization::None;
    Suggestion fingerprinterSuggestion = Suggestion::Automatic;

    // Satellite meta information
    SatID sat;
    int previousTabIndex = 0;

    // Orbit information
    ProjectDialog *project_diag;
    TLEManager tle_manager;
    Projector *proj;
    bool have_tles;
    QColorDialog *color_dialog;
    QColorDialog *landmark_color_dialog;
    QColor map_color = QColor(255, 255, 0);
    QColor landmark_color = QColor(255, 0, 0);
    QString map_shapefile;
    QString landmark_file;

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
    void get_source(QImage &image);
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
    void colorClipLimit(bool changed);

    // Image saving
    bool savingImage = false;
    void saveAllChannels();
    QString getDefaultFilename();
    void saveCurrentImage();

    // GCP Saving
    void save_gcp();
    std::map<Imager, std::vector<double>> timestamps;
    double pass_timestamp;

    // TIP saving
    void save_tip();

   protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
   private slots:
    // menuFile
    void on_actionOpen_triggered();
    void on_actionSave_Current_Image_triggered() { saveCurrentImage(); };
    void on_actionSave_All_Channels_triggered() { saveAllChannels(); };
    void on_actionSave_GCP_File_triggered() { save_gcp(); };
    void on_actionSave_TIP_triggered() { save_tip(); };
    // menuGeo
    void on_actionProjector_triggered() { project_diag->show(); };
    void on_actionMap_Shapefile_triggered();
    void on_actionLandmark_File_triggered();
    void on_actionMap_Color_triggered() { color_dialog->show(); };
    void on_actionLandmark_Color_triggered() { landmark_color_dialog->show(); };
    void on_actionEnable_Map_triggered();
    void on_actionEnable_Landmarks_triggered();
    // menuOptions
    void on_actionFlip_triggered();
    void on_actionCorrect_triggered();
    void on_actionIR_Blend_triggered();
    void on_groupProtocol_triggered();
    // menuHelp
    void on_actionDocumentation_triggered() { QDesktopServices::openUrl(QUrl("https://github.com/Xerbo/LeanHRPT-Decode/wiki")); };
    void on_actionIssue_Tracker_triggered() {
        QDesktopServices::openUrl(QUrl("https://github.com/Xerbo/LeanHRPT-Decode/issues"));
    };
    void on_actionAbout_LeanHRPT_triggered() {
        QMessageBox::about(this, "About LeanHRPT", QString("%1\nVersion: %2").arg(ABOUT_TEXT).arg(VERSION));
    };
    void on_actionAbout_Qt_triggered() { QMessageBox::aboutQt(this, "About Qt"); };

    // Channel selectors
    void on_channelSelector_activated(int index) { setChannel(index); };
    void on_redSelector_activated(int index) { setCompositeChannel(0, index); };
    void on_greenSelector_activated(int index) { setCompositeChannel(1, index); };
    void on_blueSelector_activated(int index) { setCompositeChannel(2, index); };

    // Equalization
    void on_equalisationNone_clicked() { setEqualization(Equalization::None); };
    void on_equalisationStretch_clicked() { setEqualization(Equalization::Stretch); };
    void on_equalisationHistogram_clicked() { setEqualization(Equalization::Histogram); };

    void on_zoomSelector_activated(int index);
    void on_imageTabs_currentChanged(int index);
    void on_presetSelector_textActivated(QString text);
    void on_presetReload_clicked() {
        manager.reload();
        reloadPresets();
    };
    void on_gradient_textActivated(const QString &text);

    void on_contrastLimit_valueChanged(int value) {
        clip_limit = (value/100.0f) * (value/100.0f);
        if (clip_limit != set_clip_limit){
            colorClipLimit(true);
        } else {
            colorClipLimit(false);
        }
    };
    void on_contrastLimitApply_clicked(){
        set_clip_limit = clip_limit;
        setEqualization(selectedEqualization);
        if (clip_limit != set_clip_limit){
            colorClipLimit(true);
        } else {
            colorClipLimit(false);
        }
    }
    void on_brightnessOnly_stateChanged() { setEqualization(selectedEqualization); }
};

#endif  // MAINWINDOW_H
