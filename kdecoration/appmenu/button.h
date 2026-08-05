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

    Q_PROPERTY(int buttonIndex READ buttonIndex)
    Q_PROPERTY(qreal buttonHeight READ buttonHeight WRITE setButtonHeight NOTIFY buttonHeightChanged)
    Q_PROPERTY(qreal transition READ transition WRITE setTransition NOTIFY transitionChanged)
    Q_PROPERTY(qreal verticalContentOffset READ verticalContentOffset WRITE setVerticalContentOffset NOTIFY verticalContentOffsetChanged)

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

    void setBackgroundVisibleSize(const QSizeF &value)
    {
        m_backgroundVisibleSize = value;
    }
    QSizeF backgroundVisibleSize() const
    {
        return m_backgroundVisibleSize;
    }

    void setVerticalContentOffset(qreal value)
    {
        if (qFuzzyCompare(m_verticalContentOffset, value))
            return;
        m_verticalContentOffset = value;
        verticalContentOffsetChanged();
        update();
    }
    qreal verticalContentOffset() const
    {
        return m_verticalContentOffset;
    }

    void setButtonHeight(qreal value)
    {
        if (qFuzzyCompare(m_buttonHeight, value))
            return;
        m_buttonHeight = value;
        buttonHeightChanged();
        update();
    }
    qreal buttonHeight() const
    {
        return m_buttonHeight;
    }

    void setLeftmostVisible(bool value)
    {
        m_leftmostVisible = value;
    }
    void setRightmostVisible(bool value)
    {
        m_rightmostVisible = value;
    }

    int buttonIndex() const
    {
        return m_buttonIndex;
    }

    virtual void reconfigure();

    static inline bool isShapeFullHeight(int shape)
    {
        switch (shape) {
        case InternalSettings::EnumIntegratedMenuButtonShape::FullHeightRectangle:
        case InternalSettings::EnumIntegratedMenuButtonShape::FullHeightRoundedRectangle:
        case InternalSettings::EnumIntegratedMenuButtonShape::FullHeightRoundedRectangleGrouped:
        case InternalSettings::EnumIntegratedMenuButtonShape::IntegratedRoundedRectangle:
        case InternalSettings::EnumIntegratedMenuButtonShape::IntegratedRoundedRectangleGrouped:
            return true;
        default:
            return false;
        }
    }

signals:
    void buttonHeightChanged();
    void transitionChanged();
    void verticalContentOffsetChanged();

public Q_SLOTS:
    virtual void trigger();

protected:
    virtual void drawContent(QPainter *, QPointF offsetDecorationTopLeftToIconTopLeft) const = 0;
    QColor backgroundColor() const;
    QColor outlineColor() const;
    QColor foregroundColor() const;

    Decoration *m_d;
    const int m_buttonIndex;
    const DecorationButtonType m_type;
    qreal m_devicePixelRatio = 1;
    DecorationButtonPalette *m_buttonPalette = nullptr;

private:
    void setDevicePixelRatio(QPainter *painter);
    void updateAnimationState(bool hovered);

    qreal m_buttonHeight = 0;
    qreal m_opacity = 0;
    qreal m_transition = 0;
    qreal m_verticalContentOffset = 0;
    QSizeF m_backgroundVisibleSize;
    QVariantAnimation *m_animation;
    bool m_leftmostVisible = false;
    bool m_rightmostVisible = false;
};

} // namespace Breeze
