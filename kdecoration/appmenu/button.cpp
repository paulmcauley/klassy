/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2026 Guido Iodice <guido[dot]iodice[at]gmail[dot]com>
 * Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "appmenu/button.h"
#include "appmenu/buttongroup.h"
#include "breezedecoration.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/ScaleHelpers>
#include <KWindowSystem>

#include <QApplication>
#include <QPainter>

namespace Breeze
{

AppMenuButton::AppMenuButton(DecorationButtonType type, Decoration *decoration, const int buttonIndex, QObject *parent)
    : KDecoration3::DecorationButton(KDecoration3::DecorationButtonType::Custom, decoration, parent)
    , m_d(qobject_cast<Decoration *>(decoration))
    , m_buttonIndex(buttonIndex)
    , m_type(type)
    , m_animation(new QVariantAnimation(this))
{
    m_animation->setDuration(m_d->animationsDuration());
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setTransition(value.toReal());
    });
    setCheckable(true);

    connect(this, &AppMenuButton::clicked, this, &AppMenuButton::trigger);
    connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, &AppMenuButton::updateAnimationState);

    const auto *buttonGroup = qobject_cast<AppMenuButtonGroup *>(parent);
    if (buttonGroup) {
        setOpacity(buttonGroup->opacity());
    }
}

void AppMenuButton::updateAnimationState(bool hovered)
{
    if (!(m_d && m_d->animationsDuration() > 0)) {
        return;
    }

    if (!hovered && isChecked()) {
        return;
    }

    m_animation->setDirection(hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    if (m_animation->state() != QAbstractAnimation::Running) {
        m_animation->start();
    }
}

void AppMenuButton::reconfigure()
{
    m_animation->setDuration(m_d->animationsDuration());
}

void AppMenuButton::paint(QPainter *painter, const QRectF &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    if (!m_d) {
        return;
    }

    // Nothing to paint when fully transparent
    if (qFuzzyIsNull(m_opacity)) {
        painter->restore();
        return;
    }

    painter->save();
    QRectF backgroundBoundingRect = (QRectF(geometry().topLeft(), m_backgroundVisibleSize));
    backgroundBoundingRect = KDecoration3::snapToPixelGrid(backgroundBoundingRect, m_devicePixelRatio);
    painter->setClipRect(backgroundBoundingRect);
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setOpacity(sqrt(m_opacity * m_transition));
    painter->translate(geometry().topLeft());

    m_buttonPalette = m_d->decorationColors()->buttonPalette(m_type);

    QColor background = backgroundColor();
    QColor outline = outlineColor();

    if (background.isValid() || outline.isValid()) {
        setDevicePixelRatio(painter);

        const QRectF iconRect(QPointF(0, 0), geometry().size());

        if (outline.isValid()) {
            QPen pen(outline);
            pen.setWidthF(PenWidth::Symbol * m_devicePixelRatio);
            pen.setCosmetic(true);
            painter->setPen(pen);
        } else
            painter->setPen(Qt::NoPen);
        if (background.isValid())
            painter->setBrush(background);
        else
            painter->setBrush(Qt::NoBrush);

        qreal geometryShrinkOffset = PenWidth::Symbol * 1.5;
        painter->drawRect(iconRect.adjusted(geometryShrinkOffset, geometryShrinkOffset, -geometryShrinkOffset, -geometryShrinkOffset));
    }

    // Calculate device offset for icon drawing
    QPointF deviceOffsetDecorationTopLeftToIconTopLeft;
    QPointF topLeftButtonDeviceGeometry = painter->deviceTransform().map(geometry().topLeft());
    QPointF decorationTopLeftDeviceGeometry = painter->deviceTransform().map(QRectF(m_d->rect()).topLeft());
    deviceOffsetDecorationTopLeftToIconTopLeft = topLeftButtonDeviceGeometry - decorationTopLeftDeviceGeometry;
    painter->setOpacity(m_opacity);
    drawContent(painter, deviceOffsetDecorationTopLeftToIconTopLeft);

    painter->restore();
}

QColor AppMenuButton::backgroundColor() const
{
    auto c = m_d->window();
    const bool active = c->isActive();
    if (isChecked() || isPressed()) {
        return m_buttonPalette->backgroundPressActiveStateAnimated(active, m_animation);
    } else if (m_animation->state() == QAbstractAnimation::Running) {
        QColor backgroundNormal = m_buttonPalette->backgroundNormalActiveStateAnimated(active, m_animation);
        QColor backgroundHover = m_buttonPalette->backgroundHoverActiveStateAnimated(active, m_animation);
        if (backgroundNormal.isValid() && backgroundHover.isValid()) {
            return KColorUtils::mix(backgroundNormal, backgroundHover, m_opacity);
        } else if (backgroundHover.isValid()) {
            return ColorTools::alphaMix(backgroundHover, m_opacity);
        } else
            return QColor();
    } else if (isHovered()) {
        return m_buttonPalette->backgroundHoverActiveStateAnimated(active, m_animation);
    } else {
        return m_buttonPalette->backgroundNormalActiveStateAnimated(active, m_animation);
    }
}

QColor AppMenuButton::outlineColor() const
{
    auto c = m_d->window();
    const bool active = c->isActive();
    if (isChecked() || isPressed()) {
        return m_buttonPalette->outlinePressActiveStateAnimated(active, m_animation);
    } else if (m_animation->state() == QAbstractAnimation::Running) {
        QColor backgroundNormal = m_buttonPalette->outlineNormalActiveStateAnimated(active, m_animation);
        QColor backgroundHover = m_buttonPalette->outlineHoverActiveStateAnimated(active, m_animation);
        if (backgroundNormal.isValid() && backgroundHover.isValid()) {
            return KColorUtils::mix(backgroundNormal, backgroundHover, m_opacity);
        } else if (backgroundHover.isValid()) {
            return ColorTools::alphaMix(backgroundHover, m_opacity);
        } else
            return QColor();
    } else if (isHovered()) {
        return m_buttonPalette->outlineHoverActiveStateAnimated(active, m_animation);
    } else {
        return m_buttonPalette->outlineNormalActiveStateAnimated(active, m_animation);
    }
}

QColor AppMenuButton::foregroundColor() const
{
    auto c = m_d->window();
    const bool active = c->isActive();
    if (isChecked() || isPressed()) {
        return m_buttonPalette->foregroundPressActiveStateAnimated(active, m_animation);
    } else if (m_animation->state() == QAbstractAnimation::Running) {
        QColor backgroundNormal = m_buttonPalette->foregroundNormalActiveStateAnimated(active, m_animation);
        QColor backgroundHover = m_buttonPalette->foregroundHoverActiveStateAnimated(active, m_animation);
        if (backgroundNormal.isValid() && backgroundHover.isValid()) {
            return KColorUtils::mix(backgroundNormal, backgroundHover, m_opacity);
        } else if (backgroundHover.isValid()) {
            return ColorTools::alphaMix(backgroundHover, m_opacity);
        } else
            return QColor();
    } else if (isHovered()) {
        return m_buttonPalette->foregroundHoverActiveStateAnimated(active, m_animation);
    } else {
        return m_buttonPalette->foregroundNormalActiveStateAnimated(active, m_animation);
    }
}

void AppMenuButton::setDevicePixelRatio(QPainter *painter)
{
    // on X11 Kwin just returns 1.0 for the DPR instead of the correct value, so use the scaling setting directly
    if (KWindowSystem::isPlatformX11())
        m_devicePixelRatio = m_d->systemScaleFactorX11();
    else
        m_devicePixelRatio = painter->device()->devicePixelRatioF();
}

void AppMenuButton::setHeight(qreal buttonHeight)
{
    const QSizeF size(buttonHeight * 1.1, buttonHeight);
    setGeometry(QRectF(geometry().topLeft(), size));
}

void AppMenuButton::trigger()
{
    auto *buttonGroup = qobject_cast<AppMenuButtonGroup *>(parent());
    if (buttonGroup) {
        buttonGroup->trigger(m_buttonIndex, false);
    }
}

} // namespace Breeze
