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

#ifndef LEANHRPT_QT_QPALETTEVIEW
#define LEANHRPT_QT_QPALETTEVIEW

#include <QPainter>
#include <QWidget>

#include "util.h"

class QPaletteView : public QWidget {
   public:
    QPaletteView([[maybe_unused]] QWidget* parent) {}
    std::vector<QColor> stops;

   private:
    virtual void paintEvent([[maybe_unused]] QPaintEvent* p) override {
        QPainter painter(this);
        painter.fillRect(0, 0, width(), height(), QColor(0, 0, 0));
        if (stops.size() == 0) return;

        for (size_t x = 0; x < (size_t)width(); x++) {
            double i = (double)x / (double)(width() - 1) * (stops.size() - 1);
            QColor color = lerp(stops[floor(i)], stops[ceil(i)], fmod(i, 1.0));

            painter.setPen(color);
            painter.drawLine(x, 0, x, height());
        }
    }
};

#endif
