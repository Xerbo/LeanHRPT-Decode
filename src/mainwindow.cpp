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

#include <QUrl>
#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QtConcurrent/QtConcurrent>
#include <QScrollBar>

#include "decoders/meteor.h"
#include "decoders/noaa.h"
#include "decoders/fengyun.h"
#include "decoders/metop.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    compositor = new ImageCompositor;
    graphicsScene = new QGraphicsScene;
    decodeWatcher = new QFutureWatcher<void>(this);
    QFutureWatcher<void>::connect(decodeWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::decodeFinished);

    ui->channelView->setScene(graphicsScene);
    ui->compositeView->setScene(graphicsScene);
    ui->ndviView->setScene(graphicsScene);

    zoomIn = new QShortcut(this);
    zoomIn->setKey(Qt::CTRL + Qt::Key_Plus);
    zoomOut = new QShortcut(this);
    zoomOut->setKey(Qt::CTRL + Qt::Key_Minus);
    flip = new QShortcut(this);
    flip->setKey(Qt::CTRL + Qt::Key_F);

    QShortcut::connect(zoomIn, &QShortcut::activated, [this]() {
        incrementZoom(1);
    });
    QShortcut::connect(zoomOut, &QShortcut::activated, [this]() {
        incrementZoom(-1);
    });
    QShortcut::connect(flip, &QShortcut::activated, this, [this]() {
        ui->actionFlip->setChecked(!ui->actionFlip->isChecked());
        on_actionFlip_triggered();
    });

    setEnabled(false);
}
MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::incrementZoom(int amount) {
    int newIndex = std::max(0, std::min(ui->zoomSelector->currentIndex() + amount, 3));
    ui->zoomSelector->setCurrentIndex(newIndex);
    on_zoomSelector_activated(newIndex);
}

// Oh god
void MainWindow::setEnabled(bool enabled) {
    ui->channelSelector->setEnabled(enabled);
    ui->redSelector->setEnabled(enabled);
    ui->greenSelector->setEnabled(enabled);
    ui->blueSelector->setEnabled(enabled);
    ui->actionSave_Current_Image->setEnabled(enabled);
    ui->actionSave_All_Channels->setEnabled(enabled);
    ui->zoomSelector->setEnabled(enabled);
    ui->equalisationNone->setEnabled(enabled);
    ui->equalisationStretch->setEnabled(enabled);
    ui->equalisationHistogram->setEnabled(enabled);
    ui->actionFlip->setEnabled(enabled);
    zoomIn->setEnabled(enabled);
    zoomOut->setEnabled(enabled);
    flip->setEnabled(enabled);
}

void MainWindow::populateChannelSelectors(int channels) {
    QComboBox *comboBoxes[] = { ui->channelSelector, ui->redSelector, ui->greenSelector, ui->blueSelector };

    for (QComboBox *comboBox : comboBoxes) {
        comboBox->clear();
        for (int i = 0; i < channels; i++)
            comboBox->addItem(QString("Channel %1").arg(i + 1));
    }

    // RGB221
    ui->redSelector->setCurrentIndex(1);
    ui->greenSelector->setCurrentIndex(1);
    ui->blueSelector->setCurrentIndex(0);
}

// Display a QImage on all QGraphicsView objects
void MainWindow::displayQImage(QImage *image) {
    graphicsScene->clear();
    graphicsScene->setSceneRect(0, 0, image->width(), image->height());
    graphicsScene->addPixmap(QPixmap::fromImage(*image));
}

/*
 * These 3 functions are the ones which actually decode the image
 */
void MainWindow::on_actionOpen_triggered() {
    QString filename = QFileDialog::getOpenFileName(this, "Open File", "", "Binary files (*.bin *.raw)");

    if (!filename.isEmpty()) {
        QMessageBox satelliteSelection;
        satelliteSelection.setWindowTitle("Select Satellite");
        satelliteSelection.setText("Which satellite did this file come from?");
        QPushButton *meteorButton = satelliteSelection.addButton("Meteor", QMessageBox::AcceptRole);
        QPushButton *noaaButton = satelliteSelection.addButton("NOAA", QMessageBox::AcceptRole);
        QPushButton *metopButton = satelliteSelection.addButton("MetOp", QMessageBox::AcceptRole);
        QPushButton *fengyunButton = satelliteSelection.addButton("Fengyun", QMessageBox::AcceptRole);
        QPushButton *abortButton = satelliteSelection.addButton(QMessageBox::Abort);
        satelliteSelection.exec();

        Satellite satellite;
        if (satelliteSelection.clickedButton() == meteorButton) {
            satellite = Satellite_Meteor;
        } else if (satelliteSelection.clickedButton() == noaaButton) {
            satellite = Satellite_NOAA;
        } else if (satelliteSelection.clickedButton() == fengyunButton) {
            satellite = Satellite_Fengyun;
        } else if (satelliteSelection.clickedButton() == metopButton) {
            satellite = Satellite_MetOp;
        } else if (satelliteSelection.clickedButton() == abortButton) {
            return;
        } else {
            throw std::runtime_error("invalid button pressed in satellite selector message box");
        }

        ui->statusbar->showMessage(QString("Decoding %1 (might take a while)...").arg(filename));
        decodeWatcher->setFuture(QtConcurrent::run(this, &MainWindow::startDecode, satellite, filename.toStdString()));
    }
}

void MainWindow::startDecode(Satellite satellite, std::string filename) {
    Decoder *decoder;
    switch (satellite) {
        case Satellite_Fengyun: decoder = new FengyunDecoder; break;
        case Satellite_Meteor: decoder = new MeteorDecoder; break;
        case Satellite_MetOp: decoder = new MetOpDecoder; break;
        case Satellite_NOAA: decoder = new NOAADecoder; break;
        default: throw std::runtime_error("invalid value in enum `Satellite`");
    }
    imagerName = QString::fromStdString(decoder->imagerName());
    decoder->decodeFile(filename);

    compositor->importFromRaw(decoder->getImage());

    delete decoder;
}
void MainWindow::decodeFinished() {
    channel   = QImage(compositor->width(), compositor->height(), QImage::Format_Grayscale16);
    composite = QImage(compositor->width(), compositor->height(), QImage::Format_RGBX64);
    ndvi      = QImage(compositor->width(), compositor->height(), QImage::Format_Grayscale16);

    populateChannelSelectors(compositor->channels());
    compositor->getNdvi(&ndvi);
    compositor->getComposite(&composite, selectedComposite);
    compositor->getChannel(&channel, selectedChannel);
    updateEqualization(selectedEqualization);

    ui->statusbar->showMessage(QString("Decode finished: %1, %2 lines").arg(QString(imagerName)).arg(compositor->height()));

    reloadImage();
    setEnabled(true);
}

// Zoom selector combo box
void MainWindow::on_zoomSelector_activated(int index) {
    float zoomLevels[4] = { 0.25, 0.5, 1.0, 2.0 };
    QGraphicsView * views[] = { ui->channelView, ui->compositeView, ui->ndviView };

    for(QGraphicsView *view : views) {
        view->resetMatrix();
        view->scale(zoomLevels[index], zoomLevels[index]);
    }
}

// Channel selector combo boxes
void MainWindow::on_channelSelector_activated(int index) {
    selectedChannel = index + 1;
    compositor->getChannel(&channel, selectedChannel);
    reloadImage();
}
void MainWindow::on_redSelector_activated(int index) {
    selectedComposite[0] = index + 1;
    compositor->getComposite(&composite, selectedComposite);
    reloadImage();
}
void MainWindow::on_greenSelector_activated(int index) {
    selectedComposite[1] = index + 1;
    compositor->getComposite(&composite, selectedComposite);
    reloadImage();
}
void MainWindow::on_blueSelector_activated(int index) {
    selectedComposite[2] = index + 1;
    compositor->getComposite(&composite, selectedComposite);
    reloadImage();
}

// Equalisation buttons
void MainWindow::on_equalisationNone_clicked() {
    selectedEqualization = None;
    updateEqualization(selectedEqualization);
}
void MainWindow::on_equalisationStretch_clicked() {
    selectedEqualization = Stretch;
    updateEqualization(selectedEqualization);
}
void MainWindow::on_equalisationHistogram_clicked() {
    selectedEqualization = Histogram;
    updateEqualization(selectedEqualization);
}
void MainWindow::updateEqualization(Equalization type) {
    compositor->setEqualization(type);
    compositor->getComposite(&composite, selectedComposite);
    compositor->getChannel(&channel, selectedChannel);
    reloadImage();
}

void MainWindow::on_actionFlip_triggered() {
    compositor->flip();
    compositor->getNdvi(&ndvi);
    compositor->getComposite(&composite, selectedComposite);
    compositor->getChannel(&channel, selectedChannel);
    reloadImage();
}

// menuHelp
void MainWindow::on_actionDocumentation_triggered() {
    QDesktopServices::openUrl(QUrl("https://github.com/Xerbo/LeanHRPT-Decode/wiki"));
}
void MainWindow::on_actionIssue_Tracker_triggered() {
    QDesktopServices::openUrl(QUrl("https://github.com/Xerbo/LeanHRPT-Decode/issues"));
}
void MainWindow::on_actionAbout_LeanHRPT_triggered() {
    QMessageBox::about(this, "About LeanHRPT",
"<h2>LeanHRPT Decode</h2>\
Part of the LeanHRPT project - a suite of easy to use software for processing HRPT data.<br/><br/>\
Licensed under GPL-3.0.\
<ul>\
<li><code>Ctrl++</code> Zoom in</li>\
<li><code>Ctrl+-</code> Zoom out</li>\
<li><code>Ctrl+F</code> Flip image</li>\
</ul>");
}
void MainWindow::on_actionAbout_Qt_triggered() {
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::reloadImage() {
    on_imageTabs_currentChanged(ui->imageTabs->currentIndex());
}
void MainWindow::on_imageTabs_currentChanged(int index) {
    // This nightmare fuel simply makes sure that the view port position stays the same throughout tabs
    QGraphicsView *views[] = { ui->channelView, ui->compositeView, ui->ndviView };
    views[index]->horizontalScrollBar()->setValue(views[previousTabIndex]->horizontalScrollBar()->value());
    views[index]->verticalScrollBar()->setValue(views[previousTabIndex]->verticalScrollBar()->value());

    switch (index) {
        case 0: displayQImage(&channel); break;
        case 1: displayQImage(&composite); break;
        case 2: displayQImage(&ndvi); break;
        default: throw std::runtime_error("invalid tab index");
    }

    previousTabIndex = index;
}

void MainWindow::on_actionSave_Current_Image_triggered() {
    int index = ui->imageTabs->currentIndex();

    QString types[3] = { QString::number(selectedChannel), "Composite", "NDVI" };
    QString name = QString("%1-%2.png").arg(imagerName).arg(types[index]);

    QString filename = QFileDialog::getSaveFileName(this, "Save Current Image", name, "Images (*.png *.jpg *.bmp)");

    QtConcurrent::run(this, &MainWindow::saveCurrentImage, filename);
}
void MainWindow::saveCurrentImage(QString filename) {
    int index = ui->imageTabs->currentIndex();
    switch (index) {
        case 0: channel.save(filename); break;
        case 1: composite.save(filename); break;
        case 2: ndvi.save(filename); break;
        default: throw std::runtime_error("invalid tab index");
    }
}

void MainWindow::on_actionSave_All_Channels_triggered() {
    QString dirname = QFileDialog::getExistingDirectory(this, "Save All Channels", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dirname.isEmpty()) {
        QtConcurrent::run(this, &MainWindow::saveAllChannels, dirname);
    }

    // Reset channel
    compositor->getChannel(&channel, selectedChannel);
}
void MainWindow::saveAllChannels(QString dirname) {
    QImage temporaryChannel(compositor->width(), compositor->height(), QImage::Format_Grayscale16);

    for(unsigned int i = 0; i < compositor->channels(); i++) {
        compositor->getChannel(&temporaryChannel, i + 1);
        temporaryChannel.save(QString("%1/%2-%3.png").arg(dirname).arg(imagerName).arg(i + 1), "PNG");
    }
}
