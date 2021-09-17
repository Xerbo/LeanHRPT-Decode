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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include <QScrollBar>
#include <QProgressBar>
#include <QCloseEvent>

#include "fingerprint.h"
#include "geometry.h"

#include "decoders/meteor.h"
#include "decoders/noaa.h"
#include "decoders/fengyun.h"
#include "decoders/metop.h"

template<typename T>
T clamp(T v, T lo, T hi) {
    return std::max(lo, std::min(hi, v));
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    ui = new Ui::MainWindow;
    ui->setupUi(this);

    // Image display
    scene = new QGraphicsScene;
    ui->channelView->setScene(scene);
    ui->compositeView->setScene(scene);
    ui->presetView->setScene(scene);

    // Keyboard shortcuts
    zoomIn  = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus),  this);
    zoomOut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this);
    flip    = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F),     this);

    QShortcut::connect(zoomIn,  &QShortcut::activated, std::bind(&MainWindow::incrementZoom, this,  1));
    QShortcut::connect(zoomOut, &QShortcut::activated, std::bind(&MainWindow::incrementZoom, this, -1));
    QShortcut::connect(flip, &QShortcut::activated, [this]() {
        ui->actionFlip->setChecked(!ui->actionFlip->isChecked());
        on_actionFlip_triggered();
    });

    // Status bar
    status = new QLabel();
    ui->statusbar->addPermanentWidget(status, 2);
    QProgressBar * _progressBar = new QProgressBar();
    _progressBar->setRange(0, 100);
    _progressBar->setValue(0);
    _progressBar->setTextVisible(false);
    ui->statusbar->addPermanentWidget(_progressBar, 1);

    // Progress bar
    QTimer *timer = new QTimer(this);
    QTimer::connect(timer, &QTimer::timeout, this, [this, _progressBar]() {
        if (decoder != nullptr) {
            _progressBar->setValue(decoder->progress()*100.0f);
        } else {
            _progressBar->setValue(0);
        }
    });
    timer->start(1000.0f/30.0f);

    // Decoding
    decodeWatcher = new QFutureWatcher<void>(this);
    QFutureWatcher<void>::connect(decodeWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::decodeFinished);

    setState(WindowState::Idle);

    sensor_select = new QActionGroup(this);
    QActionGroup::connect(sensor_select, &QActionGroup::triggered, [this](QAction *action) {
        sensor = sensors.at(action->text().toStdString());
        decodeFinished();
    });
}

MainWindow::~MainWindow() {
    delete zoomIn;
    delete zoomOut;
    delete flip;
    delete scene;
    delete decodeWatcher;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (savingImage) {
        QMessageBox confirm;
        confirm.setText("Are you sure you want to quit? There are images currently being saved.");
        confirm.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        confirm.setDefaultButton(QMessageBox::Cancel);
        confirm.setIcon(QMessageBox::Warning);

        if (confirm.exec() == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    if (decoder != nullptr) {
        decoder->stop();
    }

    event->accept();
}

void MainWindow::incrementZoom(int amount) {
    int newIndex = clamp(ui->zoomSelector->currentIndex() + amount, 0, ui->zoomSelector->count()-1);
    ui->zoomSelector->setCurrentIndex(newIndex);
    on_zoomSelector_activated(newIndex);
}

void MainWindow::setState(WindowState state) {
    QWidget *items[] = { ui->groupBox, ui->menuOptions, ui->menuSensor, ui->stackedOptions, ui->zoomSelectorBox, ui->imageTabs };

    for (QWidget *item : items) {
        item->setEnabled(state == WindowState::Finished);
    }

    ui->actionSave_All_Channels->setEnabled(state == WindowState::Finished);
    ui->actionSave_Current_Image->setEnabled(state == WindowState::Finished);
    ui->actionSave_Current_Image_Corrected->setEnabled(state == WindowState::Finished);
    zoomIn->setEnabled(state == WindowState::Finished);
    zoomOut->setEnabled(state == WindowState::Finished);
    flip->setEnabled(state == WindowState::Finished);
}

void MainWindow::populateChannelSelectors(size_t channels) {
    QComboBox *comboBoxes[] = { ui->channelSelector, ui->redSelector, ui->greenSelector, ui->blueSelector };

    for (QComboBox *comboBox : comboBoxes) {
        comboBox->clear();
        for (size_t i = 0; i < channels; i++) {
            comboBox->addItem(QString("Channel %1").arg(i + 1));
        }
    }

    setComposite({2, 2, 1});
}

void MainWindow::on_actionOpen_triggered() {
    QString filename = QFileDialog::getOpenFileName(this, "Open File", "", "Binary files (*.bin *.cadu)");

    if (!filename.isEmpty()) {
        decodeWatcher->setFuture(QtConcurrent::run(this, &MainWindow::startDecode, filename.toStdString()));
    }
}

void MainWindow::startDecode(std::string filename) {
    // Fingerprint
    setState(WindowState::Decoding);
    status->setText("Fingerprinting");

    sat = Fingerprint::file(filename);
    if (sat == SatID::Unknown) {
        status->setText("Fingerprinting failed");
        return;
    }
    SatelliteInfo satellite = satellite_info.at(sat);

    // Decode
    status->setText(QString("Decoding %1...").arg(QString::fromStdString(filename)));
    switch (satellite.mission) {
        case Mission::FengYun3: decoder = new FengyunDecoder; break;
        case Mission::MeteorM:  decoder = new MeteorDecoder; break;
        case Mission::MetOp:    decoder = new MetOpDecoder; break;
        case Mission::POES:     decoder = new NOAADecoder; break;
        default: throw std::runtime_error("invalid value in enum `Mission`");
    }
    decoder->decodeFile(filename);

    Data data = decoder->get();

    for (auto action : sensor_actions) {
        sensor_select->removeAction(action.second);
        ui->menuSensor->removeAction(action.second);
        delete action.second;
    }

    sensor_actions.clear();
    for (auto sensor2 : data.imagers) {
        SensorInfo info = sensor_info.at(sensor2.first);
        QAction *action = new QAction(QString::fromStdString(info.name));
        action->setCheckable(true);
        sensor_actions.insert({info.name, action});
        sensor_select->addAction(action);
        ui->menuSensor->addAction(action);

        compositors[sensor2.first] = new ImageCompositor;
        compositors[sensor2.first]->import(sensor2.second, sat, sensor2.first);
    }

    sensor = satellite.default_imager;
    sensor_actions.at(sensor_info.at(satellite.default_imager).name)->setChecked(true);

    delete decoder;
    decoder = nullptr;
}

void MainWindow::decodeFinished() {
    if (compositors.at(satellite_info.at(sat).default_imager)->height() == 0) {
        status->setText("Decode failed");
        setState(WindowState::Idle);
        return;
    }
    display = QImage(compositors.at(sensor)->width(), compositors.at(sensor)->height(), QImage::Format_RGBX64);

    // Prepare the UI
    populateChannelSelectors(compositors.at(sensor)->channels());
    status->setText(QString("%1 - %2: %3 lines").arg(QString::fromStdString(satellite_info.at(sat).name)).arg(QString::fromStdString(sensor_info.at(sensor).name)).arg(compositors.at(sensor)->height()));
    setState(WindowState::Finished);

    // Load satellite specific presets
    reloadPresets();
}

void MainWindow::reloadPresets() {
    selected_presets.clear();
    for (auto preset : manager.presets) {
        if (preset.second.imagers.count(sensor)) {
            selected_presets.insert(preset);
        }
    }

    if (selected_presets.size() == 0) {
        Preset preset = { "", "", "", { AVHRR, VIRR, MSUMR, MHS }, "bw(0)" };
        selected_presets.insert({"Unable to load presets", preset});
    }

    int index = ui->presetSelector->currentIndex();
    ui->presetSelector->clear();
    for (auto item : selected_presets) {
        ui->presetSelector->addItem(QString::fromStdString(item.first));
    }
    if (index != -1) {
        ui->presetSelector->setCurrentIndex(index);
    }
    on_presetSelector_activated(ui->presetSelector->currentText());
}

// Zoom selector combo box
void MainWindow::on_zoomSelector_activated(int index) {
    float zoomLevels[] = { 0.25f, 0.5f, 1.0f, 2.0f };
    QGraphicsView *views[] = { ui->channelView, ui->compositeView, ui->presetView };

    for(QGraphicsView *view : views) {
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
    QComboBox *selectors[] = { ui->redSelector, ui->greenSelector, ui->blueSelector };

    for (size_t i = 0; i < channels.size(); i++) {
        selectors[i]->setCurrentIndex(channels[i]-1);
        selectedComposite[i] = channels[i];
    }
}

void MainWindow::setEqualization(Equalization type) {
    selectedEqualization = type;
    if (selectedEqualization == None) {
        displayQImage(scene, display);
    } else {
        QImage copy(display);
        ImageCompositor::equalise(copy, selectedEqualization, clip_limit, ui->brightnessOnly->isChecked());
        displayQImage(scene, copy);
    }
}

void MainWindow::on_actionFlip_triggered() {
    compositors.at(sensor)->flip();
    updateDisplay();
}

void MainWindow::on_imageTabs_currentChanged(int index) {
    QGraphicsView *tabs[] = { ui->channelView, ui->compositeView, ui->presetView };
    tabs[index]->horizontalScrollBar()->setValue(tabs[previousTabIndex]->horizontalScrollBar()->value());
    tabs[index]->verticalScrollBar()->setValue(tabs[previousTabIndex]->verticalScrollBar()->value());
    previousTabIndex = index;

    updateDisplay();
}

void MainWindow::updateDisplay() {
    switch (ui->imageTabs->currentIndex()) {
        case 0: compositors.at(sensor)->getChannel(display, selectedChannel); break;
        case 1: compositors.at(sensor)->getComposite(display, selectedComposite); break;
        case 2: compositors.at(sensor)->getExpression(display, selected_presets.at(ui->presetSelector->currentText().toStdString()).expression); break;
        default: throw std::runtime_error("invalid tab index");
    }

    if (selectedEqualization == None) {
        displayQImage(scene, display);
    } else {
        QImage copy(display);
        ImageCompositor::equalise(copy, selectedEqualization, clip_limit, ui->brightnessOnly->isChecked());
        displayQImage(scene, copy);
    }
}

void MainWindow::saveCurrentImage(bool corrected) {
    QString composite;
    for (auto channel : selectedComposite) {
        if (channel < 10) {
            composite.push_back(QString::number(channel));
        } else {
            composite.push_back("[" + QString::number(channel) + "]");
        }
    }

    QString types[3] = { QString::number(selectedChannel), composite, ui->presetSelector->currentText() };
    QString name = QString("%1_%2_%3.png").arg(QString::fromStdString(satellite_info.at(sat).name)).arg(QString::fromStdString(sensor_info.at(sensor).name)).arg(types[ui->imageTabs->currentIndex()]);
    QString filename = QFileDialog::getSaveFileName(this, "Save Current Image", name, "PNG (*.png);;JPEG (*.jpg *.jpeg);;WEBP (*.webp);; BMP (*.bmp)");

    if (filename.isEmpty()) {
        return;
    }

    savingImage = true;
    QtConcurrent::run([this](QString filename, bool corrected) {
        QImage copy(display);
        ImageCompositor::equalise(copy, selectedEqualization, clip_limit, ui->brightnessOnly->isChecked());
        if (corrected) {
            correct_geometry(copy, sat, sensor).save(filename);
        } else {
            copy.save(filename);
        }
        savingImage = false;
    }, filename, corrected);
}

void MainWindow::saveAllChannels() {
    QString directory = QFileDialog::getExistingDirectory(this, "Save All Channels", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (directory.isEmpty()) return;

    savingImage = true;
    QtConcurrent::run([this](QString directory) {
        QImage channel(compositors.at(sensor)->width(), compositors.at(sensor)->height(), QImage::Format_Grayscale16);

        for(size_t i = 0; i < compositors.at(sensor)->channels(); i++) {
            status->setText(QString("Saving channel %1...").arg(i + 1));
            compositors.at(sensor)->getChannel(channel, i + 1);
            channel.save(QString("%1/%2_%3_%4.png").arg(directory).arg(QString::fromStdString(satellite_info.at(sat).name)).arg(QString::fromStdString(sensor_info.at(sensor).name)).arg(i + 1), "PNG");
        }

        status->setText("Done");
        savingImage = false;
    }, directory);
}

void MainWindow::on_presetSelector_activated(QString text) {
    Preset preset = selected_presets.at(text.toStdString());
    ui->presetDescription->setText(QString::fromStdString(preset.description));
    ui->presetAuthor->setText(QString::fromStdString(preset.author));
    ui->presetCategory->setText(QString::fromStdString(preset.category));

    updateDisplay();
}
