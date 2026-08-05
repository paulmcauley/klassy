/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2026 Guido Iodice <guido[dot]iodice[at]gmail[dot]com>
 *
 * This program is free software; you can redistribute it and/or modify
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

#include <KDecoration3/DecorationButton>

namespace Breeze
{

class Decoration;

class AppMenuButton : public KDecoration3::DecorationButton
{
    Q_OBJECT

public:
    AppMenuButton(Decoration *decoration, const int buttonIndex, QObject *parent = nullptr);
    ~AppMenuButton() override = default;

    Q_PROPERTY(int buttonIndex READ buttonIndex NOTIFY buttonIndexChanged)

    int buttonIndex() const;

    void paint(QPainter *painter, const QRectF &repaintRegion) override;

    void setOpacity(qreal value)
    {
        m_opacity = value;
    }
    qreal opacity() const
    {
        return m_opacity;
    }
    void forceUnpress()
    {
        setChecked(false);
    }
    virtual void setHeight(qreal buttonHeight);

signals:
    void buttonIndexChanged();

public Q_SLOTS:
    virtual void trigger();

protected:
    virtual void paintIcon(QPainter *painter, const QRectF &iconRect, const qreal iconScale) = 0;
    void setPenWidth(QPainter *painter, qreal width);
    qreal transitionValue() const;

private:
    int m_buttonIndex;
    qreal m_opacity = 1.0;
};

} // namespace Breeze
