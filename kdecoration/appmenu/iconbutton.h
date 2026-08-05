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
#include "breeze.h"

namespace Breeze
{

class Decoration;

class AppMenuIconButton : public AppMenuButton
{
    Q_OBJECT
public:
    explicit AppMenuIconButton(const DecorationButtonType type, Decoration *decoration, const int buttonIndex, QObject *parent = nullptr);
    ~AppMenuIconButton() override;

    void setHeight(qreal buttonHeight) override;
    void reconfigure() override;

    void setSmallButtonPaddedSize(const QSizeF &value)
    {
        m_smallButtonPaddedSize = value;
    }
    QSizeF smallButtonPaddedSize() const
    {
        return m_smallButtonPaddedSize;
    }

    void setIconSize(const QSizeF &value)
    {
        m_iconSize = value;
    }
    QSizeF iconSize() const
    {
        return m_iconSize;
    }

protected:
    void drawContent(QPainter *, QPointF) const override;

private:
    bool isSystemIconAvailable() const;
    bool shouldDrawBoldButtonIcons() const;

    QSizeF m_iconSize;
    QSizeF m_smallButtonPaddedSize;
    QString m_systemIconCheckedName;
    QString m_systemIconName;
};

} // namespace Breeze
