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

#include "commandline.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("LeanHRPT Decode");
    QApplication::setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription("LeanHRPT Decode - A high quality, easy to use HRPT decoder.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);
    parser.addPositionalArgument("file", "filename");

    if (parser.positionalArguments().isEmpty()) {
        MainWindow window;
        window.show();
        return app.exec();
    } else {
        return parseCommandLine(parser);
    }
}
