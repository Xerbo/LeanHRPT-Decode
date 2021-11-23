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

#include "tle.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QFileInfo>
#include <QDateTime>

TLEManager::TLEManager() {
    quint64 time = QDateTime::currentSecsSinceEpoch();
    quint64 modified = QFileInfo("weather.txt").lastModified().toSecsSinceEpoch();

    if (time - modified > 24*60*60) {
        QNetworkAccessManager *manager = new QNetworkAccessManager();
        QNetworkAccessManager::connect(manager, &QNetworkAccessManager::finished, [this](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError) {
                return;
            }

            QFile tle("weather.txt");
            tle.open(QIODevice::WriteOnly | QIODevice::Text);
            while (!reply->atEnd()) {
                tle.write(reply->read(1024));
            }
            tle.close();

            parse("weather.txt");
        });

        manager->get(QNetworkRequest(QUrl(TLE_URL)));
    } else {
        parse("weather.txt");
    }
}

void TLEManager::parse(std::string filename) {
    QFile tle(QString::fromStdString(filename));
    tle.open(QIODevice::ReadOnly | QIODevice::Text);

    while (!tle.atEnd()) {
        std::string name  = tle.readLine().simplified().toStdString();
        std::string line1 = tle.readLine().toStdString();
        std::string line2 = tle.readLine().toStdString();

        catalog.insert({name, {line1, line2}});
    }
}
