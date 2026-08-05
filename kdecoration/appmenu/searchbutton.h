/*
 * * Copyright (C) 2025 Guido Iodice <guido[dot]iodice[at]gmail[dot]com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "appmenu/button.h"

namespace Breeze
{

class Decoration;

class AppMenuSearchButton : public AppMenuButton
{
    Q_OBJECT
public:
    explicit AppMenuSearchButton(Decoration *decoration, const int buttonIndex, QObject *parent = nullptr);
    ~AppMenuSearchButton() override;

protected:
    void paintIcon(QPainter *painter, const QRectF &iconRect, const qreal) override;
};

} // namespace Breeze
