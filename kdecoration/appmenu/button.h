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

#include "breeze.h"
#include "decorationbuttoncolors.h"

#include <KDecoration3/DecorationButton>

#include <QVariantAnimation>

namespace Breeze
{

class Decoration;

class AppMenuButton : public KDecoration3::DecorationButton
{
    Q_OBJECT

public:
    AppMenuButton(DecorationButtonType type, Decoration *decoration, const int buttonIndex, QObject *parent = nullptr);
    ~AppMenuButton() override = default;

    Q_PROPERTY(int buttonIndex READ buttonIndex NOTIFY buttonIndexChanged)
    Q_PROPERTY(qreal transition READ transition WRITE setTransition NOTIFY transitionChanged)

    void paint(QPainter *painter, const QRectF &repaintRegion) override;

    void setOpacity(qreal value)
    {
        if (qFuzzyCompare(m_opacity, value))
            return;
        m_opacity = value;
        update();
    }
    qreal opacity() const
    {
        return m_opacity;
    }

    void forceUnpress()
    {
        setChecked(false);
    }

    void setTransition(qreal value)
    {
        if (qFuzzyCompare(m_transition, value))
            return;
        m_transition = value;
        transitionChanged();
        update();
    }
    qreal transition() const
    {
        return m_transition;
    }
    void transitionFully()
    {
        m_animation->setCurrentTime(m_animation->duration());
    }

    virtual void setHeight(qreal buttonHeight);

    void setIconOffset(const QPointF &value)
    {
        m_iconOffset = value;
    }
    QPointF iconOffset() const
    {
        return m_iconOffset;
    }

    void setBackgroundVisibleSize(const QSizeF &value)
    {
        m_backgroundVisibleSize = value;
    }
    QSizeF backgroundVisibleSize() const
    {
        return m_backgroundVisibleSize;
    }

    int buttonIndex() const
    {
        return m_buttonIndex;
    }

    virtual void reconfigure();

signals:
    void buttonIndexChanged();
    void transitionChanged();

protected:
    virtual void drawContent(QPainter *, QPointF) const = 0;
    QColor backgroundColor() const;
    QColor outlineColor() const;
    QColor foregroundColor() const;

public Q_SLOTS:
    virtual void trigger();

protected:
    Decoration *m_d;
    int m_buttonIndex;
    const DecorationButtonType m_type;
    qreal m_devicePixelRatio = 1;
    DecorationButtonPalette *m_buttonPalette = nullptr;

private:
    void setDevicePixelRatio(QPainter *painter);
    void updateAnimationState(bool hovered);

    qreal m_opacity = 0;
    qreal m_transition = 0;
    QPointF m_iconOffset;
    QSizeF m_backgroundVisibleSize;
    QVariantAnimation *m_animation;
};

} // namespace Breeze
