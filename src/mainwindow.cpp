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

#include "mainwindow.h"

#include <QCloseEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QtConcurrent/QtConcurrent>

#include "decoders/decoder.h"
#include "map.h"
#include "projectdialog.h"
#include "protocol/timestamp.h"
#include "qt/ui_mainwindow.h"
#include "util.h"
#include "geometry.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    ui = new Ui::MainWindow;
    ui->setupUi(this);

    // Image display
    scene = new QGraphicsScene(this);
    ui->channelView->setScene(scene);
    ui->compositeView->setScene(scene);
    ui->presetView->setScene(scene);

    // Keyboard shortcuts
    zoomIn = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus), this);
    zoomOut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus), this);

    QShortcut::connect(zoomIn, &QShortcut::activated, std::bind(&MainWindow::incrementZoom, this, 1));
    QShortcut::connect(zoomOut, &QShortcut::activated, std::bind(&MainWindow::incrementZoom, this, -1));

    // Status bar
    status = new QLabel();
    QLabel::connect(status, &QLabel::linkActivated, [this](const QString &link) { QDesktopServices::openUrl(QUrl(link)); });
    ui->statusbar->addPermanentWidget(status, 1);
    QProgressBar *_progressBar = new QProgressBar();
    _progressBar->setRange(0, 100);
    _progressBar->setValue(0);
    _progressBar->setTextVisible(false);
    ui->statusbar->addPermanentWidget(_progressBar, 1);
    cancel_button = new QPushButton("Cancel");
    ui->statusbar->addPermanentWidget(cancel_button);

    // Cancel button
    QPushButton::connect(cancel_button, &QPushButton::pressed, [this]() {
        if (decoder != nullptr) {
            decoder->stop();
            while (decoder != nullptr) {
                asm("nop");
            }
        }
        if (fingerprinter != nullptr) {
            fingerprinter->stop();
            while (fingerprinter != nullptr) {
                asm("nop");
            }
        }
    });

    // Progress bar
    QTimer *timer = new QTimer(this);
    QTimer::connect(timer, &QTimer::timeout, this, [this, _progressBar]() {
        if (decoder != nullptr) {
            _progressBar->setValue(decoder->progress() * 100.0f);
        } else {
            _progressBar->setValue(0);
        }
    });
    timer->start(1000.0f / 30.0f);

    // Decoding
    decodeWatcher = new QFutureWatcher<void>(this);
    QFutureWatcher<void>::connect(decodeWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::decodeFinished);

    sensor_select = new QActionGroup(this);
    QActionGroup::connect(sensor_select, &QActionGroup::triggered, [this](QAction *action) {
        sensor = get_sensor(action->text().toStdString());
        decodeFinished();
        ui->actionEnable_Overlay->setEnabled(!compositors[sensor]->overlay.empty());
        ui->actionEnable_Overlay->setChecked(compositors[sensor]->enable_map);
    });

    setState(WindowState::Idle);

    project_diag = new ProjectDialog(this);
    ProjectDialog::connect(project_diag, &ProjectDialog::get_viewport, [this]() -> QImage {
        QImage copy(display);
        compositors[sensor]->equalise(copy, selectedEqualization, clip_limit, ui->brightnessOnly->isChecked());
        compositors[sensor]->enable_map = false;
        compositors[sensor]->enable_landmarks = false;
        compositors[sensor]->postprocess(copy);
        compositors[sensor]->enable_map = ui->actionEnable_Map->isChecked();
        compositors[sensor]->enable_landmarks = ui->actionEnable_Landmarks->isChecked();
        if (compositors[sensor]->flipped()) {
            copy = copy.mirrored(true, true);
        }
        return copy;
    });
    ProjectDialog::connect(project_diag, &ProjectDialog::get_points, [this](size_t n) -> std::vector<std::pair<xy, Geodetic>> {
        size_t width = compositors[sensor]->width();
        size_t height = compositors[sensor]->height();
        size_t y = round((double)height / (double)width * (double)n);

        return proj->calculate_gcps(timestamps[sensor], y, n, sensor, sat, width);
    });
    ProjectDialog::connect(project_diag, &ProjectDialog::map_shapefile, [this]() -> QString { return map_shapefile; });
    ProjectDialog::connect(project_diag, &ProjectDialog::landmark_file, [this]() -> QString { return landmark_file; });
    ProjectDialog::connect(project_diag, &ProjectDialog::map_color, [this]() -> QColor { return map_color; });
    ProjectDialog::connect(project_diag, &ProjectDialog::landmark_color, [this]() -> QColor { return landmark_color; });
    ProjectDialog::connect(project_diag, &ProjectDialog::map_enable,
                           [this]() -> bool { return ui->actionEnable_Map->isChecked(); });
    ProjectDialog::connect(project_diag, &ProjectDialog::landmark_enable,
                           [this]() -> bool { return ui->actionEnable_Landmarks->isChecked(); });
    ProjectDialog::connect(project_diag, &ProjectDialog::default_filename, [this]() -> QString { return getDefaultFilename(); });

    UpdateChecker *update_checker = new UpdateChecker();
    UpdateChecker::connect(update_checker, &UpdateChecker::updateAvailable, [this](QString url) {
        status->setText(QString("<a href=\"%1\">A new version is available, click here to get it</a>").arg(url));
    });

    gradient_manager = new GradientManager();
    ui->gradient->addItem("None");
    for (const auto &i : gradient_manager->gradients) {
        ui->gradient->addItem(QString::fromStdString(i.first));
    }

    color_dialog = new QColorDialog(this);
    color_dialog->setCurrentColor(map_color);
    QColorDialog::connect(color_dialog, &QColorDialog::colorSelected, this, [this](QColor color) {
        QSettings settings;
        settings.setValue("map/color", color);
        settings.sync();

        map_color = color;
        ui->actionMap_Color->setText(QString("Map Color (%1)").arg(color.name()));
        on_actionEnable_Map_triggered();
    });
    landmark_color_dialog = new QColorDialog(this);
    landmark_color_dialog->setCurrentColor(map_color);
    QColorDialog::connect(landmark_color_dialog, &QColorDialog::colorSelected, this, [this](QColor color) {
        QSettings settings;
        settings.setValue("landmark/color", color);
        settings.sync();

        landmark_color = color;
        ui->actionLandmark_Color->setText(QString("Landmark Color (%1)").arg(color.name()));
        on_actionEnable_Landmarks_triggered();
    });

    setAcceptDrops(true);

    QSettings settings;
    if (settings.contains("map/shapefile")) {
        map_shapefile = settings.value("map/shapefile").toString();
        ui->actionMap_Shapefile->setText(QString("Map Shapefile (%1)").arg(QFileInfo(map_shapefile).completeBaseName()));
        ui->actionEnable_Map->setEnabled(true);
    }
    if (settings.contains("landmark/file")) {
        landmark_file = settings.value("landmark/file").toString();
        ui->actionLandmark_File->setText(QString("Landmark File (%1)").arg(QFileInfo(landmark_file).completeBaseName()));
        ui->actionEnable_Landmarks->setEnabled(true);
    }
    if (settings.contains("map/color")) {
        map_color = settings.value("map/color").value<QColor>();
        ui->actionMap_Color->setText(QString("Map Color (%1)").arg(map_color.name()));
        color_dialog->setCurrentColor(map_color);
    }
    if (settings.contains("landmark/color")) {
        landmark_color = settings.value("landmark/color").value<QColor>();
        ui->actionLandmark_Color->setText(QString("Landmark Color (%1)").arg(landmark_color.name()));
        landmark_color_dialog->setCurrentColor(landmark_color);
    }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::closeEvent(QCloseEvent *event) {
    if (savingImage) {
        QMessageBox confirm;
        confirm.setText("Are you sure you want to quit? There are image(s) currently being saved.");
        confirm.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        confirm.setDefaultButton(QMessageBox::Cancel);
        confirm.setIcon(QMessageBox::Warning);

        if (confirm.exec() == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    if (decoder != nullptr) {
        QMessageBox confirm;
        confirm.setText("Are you sure you want to quit? There is currently a decode running.");
        confirm.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        confirm.setDefaultButton(QMessageBox::Cancel);
        confirm.setIcon(QMessageBox::Warning);

        if (confirm.exec() == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    clean_up = true;
    if (decoder != nullptr) {
        decoder->stop();
        while (decoder != nullptr) {
            asm("nop");
        }
    }
    if (fingerprinter != nullptr) {
        fingerprinter->stop();
        while (fingerprinter != nullptr) {
            asm("nop");
        }
    }

    for (const auto &compositor : compositors) {
        delete compositor.second;
    }

    event->accept();
}

void MainWindow::incrementZoom(int amount) {
    int newIndex = clamp(ui->zoomSelector->currentIndex() + amount, 0, ui->zoomSelector->count() - 1);
    ui->zoomSelector->setCurrentIndex(newIndex);
    on_zoomSelector_activated(newIndex);
}

void MainWindow::setState(WindowState state) {
    QWidget *items[] = {ui->groupBox,       ui->menuGeo,         ui->menuGeo,   ui->menuOptions, ui->menuSensor,
                        ui->stackedOptions, ui->zoomSelectorBox, ui->imageTabs, ui->gradientBox};

    for (QWidget *item : items) {
        item->setEnabled(state == WindowState::Finished);
    }

    cancel_button->setEnabled(state == WindowState::Decoding);
    ui->actionOpen->setEnabled(state != WindowState::Decoding);
    ui->actionSave_All_Channels->setEnabled(state == WindowState::Finished);
    ui->actionSave_Current_Image->setEnabled(state == WindowState::Finished);
    ui->actionSave_Current_Image_Corrected->setEnabled(state == WindowState::Finished);
    ui->actionSave_GCP_File->setEnabled(state == WindowState::Finished && have_tles);
    zoomIn->setEnabled(state == WindowState::Finished);
    zoomOut->setEnabled(state == WindowState::Finished);
}

void MainWindow::populateChannelSelectors(size_t channels) {
    QComboBox *comboBoxes[] = {ui->channelSelector, ui->redSelector, ui->greenSelector, ui->blueSelector};

    for (QComboBox *comboBox : comboBoxes) {
        comboBox->clear();
        for (size_t i = 0; i < channels; i++) {
            comboBox->addItem(QString("Channel %1").arg(i + 1));
        }
    }

    setComposite({2, 2, 1});
}

void MainWindow::on_actionOpen_triggered() {
    QString filename = QFileDialog::getOpenFileName(
        this, "Open File", "", "Supported formats (*.bin *.cadu *.raw16 *.hrp *.vcdu *.tip *.dec);;All files (*)");

    if (!filename.isEmpty()) {
        decodeWatcher->setFuture(QtConcurrent::run([=]() { startDecode(filename.toStdString()); }));
    }
}

void MainWindow::startDecode(std::string filename) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Fingerprint
    scene->clear();
    setState(WindowState::Decoding);
    ui->contrastLimitApply->setEnabled(false);
    status->setText("Fingerprinting");

    fingerprinter = new Fingerprint;
    FileType type;
    Protocol protocol;
    std::tie(sat, type, protocol) = fingerprinter->file(filename,fingerprinterSuggestion);
    if (sat == SatID::Unknown) {
        delete fingerprinter;
        fingerprinter = nullptr;
        return;
    }
    delete fingerprinter;
    fingerprinter = nullptr;
    SatelliteInfo satellite = satellite_info.at(sat);

    // Decode
    status->setText(QString("Decoding %1...").arg(QString::fromStdString(filename)));
    decoder = Decoder::make(protocol, sat);
    decoder->decodeFile(filename, type);
    if (clean_up) {
        sat = SatID::Unknown;
        delete decoder;
        decoder = nullptr;
        return;
    }

    Data data = decoder->get();

    for (auto action : sensor_actions) {
        sensor_select->removeAction(action.second);
        ui->menuSensor->removeAction(action.second);
        delete action.second;
    }

    sensor_actions.clear();
    for (auto compositor : compositors) {
        delete compositor.second;
    }
    for (auto sensor2 : data.imagers) {
        SensorInfo info = sensor_info.at(sensor2.first);
        QAction *action = new QAction(QString::fromStdString(info.name));
        action->setCheckable(true);
        sensor_actions.insert({info.name, action});
        sensor_select->addAction(action);
        ui->menuSensor->addAction(action);

        compositors[sensor2.first] = new ImageCompositor;
        compositors[sensor2.first]->ch3a = data.ch3a;
        compositors[sensor2.first]->import(sensor2.second, sat, sensor2.first, data.caldata, protocol == Protocol::GACReverse);
        float sum = 0.0;
        for (const bool &x : data.ch3a) {
            sum += x;
        }
        compositors[sensor2.first]->has_ch3a = (sum / (float)data.ch3a.size() > 0.5f);
    }

    if (compositors.count(satellite.default_imager)) {
        default_sensor = sensor = satellite.default_imager;
    } else {
        default_sensor = sensor = (*data.imagers.begin()).first;
    }
    sensor_actions.at(sensor_info.at(sensor).name)->setChecked(true);

    have_tles = false;
    if (tle_manager.catalog_by_norad.count((int)sat)) {
        proj = new Projector(tle_manager.catalog_by_norad[(int)sat]);
        have_tles = true;
    }

    timestamps = data.timestamps;

    // Anomaly detection and interpolation
    for (auto &sensor : timestamps) {
        std::vector<double> &timestamp = sensor.second;

        if (protocol == Protocol::GACReverse) {
            reverse(timestamp);
        }

        timestamp = filter_timestamps(timestamp);
    }

    for (auto &sensor : timestamps) {
        if (have_tles)
            compositors[sensor.first]->sunz =
                proj->calculate_sunz(sensor.second, sensor.first, sat, compositors[sensor.first]->width());
    }

    if (timestamps.count(sensor) && timestamps.at(sensor).size() > 0) {
        pass_timestamp = timestamps[sensor][timestamps[sensor].size() / 2];
    }

    if (have_tles && timestamps.at(default_sensor).size() > 0) {
        ui->actionFlip->setChecked(proj->is_northbound(timestamps.at(default_sensor)));
    }

    delete decoder;
    decoder = nullptr;
    QApplication::restoreOverrideCursor();
}

void MainWindow::decodeFinished() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (sat == SatID::Unknown) {
        QApplication::restoreOverrideCursor();
        status->setText("Fingerprinting failed");
        setState(WindowState::Idle);
        return;
    }

    if (compositors.at(default_sensor)->height() == 0) {
        QApplication::restoreOverrideCursor();
        status->setText("Decode failed");
        setState(WindowState::Idle);
        return;
    }
    display = QImage(compositors.at(sensor)->width(), compositors.at(sensor)->height(), QImage::Format_RGBX64);

    // Prepare the UI
    compositors[sensor]->enable_map = false;
    selectedChannel = 1;
    populateChannelSelectors(compositors.at(sensor)->channels());
    reloadPresets();
    ui->gradient->setCurrentIndex(0);
    compositors[sensor]->stops = {};
    ui->gradientView->stops = {};
    ui->gradientView->repaint();
    status->setText(QString("%1 - %2: %3 lines")
                        .arg(QString::fromStdString(satellite_info.at(sat).name))
                        .arg(QString::fromStdString(sensor_info.at(sensor).name))
                        .arg(compositors.at(sensor)->height()));
    QApplication::restoreOverrideCursor();
    setState(WindowState::Finished);
    on_actionFlip_triggered();
    ui->actionEnable_Overlay->setChecked(false);
    ui->actionIR_Blend->setChecked(false);
    ui->actionEnable_Map->setChecked(false);
    ui->actionIR_Blend->setEnabled(compositors.at(sensor)->sunz.size() != 0 && sensor != Imager::MHS && sensor != Imager::MTVZA &&
                                   sensor != Imager::HIRS);
}

// Highlight the clip limit number and enable apply box as a reminder you didn't apply it yet
void MainWindow::colorClipLimit(bool changed) {
    if (changed){
        ui->contrastLimit->setStyleSheet("color: rgb(0,0,200); ");
        ui->contrastLimitApply->setEnabled(true);
    } else {
        ui->contrastLimit->setStyleSheet("");
        ui->contrastLimitApply->setEnabled(false);
    }
}

void MainWindow::reloadPresets() {
    selected_presets.clear();
    for (auto preset : manager.presets) {
        std::string expression = preset.second.expression;
        if (preset.second.overrides.count(sensor)) {
            expression = preset.second.overrides.at(sensor);
        }

        if (preset.second.imagers.count(sensor)) {
            if (sensor == Imager::AVHRR && expression.find("SWIR") != std::string::npos) {
                if (compositors[sensor]->has_ch3a) {
                    selected_presets.insert(preset);
                }
            } else {
                selected_presets.insert(preset);
            }
        }
    }

    if (selected_presets.size() == 0) {
        std::set<Imager> sensors;
        for (const auto &sensor : sensor_info) {
            sensors.insert(sensor.first);
        }
        Preset preset = {"", "", "", sensors, "1", {}};
        selected_presets.insert({"Unable to load presets", preset});
    }

    QString text = ui->presetSelector->currentText();
    ui->presetSelector->clear();
    for (auto item : selected_presets) {
        ui->presetSelector->addItem(QString::fromStdString(item.first));
    }

    int index = ui->presetSelector->findText(text);
    if (index != -1) {
        ui->presetSelector->setCurrentIndex(index);
    } else {
        ui->presetSelector->setCurrentIndex(0);
    }
    on_presetSelector_textActivated(ui->presetSelector->currentText());
}

// Zoom selector combo box
void MainWindow::on_zoomSelector_activated(int index) {
    float zoomLevels[] = {0.25f, 0.5f, 1.0f, 2.0f};
    QGraphicsView *views[] = {ui->channelView, ui->compositeView, ui->presetView};

    for (QGraphicsView *view : views) {
        view->resetTransform();
        view->scale(zoomLevels[index], zoomLevels[index]);
    }
}

void MainWindow::setChannel(int sensor_channel) {
    selectedChannel = sensor_channel + 1;
    updateDisplay();
}
void MainWindow::setCompositeChannel(int channel, int sensor_channel) {
    selectedComposite[channel] = sensor_channel + 1;
    updateDisplay();
}
void MainWindow::setComposite(std::array<size_t, 3> channels) {
    QComboBox *selectors[] = {ui->redSelector, ui->greenSelector, ui->blueSelector};

    for (size_t i = 0; i < channels.size(); i++) {
        selectors[i]->setCurrentIndex(channels[i] - 1);
        selectedComposite[i] = channels[i];
    }
}

void MainWindow::setEqualization(Equalization type) {
    // TODO make it so it doesnt re-apply correction with each equ
    selectedEqualization = type;
    if (selectedEqualization == Equalization::None) {
        QImage copy(display);
        compositors[sensor]->postprocess(copy);
        updateDisplay();
    } else {
        QImage copy(display);
        ImageCompositor::equalise(copy, selectedEqualization, clip_limit, ui->brightnessOnly->isChecked());
        compositors[sensor]->postprocess(copy);
        updateDisplay();
    }
}

void MainWindow::on_actionFlip_triggered() {
    compositors.at(sensor)->setFlipped(ui->actionFlip->isChecked());
    updateDisplay();
}

void MainWindow::on_actionCorrect_triggered() {
    updateDisplay();
}

void MainWindow::on_groupProtocol_triggered() {
    if (ui->groupProtocol->checkedAction() == ui->actionProtocolAutomatic){
        fingerprinterSuggestion = Suggestion::Automatic;
    } else if (ui->groupProtocol->checkedAction() == ui->actionProtocolPOESHRPT){
        fingerprinterSuggestion = Suggestion::POESHRPT;
    } else if (ui->groupProtocol->checkedAction() == ui->actionProtocolPOESGAC){
        fingerprinterSuggestion = Suggestion::POESGAC;
    } else if (ui->groupProtocol->checkedAction() == ui->actionProtocolPOESDSB){
        fingerprinterSuggestion = Suggestion::POESDSB;
    } else if (ui->groupProtocol->checkedAction() == ui->actionProtocolMeteorHRPT){
        fingerprinterSuggestion = Suggestion::MeteorHRPT;
    } else if (ui->groupProtocol->checkedAction() == ui->actionProtocolMeteorLRPT){
        fingerprinterSuggestion = Suggestion::MeteorLRPT;
    } else if (ui->groupProtocol->checkedAction() == ui->actionProtocolMetOp){
        fingerprinterSuggestion = Suggestion::MetOpHRPT;
    } else if (ui->groupProtocol->checkedAction() == ui->actionProtocolFY3){
        fingerprinterSuggestion = Suggestion::FengYunHRPT;
    }
    
    // TODO remember the path to the current file instead of asking to reopen it 

    QString filename = QFileDialog::getOpenFileName(
        this, "Open File", "", "Supported formats (*.bin *.cadu *.raw16 *.hrp *.vcdu *.tip *.dec);;All files (*)");

    if (!filename.isEmpty()) {
        decodeWatcher->setFuture(QtConcurrent::run([=]() { startDecode(filename.toStdString()); }));
    }
    
}

void MainWindow::on_actionIR_Blend_triggered() {
    compositors.at(sensor)->enableIRBlend(ui->actionIR_Blend->isChecked());
    updateDisplay();
}

void MainWindow::on_imageTabs_currentChanged(int index) {
    QGraphicsView *tabs[] = {ui->channelView, ui->compositeView, ui->presetView};
    tabs[index]->horizontalScrollBar()->setValue(tabs[previousTabIndex]->horizontalScrollBar()->value());
    tabs[index]->verticalScrollBar()->setValue(tabs[previousTabIndex]->verticalScrollBar()->value());
    previousTabIndex = index;

    updateDisplay();
}

void MainWindow::get_source(QImage &image) {
    switch (ui->imageTabs->currentIndex()) {
        case 0:
            compositors.at(sensor)->getChannel(image, selectedChannel);
            break;
        case 1:
            compositors.at(sensor)->getComposite(image, selectedComposite);
            break;
        case 2: {
            Preset preset = selected_presets.at(ui->presetSelector->currentText().toStdString());

            std::string expression = preset.expression;
            if (preset.overrides.count(sensor)) {
                expression = preset.overrides.at(sensor);
            }
            compositors.at(sensor)->getExpression(image, expression);
            break;
        }
        default:
            throw std::runtime_error("invalid tab index");
    }
}

void MainWindow::updateDisplay() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    get_source(display);
    if (selectedEqualization == Equalization::None) {
        QImage copy(display);
        compositors[sensor]->postprocess(copy);
            if (ui->actionCorrect->isChecked()){
                copy = correct_geometry(copy, sat, sensor, compositors[sensor]->width());
            }
        displayQImage(scene, copy);
    } else {
        QImage copy(display);
        ImageCompositor::equalise(copy, selectedEqualization, clip_limit, ui->brightnessOnly->isChecked());
        compositors[sensor]->postprocess(copy);
            if (ui->actionCorrect->isChecked()){
                copy = correct_geometry(copy, sat, sensor, compositors[sensor]->width());
            }
        displayQImage(scene, copy);
    }

    ui->gradient->setEnabled(display.format() == QImage::Format_Grayscale16);
    QApplication::restoreOverrideCursor();
}

QString MainWindow::getDefaultFilename() {
    QString composite;
    for (auto channel : selectedComposite) {
        if (channel < 10) {
            composite.push_back(QString::number(channel));
        } else {
            composite.push_back("[" + QString::number(channel) + "]");
        }
    }

    QString types[3] = {QString::number(selectedChannel), composite, ui->presetSelector->currentText()};
    return QString("%1_%2_%3_%4")
        .arg(QString::fromStdString(satellite_info.at(sat).name))
        .arg(QString::fromStdString(sensor_info.at(sensor).name))
        .arg(QDateTime::fromSecsSinceEpoch(pass_timestamp, Qt::UTC).toString("yyyyMMdd-hhmmss"))
        .arg(types[ui->imageTabs->currentIndex()]);
}

void MainWindow::saveCurrentImage() {

    corrected = ui->actionCorrect->isChecked();

    QString filename = QFileDialog::getSaveFileName(this, "Save Current Image", getDefaultFilename() + ".png",
                                                    "PNG (*.png);;JPEG (*.jpg *.jpeg);;WEBP (*.webp);; BMP (*.bmp)");

    if (filename.isEmpty()) {
        return;
    }

    savingImage = true;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QtConcurrent::run(
        [this](QString filename, bool corrected) {
            QImage image(compositors[sensor]->width(), compositors[sensor]->height(), QImage::Format_RGBX64);
            get_source(image);
            ImageCompositor::equalise(image, selectedEqualization, clip_limit, ui->brightnessOnly->isChecked());
            compositors[sensor]->postprocess(image, corrected);
            image.save(filename);

            savingImage = false;
            QApplication::restoreOverrideCursor();
        },
        filename, corrected);
}

void MainWindow::saveAllChannels() {
    QString directory = QFileDialog::getExistingDirectory(this, "Save All Channels", "",
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (directory.isEmpty()) return;

    savingImage = true;
    QtConcurrent::run(
        [this](QString directory) {
            QImage channel(compositors.at(sensor)->width(), compositors.at(sensor)->height(), QImage::Format_Grayscale16);

            for (size_t i = 0; i < compositors.at(sensor)->channels(); i++) {
                status->setText(QString("Saving channel %1...").arg(i + 1));
                compositors.at(sensor)->getChannel(channel, i + 1);
                channel.save(QString("%1/%2_%3_%4_%5.png")
                                 .arg(directory)
                                 .arg(QString::fromStdString(satellite_info.at(sat).name))
                                 .arg(QString::fromStdString(sensor_info.at(sensor).name))
                                 .arg(QDateTime::fromSecsSinceEpoch(pass_timestamp, Qt::UTC).toString("yyyyMMdd-hhmmss"))
                                 .arg(i + 1),
                             "PNG");
            }

            status->setText("Done");
            savingImage = false;
        },
        directory);
}

void MainWindow::save_gcp() {
    if (tle_manager.catalog.size() == 0) {
        QMessageBox::warning(this, "Error", "No TLEs loaded, cannot save control points.", QMessageBox::Ok);
        return;
    }

    QString name = QString("%1_%2_%3.gcp")
                       .arg(QString::fromStdString(satellite_info.at(sat).name))
                       .arg(QString::fromStdString(sensor_info.at(sensor).name))
                       .arg(QDateTime::fromSecsSinceEpoch(pass_timestamp, Qt::UTC).toString("yyyyMMdd-hhmmss"));
    QString filename = QFileDialog::getSaveFileName(this, "Save GCP File", name, "GCP (*.gcp)");
    if (filename.isEmpty()) return;
    double width = compositors[sensor]->width();
    double height = compositors[sensor]->height();
    proj->save_gcp_file(timestamps[sensor], (double)height / (double)width * 21.0, 21, sensor, sat, filename.toStdString(),
                        width);
}

void MainWindow::on_presetSelector_textActivated(QString text) {
    Preset preset = selected_presets.at(text.toStdString());
    ui->presetDescription->setText(QString::fromStdString(preset.description));
    ui->presetAuthor->setText(QString::fromStdString(preset.author));
    ui->presetCategory->setText(QString::fromStdString(preset.category));

    updateDisplay();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->urls().size() == 1) {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e) {
    for (const QUrl &url : e->mimeData()->urls()) {
        QString filename = url.toLocalFile();
        decodeWatcher->setFuture(QtConcurrent::run([=]() { startDecode(filename.toStdString()); }));
    }
}

void MainWindow::on_gradient_textActivated(const QString &text) {
    std::vector<QColor> gradient;
    if (text != "None") {
        gradient = gradient_manager->gradients.at(text.toStdString()).stops;
    }

    ui->gradientView->stops = gradient;
    ui->gradientView->repaint();
    compositors[sensor]->stops = gradient;

    updateDisplay();
}

void MainWindow::on_actionMap_Shapefile_triggered() {
    QString filename = QFileDialog::getOpenFileName(this, "Open File", "", "Shapefiles (*.shp)");
    if (filename.isEmpty()) return;

    if (!map::verify_shapefile(filename.toStdString())) {
        QMessageBox::critical(
            this, "Error",
            "Unable to open Shapefile, this may mean a required file is missing (check .shx/dbf) or that it's unsupported.");
        return;
    }

    ui->actionMap_Shapefile->setText(QString("Map Shapefile (%1)").arg(QFileInfo(filename).completeBaseName()));
    ui->actionEnable_Map->setEnabled(true);
    bool update = !map_shapefile.isEmpty();
    map_shapefile = filename;

    QSettings settings;
    settings.setValue("map/shapefile", map_shapefile);
    settings.sync();

    if (update) {
        on_actionEnable_Map_triggered();
    }
}

void MainWindow::on_actionLandmark_File_triggered() {
    QString filename = QFileDialog::getOpenFileName(this, "Open File", "", "Landmark Files (*.csv)");
    if (filename.isEmpty()) return;

    if (map::read_landmarks(filename.toStdString()).size() == 0) {
        QMessageBox::critical(this, "Error", "Unable to open Landmark file.");
        return;
    }

    ui->actionLandmark_File->setText(QString("Map Shapefile (%1)").arg(QFileInfo(filename).completeBaseName()));
    ui->actionEnable_Landmarks->setEnabled(true);
    bool update = !landmark_file.isEmpty();
    landmark_file = filename;

    QSettings settings;
    settings.setValue("landmark/file", landmark_file);
    settings.sync();

    if (update) {
        on_actionEnable_Landmarks_triggered();
    }
}

void MainWindow::on_actionEnable_Map_triggered() {
    if (!ui->actionEnable_Map->isChecked()) {
        compositors[sensor]->enable_map = false;
        updateDisplay();
        return;
    }

    size_t yn = (double)compositors[sensor]->height() / (double)compositors[sensor]->width() * 21.0;
    size_t xn = 21;
    auto points = proj->calculate_gcps(timestamps[sensor], yn, xn, sensor, sat, compositors[sensor]->width());

    std::vector<QLineF> shapefile = map::read_shapefile(map_shapefile.toStdString());
    std::array<std::vector<QLineF>, 36 * 18> buckets = map::index_line_segments(shapefile);
    compositors[sensor]->map_color = map_color;
    compositors[sensor]->overlay = map::warp_to_pass(buckets, points, xn);
    compositors[sensor]->enable_map = true;

    updateDisplay();
}

void MainWindow::on_actionEnable_Landmarks_triggered() {
    if (!ui->actionEnable_Landmarks->isChecked()) {
        compositors[sensor]->enable_landmarks = false;
        updateDisplay();
        return;
    }

    size_t yn = (double)compositors[sensor]->height() / (double)compositors[sensor]->width() * 21.0;
    size_t xn = 21;
    auto points = proj->calculate_gcps(timestamps[sensor], yn, xn, sensor, sat, compositors[sensor]->width());

    std::vector<Landmark> landmarks = map::read_landmarks(landmark_file.toStdString());
    compositors[sensor]->landmark_color = landmark_color;
    compositors[sensor]->landmarks = map::warp_to_pass(landmarks, points, xn);
    compositors[sensor]->enable_landmarks = true;

    updateDisplay();
}