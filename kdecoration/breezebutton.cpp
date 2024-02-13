/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezebutton.h"
#include "colortools.h"
#include "geometrytools.h"
#include "renderdecorationbuttonicon.h"
#include "systemicontheme.h"

#include <KColorScheme>
#include <KColorUtils>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KIconLoader>
#include <KWindowSystem>

#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

namespace Breeze
{

using KDecoration2::ColorGroup;
using KDecoration2::ColorRole;
using KDecoration2::DecorationButtonType;

//__________________________________________________________________
Button::Button(DecorationButtonType type, Decoration *decoration, QObject *parent)
    : DecorationButton(type, decoration, parent)
    , m_animation(new QVariantAnimation(this))
    , m_isGtkCsdButton(false)
{
    auto c = decoration->client().toStrongRef();
    Q_ASSERT(c);

    // setup animation
    // It is important start and end value are of the same type, hence 0.0 and not just 0
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setOpacity(value.toReal());
    });

    // detect the kde-gtk-config-daemon
    // kde-gtk-config has a kded5 module which renders the buttons to svgs for gtk
    if (QCoreApplication::applicationName() == QStringLiteral("kded5")) {
        m_isGtkCsdButton = true;
    }

    // setup default geometry
    int smallButtonPaddedHeight = decoration->smallButtonPaddedHeight();
    int iconHeight = decoration->iconHeight();
    int smallButtonBackgroundHeight = decoration->smallButtonBackgroundHeight();

    setGeometry(QRect(0, 0, smallButtonPaddedHeight, smallButtonPaddedHeight));
    setSmallButtonPaddedSize(QSize(smallButtonPaddedHeight, smallButtonPaddedHeight));
    setIconSize(QSize(iconHeight, iconHeight));
    setBackgroundVisibleSize((QSizeF(smallButtonBackgroundHeight, smallButtonBackgroundHeight)));

    // connections
    connect(c.data(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
    connect(decoration, &Decoration::reconfigured, this, &Button::reconfigure);
    connect(this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState);
    connect(this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateThinWindowOutlineWithButtonColor);
    connect(this, &KDecoration2::DecorationButton::pressedChanged, this, &Button::updateThinWindowOutlineWithButtonColor);

    reconfigure();
}

//__________________________________________________________________
Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration *>(), parent)
{
    m_flag = FlagStandalone;
    //! small button size must return to !valid because it was altered from the default constructor,
    //! in Standalone mode the button is not using the decoration metrics but its geometry
    m_smallButtonPaddedSize = QSize(-1, -1);
}

//__________________________________________________________________
Button *Button::create(DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    if (auto d = qobject_cast<Decoration *>(decoration)) {
        auto c = d->client().toStrongRef();
        Q_ASSERT(c);

        Button *b = new Button(type, d, parent);
        switch (type) {
        case DecorationButtonType::Close:
            b->setVisible(c->isCloseable());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::closeableChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Maximize:
            b->setVisible(c->isMaximizeable());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Minimize:
            b->setVisible(c->isMinimizeable());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::minimizeableChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::ContextHelp:
            b->setVisible(c->providesContextHelp());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::providesContextHelpChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Shade:
            b->setVisible(c->isShadeable());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::shadeableChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Menu:
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() {
                b->update();
            });
            break;

        default:
            break;
        }

        return b;
    }

    return nullptr;
}

//__________________________________________________________________
void Button::paint(QPainter *painter, const QRect &repaintRegion)
{
    if (!geometry().intersects(repaintRegion)) {
        return;
    }

    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return;
    }
    auto c = d->client().toStrongRef();
    Q_ASSERT(c);

    m_buttonPalette = d->decorationColors()->buttonPalette(type()); // this is in paint() in-case caching type on m_buttonPalette changes
    m_titlebarTextPinnedInversion = titlebarTextPinnedInversion();

    setDevicePixelRatio(painter);
    setShouldDrawBoldButtonIcons();
    m_renderSystemIcon = d->internalSettings()->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme && isSystemIconAvailable();
    setStandardScaledPenWidth();

    m_backgroundColor = this->backgroundColor();
    m_foregroundColor = this->foregroundColor();
    m_outlineColor = this->outlineColor();

    if (!m_smallButtonPaddedSize.isValid() || isStandAlone()) {
        m_smallButtonPaddedSize = geometry().size().toSize();
        int iconWidth = qRound(qreal(m_smallButtonPaddedSize.width()) * 0.9);
        setIconSize(QSize(iconWidth, iconWidth));
        setBackgroundVisibleSize(QSizeF(iconWidth, iconWidth));
    }

    painter->save();

    // menu button (with application icon)
    if (type() == DecorationButtonType::Menu) {
        // draw a background only with Full-sized background shapes;
        // for standalone/GTK we draw small buttons so can't draw menu
        if (d->buttonBackgroundType() == ButtonBackgroundType::FullHeight && !(isStandAlone() || m_isGtkCsdButton))
            paintFullHeightButtonBackground(painter);

        // translate from icon offset -- translates to the edge of smallButtonPaddedSize
        painter->translate(m_iconOffset);

        // translate to draw icon in the centre of smallButtonPaddedWidth (smallButtonPaddedWidth has additional padding)
        qreal iconTranslationOffset = (m_smallButtonPaddedSize.width() - m_iconSize.width()) / 2;
        painter->translate(iconTranslationOffset, iconTranslationOffset);

        const QRectF iconRect(geometry().topLeft(), m_iconSize);

        const QPalette originalPalette = KIconLoader::global()->customPalette();
        QPalette palette = c->palette();
        palette.setColor(QPalette::WindowText, m_foregroundColor);
        KIconLoader::global()->setCustomPalette(palette);
        c->icon().paint(painter, iconRect.toRect());
        if (originalPalette == QPalette()) {
            KIconLoader::global()->resetPalette();
        } else {
            KIconLoader::global()->setCustomPalette(originalPalette);
        }

    } else {
        drawIcon(painter);
    }

    painter->restore();
}

//__________________________________________________________________
void Button::drawIcon(QPainter *painter) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;

    painter->setRenderHints(QPainter::Antialiasing);

    // for standalone/GTK we draw small buttons so don't do anything
    if (!(isStandAlone() || m_isGtkCsdButton)) {
        // draw a background only with Full-sized Rectangle button shape;
        // NB: paintFullHeightRectangleBackground function applies a translation to painter as different full-sized button geometry
        if (d->buttonBackgroundType() == ButtonBackgroundType::FullHeight)
            paintFullHeightButtonBackground(painter);
    }

    QPointF deviceOffsetDecorationTopLeftToIconTopLeft;
    QPointF topLeftPaddedButtonDeviceGeometry = painter->deviceTransform().map(geometry().topLeft());

    // get top-left geometry relative to the decoration top-left as is is what kwin snaps to a whole pixel since Plasma 5.27
    //(on button hover sometimes the painter gives geometry relative to the button rather than to titlebar, so this is also why this is necessary)
    QPointF decorationTopLeftDeviceGeometry = painter->deviceTransform().map(QRectF(d->rect()).topLeft());
    deviceOffsetDecorationTopLeftToIconTopLeft = topLeftPaddedButtonDeviceGeometry - decorationTopLeftDeviceGeometry;

    painter->translate(geometry().topLeft());

    // translate from icon offset -- translates to the edge of smallButtonPaddedWidth
    painter->translate(m_iconOffset);
    deviceOffsetDecorationTopLeftToIconTopLeft += (m_iconOffset * painter->device()->devicePixelRatioF());

    const qreal smallButtonPaddedWidth(m_smallButtonPaddedSize.width());

    if (m_isGtkCsdButton) {
        if (smallButtonPaddedWidth > 20) { // outlines appear thin so scale them proportionally
            m_standardScaledNonCosmeticPenWidth = PenWidth::Symbol * smallButtonPaddedWidth / 20;
        } else {
            m_standardScaledNonCosmeticPenWidth = PenWidth::Symbol * qMax((qreal)1.0, 20 / smallButtonPaddedWidth);
        }
    }

    if (d->buttonBackgroundType() == ButtonBackgroundType::Small || isStandAlone() || m_isGtkCsdButton)
        paintSmallSizedButtonBackground(painter);

    if (!m_foregroundColor.isValid())
        return;

    // render the actual icon
    qreal iconWidth(m_iconSize.width());

    // translate to draw icon in the centre of smallButtonPaddedWidth (smallButtonPaddedWidth has additional padding)
    qreal iconTranslationOffset = (smallButtonPaddedWidth - iconWidth) / 2;
    painter->translate(iconTranslationOffset, iconTranslationOffset);
    deviceOffsetDecorationTopLeftToIconTopLeft += (QPointF(iconTranslationOffset, iconTranslationOffset) * painter->device()->devicePixelRatioF());

    // setup painter
    QPen pen(m_foregroundColor);

    // cannot use a scaled cosmetic pen if GTK CSD as kde-gtk-config generates svg icons.
    if (m_isGtkCsdButton) {
        pen.setWidthF(PenWidth::Symbol * qMax((qreal)1.0, 18 / iconWidth));
    } else {
        pen.setWidthF(m_standardScaledCosmeticPenWidth);
        pen.setCosmetic(true);
    }
    painter->setPen(pen);

    if (m_renderSystemIcon) {
        auto c = d->client().toStrongRef();
        Q_ASSERT(c);
        QString systemIconName;
        systemIconName = isChecked() ? m_systemIconCheckedName : m_systemIconName;
        SystemIconTheme iconRenderer(painter, iconWidth, systemIconName, d->internalSettings(), c->palette());
        iconRenderer.renderIcon();
    } else {
        auto [iconRenderer, localRenderingWidth] = RenderDecorationButtonIcon::factory(d->internalSettings(),
                                                                                       painter,
                                                                                       false,
                                                                                       m_boldButtonIcons,
                                                                                       m_devicePixelRatio,
                                                                                       deviceOffsetDecorationTopLeftToIconTopLeft);

        qreal scaleFactor = iconWidth / localRenderingWidth;
        /*
        scale painter so that all further rendering is preformed inside QRect( 0, 0, localRenderingWidth, localRenderingWidth )
        TODO: localRenderingWidth currently assumed to be 18. If add 16x16 rendering support most likely need an additional translate to centre the icon with an
        additional deviceOffsetDecorationTopLeftToIconTopLeft adjustment
        */
        painter->scale(scaleFactor, scaleFactor);

        iconRenderer->renderIcon(type(), isChecked());
    }
}

//__________________________________________________________________
QColor Button::foregroundColor() const
{
    auto d = qobject_cast<Decoration *>(decoration());

    if (!d)
        return QColor();

    auto c = d->client().toStrongRef();
    Q_ASSERT(c);
    const bool active = c->isActive();
    DecorationButtonPaletteGroup *group = active ? m_buttonPalette->active() : m_buttonPalette->inactive();
    QColor foregroundPress = group->foregroundPress;
    QColor foregroundHover = group->foregroundHover;
    QColor foregroundNormal = group->foregroundNormal;

    // active change state animation
    if (d->activeStateChangeAnimation()->state() == QAbstractAnimation::Running) {
        if (m_buttonPalette->active()->foregroundPress.isValid() && m_buttonPalette->inactive()->foregroundPress.isValid()) {
            foregroundPress = KColorUtils::mix(m_buttonPalette->inactive()->foregroundPress,
                                               m_buttonPalette->active()->foregroundPress,
                                               d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->foregroundPress.isValid() && !m_buttonPalette->inactive()->foregroundPress.isValid()) {
            foregroundPress = ColorTools::alphaMix(m_buttonPalette->active()->foregroundPress, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->foregroundPress.isValid() && m_buttonPalette->inactive()->foregroundPress.isValid()) {
            foregroundPress = ColorTools::alphaMix(m_buttonPalette->inactive()->foregroundPress, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette->active()->foregroundHover.isValid() && m_buttonPalette->inactive()->foregroundHover.isValid()) {
            foregroundHover = KColorUtils::mix(m_buttonPalette->inactive()->foregroundHover,
                                               m_buttonPalette->active()->foregroundHover,
                                               d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->foregroundHover.isValid() && !m_buttonPalette->inactive()->foregroundHover.isValid()) {
            foregroundHover = ColorTools::alphaMix(m_buttonPalette->active()->foregroundHover, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->foregroundHover.isValid() && m_buttonPalette->inactive()->foregroundHover.isValid()) {
            foregroundHover = ColorTools::alphaMix(m_buttonPalette->inactive()->foregroundHover, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette->active()->foregroundNormal.isValid() && m_buttonPalette->inactive()->foregroundNormal.isValid()) {
            foregroundNormal = KColorUtils::mix(m_buttonPalette->inactive()->foregroundNormal,
                                                m_buttonPalette->active()->foregroundNormal,
                                                d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->foregroundNormal.isValid() && !m_buttonPalette->inactive()->foregroundNormal.isValid()) {
            foregroundNormal = ColorTools::alphaMix(m_buttonPalette->active()->foregroundNormal, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->foregroundNormal.isValid() && m_buttonPalette->inactive()->foregroundNormal.isValid()) {
            foregroundNormal = ColorTools::alphaMix(m_buttonPalette->inactive()->foregroundNormal, (1.0 - d->activeStateChangeAnimationOpacity()));
        }
    }

    // return a variant of normal, hover and press colours, depending on state
    if (isPressed()) {
        return foregroundPress;
    } else if (isChecked()
               && (type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade
                   || (type() == DecorationButtonType::OnAllDesktops && !m_titlebarTextPinnedInversion))) {
        if (d->internalSettings()->buttonStateChecked(active) == InternalSettings::EnumButtonStateChecked::Hover) {
            return foregroundHover;
        } else {
            return foregroundPress;
        }
    } else if (m_animation->state() == QAbstractAnimation::Running) { // button hover animation
        if (foregroundNormal.isValid() && foregroundHover.isValid()) {
            return KColorUtils::mix(foregroundNormal, foregroundHover, m_opacity);
        } else if (foregroundHover.isValid()) {
            return ColorTools::alphaMix(foregroundHover, m_opacity);
        } else
            return QColor();
    } else if (isHovered()) {
        return foregroundHover;
    } else {
        return foregroundNormal;
    }
}

//__________________________________________________________________
QColor Button::backgroundColor(const bool getNonAnimatedColor) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return QColor();
    }

    auto c = d->client().toStrongRef();
    Q_ASSERT(c);
    const bool active = c->isActive();
    DecorationButtonPaletteGroup *group = active ? m_buttonPalette->active() : m_buttonPalette->inactive();
    QColor backgroundPress = group->backgroundPress;
    QColor backgroundHover = group->backgroundHover;
    QColor backgroundNormal = group->backgroundNormal;

    // active change state animation
    if (d->activeStateChangeAnimation()->state() == QAbstractAnimation::Running && !getNonAnimatedColor) {
        if (m_buttonPalette->active()->backgroundPress.isValid() && m_buttonPalette->inactive()->backgroundPress.isValid()) {
            backgroundPress = KColorUtils::mix(m_buttonPalette->inactive()->backgroundPress,
                                               m_buttonPalette->active()->backgroundPress,
                                               d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->backgroundPress.isValid() && !m_buttonPalette->inactive()->backgroundPress.isValid()) {
            backgroundPress = ColorTools::alphaMix(m_buttonPalette->active()->backgroundPress, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->backgroundPress.isValid() && m_buttonPalette->inactive()->backgroundPress.isValid()) {
            backgroundPress = ColorTools::alphaMix(m_buttonPalette->inactive()->backgroundPress, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette->active()->backgroundHover.isValid() && m_buttonPalette->inactive()->backgroundHover.isValid()) {
            backgroundHover = KColorUtils::mix(m_buttonPalette->inactive()->backgroundHover,
                                               m_buttonPalette->active()->backgroundHover,
                                               d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->backgroundHover.isValid() && !m_buttonPalette->inactive()->backgroundHover.isValid()) {
            backgroundHover = ColorTools::alphaMix(m_buttonPalette->active()->backgroundHover, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->backgroundHover.isValid() && m_buttonPalette->inactive()->backgroundHover.isValid()) {
            backgroundHover = ColorTools::alphaMix(m_buttonPalette->inactive()->backgroundHover, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette->active()->backgroundNormal.isValid() && m_buttonPalette->inactive()->backgroundNormal.isValid()) {
            backgroundNormal = KColorUtils::mix(m_buttonPalette->inactive()->backgroundNormal,
                                                m_buttonPalette->active()->backgroundNormal,
                                                d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->backgroundNormal.isValid() && !m_buttonPalette->inactive()->backgroundNormal.isValid()) {
            backgroundNormal = ColorTools::alphaMix(m_buttonPalette->active()->backgroundNormal, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->backgroundNormal.isValid() && m_buttonPalette->inactive()->backgroundNormal.isValid()) {
            backgroundNormal = ColorTools::alphaMix(m_buttonPalette->inactive()->backgroundNormal, (1.0 - d->activeStateChangeAnimationOpacity()));
        }
    }

    // return a variant of normal, hover and press colours, depending on state
    if (isPressed()) {
        return backgroundPress;
    } else if (isChecked()
               && (type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade
                   || (type() == DecorationButtonType::OnAllDesktops && !m_titlebarTextPinnedInversion))) {
        if (d->internalSettings()->buttonStateChecked(active) == InternalSettings::EnumButtonStateChecked::Hover) {
            return backgroundHover;
        } else {
            return backgroundPress;
        }
    } else if (m_animation->state() == QAbstractAnimation::Running && !getNonAnimatedColor) { // button hover animation
        if (backgroundNormal.isValid() && backgroundHover.isValid()) {
            return KColorUtils::mix(backgroundNormal, backgroundHover, m_opacity);
        } else if (backgroundHover.isValid()) {
            return ColorTools::alphaMix(backgroundHover, m_opacity);
        } else
            return QColor();
    } else if (isHovered()) {
        return backgroundHover;
    } else {
        return backgroundNormal;
    }
}

// Returns a colour if an outline is to be drawn around the button
QColor Button::outlineColor(bool getNonAnimatedColor) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return QColor();

    auto c = d->client().toStrongRef();
    Q_ASSERT(c);
    const bool active = c->isActive();
    DecorationButtonPaletteGroup *group = active ? m_buttonPalette->active() : m_buttonPalette->inactive();
    QColor outlinePress = group->outlinePress;
    QColor outlineHover = group->outlineHover;
    QColor outlineNormal = group->outlineNormal;

    // active change state animation
    if (d->activeStateChangeAnimation()->state() == QAbstractAnimation::Running && !getNonAnimatedColor) {
        if (m_buttonPalette->active()->outlinePress.isValid() && m_buttonPalette->inactive()->outlinePress.isValid()) {
            outlinePress =
                KColorUtils::mix(m_buttonPalette->inactive()->outlinePress, m_buttonPalette->active()->outlinePress, d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->outlinePress.isValid() && !m_buttonPalette->inactive()->outlinePress.isValid()) {
            outlinePress = ColorTools::alphaMix(m_buttonPalette->active()->outlinePress, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->outlinePress.isValid() && m_buttonPalette->inactive()->outlinePress.isValid()) {
            outlinePress = ColorTools::alphaMix(m_buttonPalette->inactive()->outlinePress, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette->active()->outlineHover.isValid() && m_buttonPalette->inactive()->outlineHover.isValid()) {
            outlineHover =
                KColorUtils::mix(m_buttonPalette->inactive()->outlineHover, m_buttonPalette->active()->outlineHover, d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->outlineHover.isValid() && !m_buttonPalette->inactive()->outlineHover.isValid()) {
            outlineHover = ColorTools::alphaMix(m_buttonPalette->active()->outlineHover, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->outlineHover.isValid() && m_buttonPalette->inactive()->outlineHover.isValid()) {
            outlineHover = ColorTools::alphaMix(m_buttonPalette->inactive()->outlineHover, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette->active()->outlineNormal.isValid() && m_buttonPalette->inactive()->outlineNormal.isValid()) {
            outlineNormal =
                KColorUtils::mix(m_buttonPalette->inactive()->outlineNormal, m_buttonPalette->active()->outlineNormal, d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette->active()->outlineNormal.isValid() && !m_buttonPalette->inactive()->outlineNormal.isValid()) {
            outlineNormal = ColorTools::alphaMix(m_buttonPalette->active()->outlineNormal, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette->active()->outlineNormal.isValid() && m_buttonPalette->inactive()->outlineNormal.isValid()) {
            outlineNormal = ColorTools::alphaMix(m_buttonPalette->inactive()->outlineNormal, (1.0 - d->activeStateChangeAnimationOpacity()));
        }
    }

    // return a variant of normal, hover and press colours, depending on state
    if (isPressed()) {
        return outlinePress;
    } else if (isChecked()
               && (type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade
                   || (type() == DecorationButtonType::OnAllDesktops && !m_titlebarTextPinnedInversion))) {
        if (d->internalSettings()->buttonStateChecked(active) == InternalSettings::EnumButtonStateChecked::Hover) {
            return outlineHover;
        } else {
            return outlinePress;
        }
    } else if (m_animation->state() == QAbstractAnimation::Running && !getNonAnimatedColor) { // button hover animation
        if (outlineNormal.isValid() && outlineHover.isValid()) {
            return KColorUtils::mix(outlineNormal, outlineHover, m_opacity);
        } else if (outlineHover.isValid()) {
            return ColorTools::alphaMix(outlineHover, m_opacity);
        } else
            return QColor();
    } else if (isHovered()) {
        return outlineHover;
    } else {
        return outlineNormal;
    }
}

bool Button::titlebarTextPinnedInversion() const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return false;
    auto c = d->client().toStrongRef();
    Q_ASSERT(c);
    bool active = c->isActive();

    return type() == DecorationButtonType::OnAllDesktops
        && d->internalSettings()->buttonIconStyle() != InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme
        && (d->internalSettings()->buttonBackgroundOpacity(active) == 100 && d->internalSettings()->buttonIconOpacity(active) == 100
            && (((d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::TitlebarText
                  || d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)
                 && (d->internalSettings()->buttonIconColors(active) == InternalSettings::EnumButtonIconColors::TitlebarText
                     || d->internalSettings()->buttonIconColors(active) == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose))
                || ((d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::Accent
                     || d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
                     || d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights)
                    && (d->internalSettings()->buttonIconColors(active) == InternalSettings::EnumButtonIconColors::Accent
                        || d->internalSettings()->buttonIconColors(active) == InternalSettings::EnumButtonIconColors::AccentNegativeClose
                        || d->internalSettings()->buttonIconColors(active) == InternalSettings::EnumButtonIconColors::AccentTrafficLights)))

                )
        && !d->internalSettings()->showBackgroundNormally(active); // inversion occuring for compatibility with breeze's circular pin on all desktops icon
}

//________________________________________________________________
void Button::reconfigure()
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;

    // animation
    m_animation->setDuration(d->animationsDuration());

    // set m_systemIconName and m_systemIconCheckedName if a system icon theme is set
    if (d->internalSettings()->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme) {
        SystemIconTheme::systemIconNames(type(), m_systemIconName, m_systemIconCheckedName);
    }
}

//__________________________________________________________________
void Button::updateAnimationState(bool hovered)
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!(d && d->animationsDuration() > 0)) {
        return;
    }

    m_animation->setDirection(hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    if (m_animation->state() != QAbstractAnimation::Running) {
        m_animation->start();
    }
}

void Button::updateThinWindowOutlineWithButtonColor(bool on)
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d || !d->internalSettings()->colorizeThinWindowOutlineWithButton() || isStandAlone())
        return;

    QColor color = QColor();
    if (on) {
        m_buttonPalette = d->decorationColors()->buttonPalette(type()); // this is here in-case caching type on m_buttonPalette changes
        m_titlebarTextPinnedInversion = titlebarTextPinnedInversion();
        color = this->outlineColor(true); // generate colour again in non-animated state
        if (!color.isValid())
            color = this->backgroundColor(true); // use a background colour if outline colour not valid
        d->setThinWindowOutlineOverrideColor(on, color); // generate colour again in non-animated state
    } else {
        if (!isHovered() && isPressed())
            return; // don't remove the window outline highlight if the button is still pressed

        // Check if any other button is hovered/pressed.
        // This is to prevent glitches when you directly mouse over one button to another and the second button does not trigger on.
        // In the case where another button is hovered/pressed do not send an off flag.
        for (QPointer<KDecoration2::DecorationButton> &decButton : d->leftButtons()->buttons() + d->rightButtons()->buttons()) {
            Button *button = static_cast<Button *>(decButton.data());

            if (button != this && (button->isHovered() || button->isPressed())) {
                return;
            }
        }

        d->setThinWindowOutlineOverrideColor(on, color);
    }
}

void Button::paintFullHeightButtonBackground(QPainter *painter) const
{
    if (!m_backgroundColor.isValid() && !m_outlineColor.isValid())
        return;
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;
    auto s = d->settings();

    painter->save();
    painter->translate(m_fullHeightVisibleBackgroundOffset);

    qreal cornerRadius;
    if (d->scaledCornerRadius() >= 0.05)
        cornerRadius = d->scaledCornerRadius();
    else
        cornerRadius = 0;

    QRectF backgroundBoundingRect = (QRectF(geometry().topLeft(), m_backgroundVisibleSize));
    painter->setClipRect(backgroundBoundingRect);
    QPainterPath background;
    QPainterPath outline;
    painter->setPen(Qt::NoPen);

    bool drawOutlineUsingPath = false;

    qreal penWidth = PenWidth::Symbol;
    qreal geometryShrinkOffsetHorizontal = PenWidth::Symbol * 1.5;
    if (KWindowSystem::isPlatformX11()) {
        penWidth *= m_devicePixelRatio;
        geometryShrinkOffsetHorizontal *= m_devicePixelRatio;
    }

    if (m_outlineColor.isValid()) {
        qreal geometryShrinkOffsetVertical = geometryShrinkOffsetHorizontal;

        if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle) {
            // shrink the backgroundBoundingRect to make border more visible
            backgroundBoundingRect = QRectF(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal,
                                                                            geometryShrinkOffsetVertical,
                                                                            -geometryShrinkOffsetHorizontal,
                                                                            -geometryShrinkOffsetVertical));
            background.addRoundedRect(backgroundBoundingRect, cornerRadius, cornerRadius);

        } else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle) {
            QPainterPath inner;
            qreal halfPenWidth = penWidth / 2;
            geometryShrinkOffsetHorizontal = halfPenWidth;
            geometryShrinkOffsetVertical = halfPenWidth;
            qreal geometryShrinkOffsetHorizontalOuter = geometryShrinkOffsetHorizontal - halfPenWidth;
            qreal geometryShrinkOffsetHorizontalInner = geometryShrinkOffsetHorizontal + halfPenWidth;
            qreal geometryShrinkOffsetVerticalOuter = geometryShrinkOffsetVertical - halfPenWidth;
            qreal geometryShrinkOffsetVerticalInner = geometryShrinkOffsetVertical + halfPenWidth;

            qreal outerCornerRadius = cornerRadius + halfPenWidth;
            qreal innerCornerRadius = qMax(0.0, cornerRadius - halfPenWidth);

            drawOutlineUsingPath = true;

            if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin()) { // right-most-right
                outline = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(0, 0, 0, -geometryShrinkOffsetVerticalOuter),
                                                     CornerBottomLeft,
                                                     outerCornerRadius);
                inner = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(penWidth, 0, 0, -geometryShrinkOffsetVerticalInner),
                                                   CornerBottomLeft,
                                                   innerCornerRadius);
                background = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(halfPenWidth, 0, 0, -geometryShrinkOffsetVertical),
                                                        CornerBottomLeft,
                                                        cornerRadius);
            } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin()) { // left-most-left
                outline = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(0, 0, 0, -geometryShrinkOffsetVerticalOuter),
                                                     CornerBottomRight,
                                                     outerCornerRadius);
                inner = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(0, 0, -penWidth, -geometryShrinkOffsetVerticalInner),
                                                   CornerBottomRight,
                                                   innerCornerRadius);
                background = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(0, 0, -halfPenWidth, -geometryShrinkOffsetVertical),
                                                        CornerBottomRight,
                                                        cornerRadius);
            } else {
                outline = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalOuter,
                                                                                     0,
                                                                                     -geometryShrinkOffsetHorizontalOuter,
                                                                                     -geometryShrinkOffsetVerticalOuter),
                                                     CornersBottom,
                                                     outerCornerRadius);
                inner = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalInner,
                                                                                   0,
                                                                                   -geometryShrinkOffsetHorizontalInner,
                                                                                   -geometryShrinkOffsetVerticalInner),
                                                   CornersBottom,
                                                   innerCornerRadius);
                background = GeometryTools::roundedPath(
                    backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal, 0, -geometryShrinkOffsetHorizontal, -geometryShrinkOffsetVertical),
                    CornersBottom,
                    cornerRadius);
            }

            outline = outline.subtracted(inner);
        } else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangleGrouped) {
            if (type() != DecorationButtonType::Menu) {
                QPainterPath inner;
                qreal halfPenWidth = penWidth / 2;
                geometryShrinkOffsetHorizontal = halfPenWidth;
                geometryShrinkOffsetVertical = halfPenWidth;
                qreal geometryShrinkOffsetHorizontalOuter = geometryShrinkOffsetHorizontal - halfPenWidth;
                qreal geometryShrinkOffsetHorizontalInner = geometryShrinkOffsetHorizontal + halfPenWidth;

                qreal geometryShrinkOffsetHorizontalMiddle;
                qreal geometryShrinkOffsetHorizontalMiddleOuter;
                qreal geometryShrinkOffsetHorizontalMiddleInner;
                if ((m_leftButtonVisible && d->internalSettings()->fullHeightButtonSpacingLeft() == 0)
                    || (m_rightButtonVisible && d->internalSettings()->fullHeightButtonSpacingRight() == 0)) {
                    geometryShrinkOffsetHorizontalMiddle = 0;
                    geometryShrinkOffsetHorizontalMiddleOuter = -halfPenWidth;
                    geometryShrinkOffsetHorizontalMiddleInner = halfPenWidth;
                } else {
                    geometryShrinkOffsetHorizontalMiddle = geometryShrinkOffsetHorizontal;
                    geometryShrinkOffsetHorizontalMiddleOuter = geometryShrinkOffsetHorizontalOuter;
                    geometryShrinkOffsetHorizontalMiddleInner = geometryShrinkOffsetHorizontalInner;
                }

                qreal geometryShrinkOffsetVerticalOuter = geometryShrinkOffsetVertical - halfPenWidth;
                qreal geometryShrinkOffsetVerticalInner = geometryShrinkOffsetVertical + halfPenWidth;

                qreal outerCornerRadius = cornerRadius + halfPenWidth;
                qreal innerCornerRadius = qMax(0.0, cornerRadius - halfPenWidth);

                drawOutlineUsingPath = true;

                if (((m_leftmostLeftVisible && d->internalSettings()->titlebarLeftMargin()) && m_rightmostLeftVisible)
                    || (m_visibleAfterMenu && (m_rightmostRightVisible || m_rightmostLeftVisible))
                    || ((m_rightmostRightVisible && d->internalSettings()->titlebarRightMargin()) && m_leftmostRightVisible)
                    || (m_visibleBeforeMenu && (m_leftmostRightVisible || m_leftmostLeftVisible))) {
                    outline = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalOuter,
                                                                                         0,
                                                                                         -geometryShrinkOffsetHorizontalOuter,
                                                                                         -geometryShrinkOffsetVerticalOuter),
                                                         CornersBottom,
                                                         outerCornerRadius);
                    inner = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalInner,
                                                                                       0,
                                                                                       -geometryShrinkOffsetHorizontalInner,
                                                                                       -geometryShrinkOffsetVerticalInner),
                                                       CornersBottom,
                                                       innerCornerRadius);
                    background = GeometryTools::roundedPath(
                        backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal, 0, -geometryShrinkOffsetHorizontal, -geometryShrinkOffsetVertical),
                        CornersBottom,
                        cornerRadius);
                } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin() && m_rightmostLeftVisible) {
                    outline = GeometryTools::roundedPath(
                        backgroundBoundingRect.adjusted(0, 0, -geometryShrinkOffsetHorizontalOuter, -geometryShrinkOffsetVerticalOuter),
                        CornerBottomRight,
                        outerCornerRadius);
                    inner = GeometryTools::roundedPath(
                        backgroundBoundingRect.adjusted(0, 0, -geometryShrinkOffsetHorizontalInner, -geometryShrinkOffsetVerticalInner),
                        CornerBottomRight,
                        innerCornerRadius);
                    background =
                        GeometryTools::roundedPath(backgroundBoundingRect.adjusted(0, 0, -geometryShrinkOffsetHorizontal, -geometryShrinkOffsetVertical),
                                                   CornerBottomRight,
                                                   cornerRadius);
                } else if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin() && m_leftmostRightVisible) {
                    outline = GeometryTools::roundedPath(
                        backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalOuter, 0, 0, -geometryShrinkOffsetVerticalOuter),
                        CornerBottomLeft,
                        outerCornerRadius);
                    inner = GeometryTools::roundedPath(
                        backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalInner, 0, 0, -geometryShrinkOffsetVerticalInner),
                        CornerBottomLeft,
                        innerCornerRadius);
                    background =
                        GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal, 0, 0, -geometryShrinkOffsetVertical),
                                                   CornerBottomLeft,
                                                   cornerRadius);
                } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin()) {
                    outline.addRect(backgroundBoundingRect.adjusted(0, 0, -geometryShrinkOffsetHorizontalMiddleOuter, -geometryShrinkOffsetVerticalOuter));
                    inner.addRect(backgroundBoundingRect.adjusted(0, 0, -geometryShrinkOffsetHorizontalMiddleInner, -geometryShrinkOffsetVerticalInner));
                    background.addRect(backgroundBoundingRect.adjusted(0, 0, -geometryShrinkOffsetHorizontalMiddle, -geometryShrinkOffsetVertical));
                } else if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin()) {
                    outline.addRect(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddleOuter, 0, 0, -geometryShrinkOffsetVerticalOuter));
                    inner.addRect(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddleInner, 0, 0, -geometryShrinkOffsetVerticalInner));
                    background.addRect(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddle, 0, 0, -geometryShrinkOffsetVertical));
                } else if ((m_rightmostRightVisible && d->internalSettings()->titlebarRightMargin()) || m_visibleBeforeMenu || m_rightmostLeftVisible) {
                    outline = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddleOuter,
                                                                                         0,
                                                                                         -geometryShrinkOffsetHorizontalOuter,
                                                                                         -geometryShrinkOffsetVerticalOuter),
                                                         CornerBottomRight,
                                                         outerCornerRadius);
                    inner = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddleInner,
                                                                                       0,
                                                                                       -geometryShrinkOffsetHorizontalInner,
                                                                                       -geometryShrinkOffsetVerticalInner),
                                                       CornerBottomRight,
                                                       innerCornerRadius);
                    background = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddle,
                                                                                            0,
                                                                                            -geometryShrinkOffsetHorizontal,
                                                                                            -geometryShrinkOffsetVertical),
                                                            CornerBottomRight,
                                                            cornerRadius);
                } else if ((m_leftmostLeftVisible && d->internalSettings()->titlebarLeftMargin()) || m_visibleAfterMenu || m_leftmostRightVisible) {
                    outline = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalOuter,
                                                                                         0,
                                                                                         -geometryShrinkOffsetHorizontalMiddleOuter,
                                                                                         -geometryShrinkOffsetVerticalOuter),
                                                         CornerBottomLeft,
                                                         outerCornerRadius);
                    inner = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalInner,
                                                                                       0,
                                                                                       -geometryShrinkOffsetHorizontalMiddleInner,
                                                                                       -geometryShrinkOffsetVerticalInner),
                                                       CornerBottomLeft,
                                                       innerCornerRadius);
                    background = GeometryTools::roundedPath(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal,
                                                                                            0,
                                                                                            -geometryShrinkOffsetHorizontalMiddle,
                                                                                            -geometryShrinkOffsetVertical),
                                                            CornerBottomLeft,
                                                            cornerRadius);
                } else {
                    outline.addRect(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddleOuter,
                                                                    0,
                                                                    -geometryShrinkOffsetHorizontalMiddleOuter,
                                                                    -geometryShrinkOffsetVerticalOuter));
                    inner.addRect(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddleInner,
                                                                  0,
                                                                  -geometryShrinkOffsetHorizontalMiddleInner,
                                                                  -geometryShrinkOffsetVerticalInner));
                    background.addRect(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalMiddle,
                                                                       0,
                                                                       -geometryShrinkOffsetHorizontalMiddle,
                                                                       -geometryShrinkOffsetVertical));
                }

                outline = outline.subtracted(inner);
            }
        } else { // plain rectangle

            // shrink the backgroundBoundingRect to make border more visible
            backgroundBoundingRect = backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal,
                                                                     geometryShrinkOffsetVertical,
                                                                     -geometryShrinkOffsetHorizontal,
                                                                     -geometryShrinkOffsetVertical);
            background.addRect(backgroundBoundingRect);
        }

    } else { // non-shrunk background without outline
        painter->setPen(Qt::NoPen);
        if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle) {
            background.addRoundedRect(backgroundBoundingRect, cornerRadius, cornerRadius);

        } else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle) {
            if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin()) { // right-most-right
                background = GeometryTools::roundedPath(backgroundBoundingRect, CornerBottomLeft, cornerRadius);
            } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin()) { // left-most-left
                background = GeometryTools::roundedPath(backgroundBoundingRect, CornerBottomRight, cornerRadius);
            } else {
                background = GeometryTools::roundedPath(backgroundBoundingRect, CornersBottom, cornerRadius);
            }
        } else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangleGrouped) {
            if (type() != DecorationButtonType::Menu) {
                painter->setPen(Qt::NoPen);

                if (((m_leftmostLeftVisible && d->internalSettings()->titlebarLeftMargin()) && m_rightmostLeftVisible)
                    || (m_visibleAfterMenu && (m_rightmostRightVisible || m_rightmostLeftVisible))
                    || ((m_rightmostRightVisible && d->internalSettings()->titlebarRightMargin()) && m_leftmostRightVisible)
                    || (m_visibleBeforeMenu && (m_leftmostRightVisible || m_leftmostLeftVisible))) {
                    background = GeometryTools::roundedPath(backgroundBoundingRect, CornersBottom, cornerRadius);
                } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin() && m_rightmostLeftVisible) {
                    background = GeometryTools::roundedPath(backgroundBoundingRect, CornerBottomRight, cornerRadius);
                } else if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin() && m_leftmostRightVisible) {
                    background = GeometryTools::roundedPath(backgroundBoundingRect, CornerBottomLeft, cornerRadius);
                } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin()) {
                    background.addRect(backgroundBoundingRect);
                } else if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin()) {
                    background.addRect(backgroundBoundingRect);
                } else if ((m_rightmostRightVisible && d->internalSettings()->titlebarRightMargin()) || m_visibleBeforeMenu || m_rightmostLeftVisible) {
                    background = GeometryTools::roundedPath(backgroundBoundingRect, CornerBottomRight, cornerRadius);
                } else if ((m_leftmostLeftVisible && d->internalSettings()->titlebarLeftMargin()) || m_visibleAfterMenu || m_leftmostRightVisible) {
                    background = GeometryTools::roundedPath(backgroundBoundingRect, CornerBottomLeft, cornerRadius);
                } else {
                    background.addRect(backgroundBoundingRect);
                }
            }
        } else { // plain rectangle
            background.addRect(backgroundBoundingRect);
        }
    }

    // clip the rounded corners using the windowPath
    if (!d->isMaximized() && (!(!m_backgroundColor.isValid() && m_outlineColor.isValid() && drawOutlineUsingPath)))
        background = background.intersected(*(d->windowPath()));

    if (m_outlineColor.isValid() && !drawOutlineUsingPath) {
        QPen pen(m_outlineColor);
        pen.setWidthF(m_standardScaledCosmeticPenWidth);
        pen.setCosmetic(true);
        painter->setPen(pen);
    }
    if (m_backgroundColor.isValid()) {
        painter->setBrush(m_backgroundColor);
        painter->drawPath(background);
    } else if (m_outlineColor.isValid() && !drawOutlineUsingPath) {
        painter->drawPath(background);
    }

    if (m_outlineColor.isValid() && drawOutlineUsingPath) {
        // clip the rounded corners using the windowPath
        if (!d->isMaximized())
            outline = outline.intersected(*(d->windowPath()));
        painter->setBrush(m_outlineColor);
        painter->drawPath(outline);
    }

    painter->restore();
}

void Button::paintSmallSizedButtonBackground(QPainter *painter) const
{
    if (!m_backgroundColor.isValid() && (!m_outlineColor.isValid()))
        return;
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;

    painter->save();

    qreal translationOffset = (m_smallButtonPaddedSize.width() - m_backgroundVisibleSize.width()) / 2;
    painter->translate(translationOffset, translationOffset);
    qreal geometryEnlargeOffset = 0;
    qreal backgroundSize = m_backgroundVisibleSize.width();

    qreal penWidth = m_isGtkCsdButton ? m_standardScaledNonCosmeticPenWidth : PenWidth::Symbol;
    if (KWindowSystem::isPlatformX11()) {
        penWidth *= m_devicePixelRatio;
    }

    if (m_outlineColor.isValid()) {
        QPen pen(m_outlineColor);
        if (m_isGtkCsdButton) { // kde-gtk-config GTK CSD button generator does not work properly with cosmetic pens
            pen.setWidthF(penWidth);
            pen.setCosmetic(false);
        } else { // standard case
            pen.setWidthF(m_standardScaledCosmeticPenWidth); // this is a scaled pen width for use with drawing cosmetic pen outlines
            pen.setCosmetic(true);
        }
        painter->setPen(pen);
    } else
        painter->setPen(Qt::NoPen);
    if (m_backgroundColor.isValid())
        painter->setBrush(m_backgroundColor);
    else
        painter->setBrush(Qt::NoBrush);

    if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeSmallSquare
        || d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRectangle
        || ((d->internalSettings()->cornerRadius() < 0.2)
            && (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle))
        || ((d->internalSettings()->cornerRadius() < 0.2)
            && (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle))) {
        if (m_outlineColor.isValid())
            geometryEnlargeOffset = penWidth / 2;
        painter->drawRect(QRectF(0 - geometryEnlargeOffset,
                                 0 - geometryEnlargeOffset,
                                 backgroundSize + geometryEnlargeOffset * 2,
                                 backgroundSize + geometryEnlargeOffset * 2));
    } else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeSmallRoundedSquare
               || d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle // case where standalone
               || d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle // case where standalone
    ) {
        if (m_outlineColor.isValid())
            geometryEnlargeOffset = penWidth / 2;
        painter->drawRoundedRect(QRectF(0 - geometryEnlargeOffset,
                                        0 - geometryEnlargeOffset,
                                        backgroundSize + geometryEnlargeOffset * 2,
                                        backgroundSize + geometryEnlargeOffset * 2),
                                 20,
                                 20,
                                 Qt::RelativeSize);
    } else
        painter->drawEllipse(QRectF(0 - geometryEnlargeOffset,
                                    0 - geometryEnlargeOffset,
                                    backgroundSize + geometryEnlargeOffset * 2,
                                    backgroundSize + geometryEnlargeOffset * 2));

    painter->restore();
}

void Button::setDevicePixelRatio(QPainter *painter)
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;
    // determine DPR
    m_devicePixelRatio = painter->device()->devicePixelRatioF();

    // on X11 Kwin just returns 1.0 for the DPR instead of the correct value, so use the scaling setting directly
    if (KWindowSystem::isPlatformX11())
        m_devicePixelRatio = d->systemScaleFactorX11();
    if (m_isGtkCsdButton)
        m_devicePixelRatio = 1.0;
}

void Button::setStandardScaledPenWidth()
{
    m_standardScaledCosmeticPenWidth = PenWidth::Symbol;
    m_standardScaledCosmeticPenWidth *= m_devicePixelRatio; // this is assuming you are going to use setCosmetic(true) for pen sizes
}

void Button::setShouldDrawBoldButtonIcons()
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;

    m_boldButtonIcons = false;

    if (!m_isGtkCsdButton) {
        switch (d->internalSettings()->boldButtonIcons()) {
        default:
            break;
        case InternalSettings::EnumBoldButtonIcons::BoldIconsHiDpiOnly:
            // If HiDPI system scaling use bold icons
            if (m_devicePixelRatio > 1.2)
                m_boldButtonIcons = true;
            break;
        case InternalSettings::EnumBoldButtonIcons::BoldIconsBold:
            m_boldButtonIcons = true;
            break;
        case InternalSettings::EnumBoldButtonIcons::BoldIconsFine:
            break;
        }
    }
}

bool Button::isSystemIconAvailable() const
{
    if (isChecked()) {
        if (m_systemIconCheckedName.isEmpty())
            return false;
        else
            return true;
    } else {
        if (m_systemIconName.isEmpty())
            return false;
        else
            return true;
    }
}

} // namespace
