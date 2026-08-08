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

class AppMenuButtonGroup;
class Decoration;

class AppMenuIconButton : public AppMenuButton
{
    Q_OBJECT
public:
    explicit AppMenuIconButton(const DecorationButtonType type, Decoration *decoration, const int buttonIndex, AppMenuButtonGroup *parent);
    ~AppMenuIconButton() override;

    void reconfigure() override;

    void setIconOffset(const QPointF &value)
    {
        m_iconOffset = value;
    }
    QPointF iconOffset() const
    {
        return m_iconOffset;
    }

    void setIconWidth(const qreal &value)
    {
        m_iconWidth = value;
    }
    qreal iconWidth() const
    {
        return m_iconWidth;
    }

    void setSmallButtonPaddedWidth(const qreal &value)
    {
        m_smallButtonPaddedWidth = value;
    }
    qreal smallButtonPaddedWidth() const
    {
        return m_smallButtonPaddedWidth;
    }

protected:
    void drawContent(QPainter *, QPointF) const override;

private:
    bool isSystemIconAvailable() const;
    bool shouldDrawBoldButtonIcons() const;
    void updateGeometry();

    QPointF m_iconOffset;
    qreal m_iconWidth;
    qreal m_smallButtonPaddedWidth;
    QString m_systemIconCheckedName;
    QString m_systemIconName;
};

} // namespace Breeze
