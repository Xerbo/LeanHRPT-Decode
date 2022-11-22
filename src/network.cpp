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

#include "network.h"

#include <QDateTime>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "config/config.h"

TLEManager::TLEManager() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/weather.txt";
    quint64 time = QDateTime::currentSecsSinceEpoch();
    quint64 modified = QFileInfo(path).lastModified().toSecsSinceEpoch();

    if (time - modified > 24 * 60 * 60) {
        qInfo() << "TLEManager: Downloading fresh TLEs";
        QNetworkAccessManager *manager = new QNetworkAccessManager();
        QNetworkAccessManager::connect(manager, &QNetworkAccessManager::finished, [this, path](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError) {
                return;
            }

            QFile tle(path);
            tle.open(QIODevice::WriteOnly | QIODevice::Text);
            while (!reply->atEnd()) {
                tle.write(reply->read(1024));
            }
            tle.close();
            qInfo() << "TLEManager: Downloaded finished";

            parse(path.toStdString());
        });

        QNetworkRequest request;
        request.setUrl(QUrl(TLE_URL));
        request.setRawHeader("User-Agent", USER_AGENT);
        manager->get(request);
    } else {
        qInfo() << "TLEManager: Reusing existing TLEs";
        parse(path.toStdString());
    }
}

void TLEManager::parse(std::string filename) {
    QFile tle(QString::fromStdString(filename));
    tle.open(QIODevice::ReadOnly | QIODevice::Text);

    while (!tle.atEnd()) {
        std::string name = tle.readLine().simplified().toStdString();
        QString line1 = tle.readLine();
        QString line2 = tle.readLine();
        int norad = line2.split(" ")[1].toInt();

        catalog.insert({name, {line1.toStdString(), line2.toStdString()}});
        catalog_by_norad.insert({norad, {line1.toStdString(), line2.toStdString()}});
    }
}

UpdateChecker::UpdateChecker() {
    if (std::string(VERSION).find("-") != std::string::npos || std::string(VERSION) == "Unknown") return;

    qInfo() << "UpdateChecker: Checking for updates";
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkAccessManager::connect(manager, &QNetworkAccessManager::finished, [this](QNetworkReply *reply) {
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        QJsonArray tags = QJsonDocument::fromJson(reply->readAll()).array();
        QString latest_version = tags.at(0)["name"].toString();
        if (latest_version != VERSION) {
            QString url = QString("https://github.com/Xerbo/LeanHRPT-Decode/releases/tag/%1").arg(latest_version);
            qInfo() << "UpdateChecker: An update is available" << url;
            updateAvailable(url);
        }
    });

    QNetworkRequest request;
    request.setUrl(QUrl(TAG_URL));
    request.setRawHeader("User-Agent", USER_AGENT);
    manager->get(request);
}
