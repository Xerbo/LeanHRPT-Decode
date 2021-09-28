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

TLEManager::TLEManager() {
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkAccessManager::connect(manager, &QNetworkAccessManager::finished, [this](QNetworkReply *reply) {
        QStringList lines = QString(reply->readAll()).split("\r\n");

        for (size_t i = 0; i < lines.size()-2; i += 3) {
            std::string name  = lines[i  ].simplified().toStdString();
            std::string line1 = lines[i+1].toStdString();
            std::string line2 = lines[i+2].toStdString();

            catalog.insert({name, {line1, line2}});
        }
    });

    manager->get(QNetworkRequest(QUrl(TLE_URL)));
}

