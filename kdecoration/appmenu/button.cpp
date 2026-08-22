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
#include "breeze.h"
#include "breezedecoration.h"
#include "geometrytools.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/ScaleHelpers>
#include <KWindowSystem>

#include <QApplication>
#include <QPainter>

namespace Breeze
{

AppMenuButton::AppMenuButton(DecorationButtonType type, Decoration *decoration, const int buttonIndex, AppMenuButtonGroup *parent)
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
    connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, [this](bool v) {
        updateAnimationState(v || qobject_cast<AppMenuButtonGroup *>(this->parent())->unisonHovered());
    });
    connect(parent, &AppMenuButtonGroup::unisonHoveredChanged, this, [this](bool v) {
        updateAnimationState(v || this->isHovered());
    });

    const auto *buttonGroup = qobject_cast<AppMenuButtonGroup *>(parent);
    if (buttonGroup) {
        setOpacity(buttonGroup->opacity());
    }
}

bool AppMenuButton::hovered() const // for integrated menu unison hovering
{
    return isHovered() || qobject_cast<AppMenuButtonGroup *>(parent())->unisonHovered();
}

void AppMenuButton::updateAnimationState(bool hovered)
{
    if (!(m_d && m_d->animationsDuration() > 0)) {
        setTransition(hovered ? 1 : 0);
        return;
    }

    if (!hovered && isChecked()) {
        return;
    }

    m_animation->setDirection(hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    const bool atTheEnd = hovered && qFuzzyCompare(m_animation->currentValue().toReal(), m_animation->endValue().toReal());
    if (m_animation->state() != QAbstractAnimation::Running && !atTheEnd) {
        m_animation->start();
    }
}

void AppMenuButton::reconfigure()
{
    m_animation->setDuration(m_d->animationsDuration());

    const auto internalSettings = m_d->internalSettings();
    switch (internalSettings->integratedMenuButtonCornerRadius()) {
    case InternalSettings::EnumIntegratedMenuButtonCornerRadius::IMCR_DerivedFromApplicationStyle:
        m_cornerRadius =
            internalSettings->frameCornerRadius() ? internalSettings->frameCustomCornerRadius() : qMin(5.0, internalSettings->windowCornerRadius());
        break;
    case InternalSettings::EnumIntegratedMenuButtonCornerRadius::IMCR_DerivedFromWindowButton:
        m_cornerRadius = internalSettings->buttonCornerRadius() ? internalSettings->buttonCustomCornerRadius() : internalSettings->windowCornerRadius();
        break;
    default:
        m_cornerRadius = internalSettings->integratedMenuButtonCustomCornerRadius();
        break;
    }
    m_cornerRadius *= m_d->x11Scale();
    if (m_cornerRadius < 0.1) {
        m_cornerRadius = 0;
    }
}

void AppMenuButton::paint(QPainter *painter, const QRectF &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    if (!m_d) {
        return;
    }

    // Nothing to paint when fully transparent
    if (qFuzzyIsNull(m_opacity) || qFuzzyIsNull(m_expansionOpacity)) {
        return;
    }

    setDevicePixelRatio(painter);

    painter->save();
    QRectF backgroundBoundingRect = (QRectF(geometry().topLeft() + QPointF(0, m_verticalBackgroundOffset), m_backgroundVisibleSize));
    backgroundBoundingRect = KDecoration3::snapToPixelGrid(backgroundBoundingRect, m_devicePixelRatio);
    painter->setClipRect(backgroundBoundingRect);
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setOpacity(m_opacity * m_expansionOpacity);
    painter->translate(geometry().topLeft() + QPointF(0, m_verticalBackgroundOffset));

    m_buttonPalette = m_d->decorationColors()->buttonPalette(m_type);

    QColor background = backgroundColor();
    QColor outline = outlineColor();

    if (background.isValid() || outline.isValid()) {
        const QRectF contentRect(QPointF(geometry().topLeft() + QPointF(geometry().width() - m_backgroundVisibleSize.width(), m_verticalContentOffset)),
                                 geometry().size() - QSizeF(0, m_verticalBackgroundOffset * 2 + m_verticalContentOffset));

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
        if (KWindowSystem::isPlatformX11())
            geometryShrinkOffset *= m_devicePixelRatio;

        const auto buttonShape = m_d->internalSettings()->integratedMenuButtonShape();
        QRectF backgroundRect = contentRect;
        if (isShapeFullHeight(buttonShape)) {
            backgroundRect.adjust(0, -m_verticalContentOffset, 0, 0);
        }
        backgroundRect = KDecoration3::snapToPixelGrid(backgroundRect, m_devicePixelRatio);
        if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::IntegratedRoundedRectangle
            || buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::IntegratedRoundedRectangleGrouped) {
            backgroundRect.adjust(geometryShrinkOffset, -geometryShrinkOffset, -geometryShrinkOffset, -geometryShrinkOffset);
        } else if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::Tab) {
            backgroundRect.adjust(geometryShrinkOffset, geometryShrinkOffset, -geometryShrinkOffset, geometryShrinkOffset);
        } else {
            backgroundRect.adjust(geometryShrinkOffset, geometryShrinkOffset, -geometryShrinkOffset, -geometryShrinkOffset);
        }
        backgroundRect = backgroundRect.translated(-geometry().topLeft());

        if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::Rectangle) {
            painter->drawRect(backgroundRect);
        } else {
            Corners corners = Corners();
            if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::RoundedRectangle
                || buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::FullHeightRoundedRectangle) {
                corners = AllCorners;
            } else if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::IntegratedRoundedRectangle) {
                corners = CornerBottomLeft | CornerBottomRight;
            } else if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::RoundedRectangleGrouped
                       || buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::FullHeightRoundedRectangleGrouped) {
                if (m_leftmostVisible) {
                    corners |= CornerTopLeft | CornerBottomLeft;
                }
                if (m_rightmostVisible) {
                    corners |= CornerTopRight | CornerBottomRight;
                }
            } else if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::IntegratedRoundedRectangleGrouped) {
                if (m_leftmostVisible) {
                    corners |= CornerBottomLeft;
                }
                if (m_rightmostVisible) {
                    corners |= CornerBottomRight;
                }
            } else if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::Tab) {
                corners |= CornerTopLeft | CornerTopRight;
            }

            QPainterPath background;
            if (!corners) {
                background.addRect(backgroundRect);
            } else {
                background = GeometryTools::roundedPath(backgroundRect, corners, m_cornerRadius);
            }
            painter->drawPath(background);
        }
    }

    const QPointF offsetDecorationTopLeftToIconTopLeft = geometry().topLeft() + QPointF(0, m_verticalBackgroundOffset - m_verticalContentOffset);
    drawContent(painter, offsetDecorationTopLeftToIconTopLeft);

    painter->restore();
}

QColor AppMenuButton::backgroundColor() const
{
    auto c = m_d->window();
    const bool active = c->isActive();
    const QVariantAnimation *animation = m_d->activeStateChangeAnimation();
    if (isChecked() || isPressed()) {
        return m_buttonPalette->backgroundPressActiveStateAnimated(active, animation);
    } else if (m_animation->state() == QAbstractAnimation::Running) {
        QColor backgroundNormal = m_buttonPalette->backgroundNormalActiveStateAnimated(active, animation);
        QColor backgroundHover = m_buttonPalette->backgroundHoverActiveStateAnimated(active, animation);
        if (backgroundNormal.isValid() && backgroundHover.isValid()) {
            return KColorUtils::mix(backgroundNormal, backgroundHover, m_transition);
        } else if (backgroundHover.isValid()) {
            return ColorTools::alphaMix(backgroundHover, m_transition);
        } else
            return QColor();
    } else if (hovered()) {
        return m_buttonPalette->backgroundHoverActiveStateAnimated(active, animation);
    } else {
        return m_buttonPalette->backgroundNormalActiveStateAnimated(active, animation);
    }
}

QColor AppMenuButton::outlineColor() const
{
    auto c = m_d->window();
    const bool active = c->isActive();
    const QVariantAnimation *animation = m_d->activeStateChangeAnimation();
    if (isChecked() || isPressed()) {
        return m_buttonPalette->outlinePressActiveStateAnimated(active, animation);
    } else if (m_animation->state() == QAbstractAnimation::Running) {
        QColor backgroundNormal = m_buttonPalette->outlineNormalActiveStateAnimated(active, animation);
        QColor backgroundHover = m_buttonPalette->outlineHoverActiveStateAnimated(active, animation);
        if (backgroundNormal.isValid() && backgroundHover.isValid()) {
            return KColorUtils::mix(backgroundNormal, backgroundHover, m_transition);
        } else if (backgroundHover.isValid()) {
            return ColorTools::alphaMix(backgroundHover, m_transition);
        } else
            return QColor();
    } else if (hovered()) {
        return m_buttonPalette->outlineHoverActiveStateAnimated(active, animation);
    } else {
        return m_buttonPalette->outlineNormalActiveStateAnimated(active, animation);
    }
}

QColor AppMenuButton::foregroundColor() const
{
    auto c = m_d->window();
    const bool active = c->isActive();
    const QVariantAnimation *animation = m_d->activeStateChangeAnimation();
    if (isChecked() || isPressed()) {
        return m_buttonPalette->foregroundPressActiveStateAnimated(active, animation);
    } else if (m_animation->state() == QAbstractAnimation::Running) {
        QColor backgroundNormal = m_buttonPalette->foregroundNormalActiveStateAnimated(active, animation);
        QColor backgroundHover = m_buttonPalette->foregroundHoverActiveStateAnimated(active, animation);
        if (backgroundNormal.isValid() && backgroundHover.isValid()) {
            return KColorUtils::mix(backgroundNormal, backgroundHover, m_transition);
        } else if (backgroundHover.isValid()) {
            return ColorTools::alphaMix(backgroundHover, m_transition);
        } else
            return QColor();
    } else if (hovered()) {
        return m_buttonPalette->foregroundHoverActiveStateAnimated(active, animation);
    } else {
        return m_buttonPalette->foregroundNormalActiveStateAnimated(active, animation);
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

void AppMenuButton::trigger()
{
    auto *buttonGroup = qobject_cast<AppMenuButtonGroup *>(parent());
    if (buttonGroup) {
        buttonGroup->trigger(m_buttonIndex, false);
    }
}

void AppMenuButton::forceUnpress()
{
    // HACK: There is no public API to set the pressed state of a
    // KDecoration3::DecorationButton. This works around the issue by toggling
    // the enabled state, which has the side effect of resetting the
    // button's internal pressed state.
    const bool wasEnabled = isEnabled();
    setEnabled(!wasEnabled);
    setEnabled(wasEnabled);
}

} // namespace Breeze
