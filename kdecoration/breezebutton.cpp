/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezebutton.h"
#include "colortools.h"
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
    , m_buttonPalette(type)
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
    ;
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
    Q_UNUSED(repaintRegion)

    if (!decoration()) {
        return;
    }
    auto d = qobject_cast<Decoration *>(decoration());
    auto c = d->client().toStrongRef();
    Q_ASSERT(c);

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
        if (auto deco = qobject_cast<Decoration *>(decoration())) {
            const QPalette activePalette = KIconLoader::global()->customPalette();
            QPalette palette = c->palette();
            palette.setColor(QPalette::WindowText, deco->fontColor());
            KIconLoader::global()->setCustomPalette(palette);
            c->icon().paint(painter, iconRect.toRect());
            if (activePalette == QPalette()) {
                KIconLoader::global()->resetPalette();
            } else {
                KIconLoader::global()->setCustomPalette(palette);
            }
        } else {
            c->icon().paint(painter, iconRect.toRect());
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

    // get the device offset of the paddedIcon from the top-left of the titlebar as a reference-point for pixel-snapping algorithms
    //(ideally, the device offset from the top-left of the screen would be better for fractional scaling, but it is not available in the API)
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

    if (d->buttonBackgroundType() == ButtonBackgroundType::Small || isStandAlone() || m_isGtkCsdButton)
        paintSmallSizedButtonBackground(painter);

    if (!m_foregroundColor.isValid())
        return;

    // render the actual icon
    const qreal smallButtonPaddedWidth(m_smallButtonPaddedSize.width());
    qreal iconWidth(m_iconSize.width());

    // translate to draw icon in the centre of smallButtonPaddedWidth (smallButtonPaddedWidth has additional padding)
    qreal iconTranslationOffset = (smallButtonPaddedWidth - iconWidth) / 2;
    painter->translate(iconTranslationOffset, iconTranslationOffset);
    deviceOffsetDecorationTopLeftToIconTopLeft += (QPointF(iconTranslationOffset, iconTranslationOffset) * painter->device()->devicePixelRatioF());

    // setup painter
    QPen pen(m_foregroundColor);

    // this method commented out is for original non-cosmetic pen painting method (gives blurry icons at larger sizes )
    // pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/smallButtonPaddedWidth ) );

    // cannot use a scaled cosmetic pen if GTK CSD as kde-gtk-config generates svg icons.
    if (m_isGtkCsdButton) {
        pen.setWidthF(PenWidth::Symbol);
    } else {
        pen.setWidthF(m_standardScaledPenWidth);
        pen.setCosmetic(true);
    }
    painter->setPen(pen);

    if (m_renderSystemIcon) {
        QString systemIconName;
        systemIconName = isChecked() ? m_systemIconCheckedName : m_systemIconName;
        SystemIconTheme iconRenderer(painter, iconWidth, systemIconName, d->internalSettings(), m_devicePixelRatio);
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
        localRenderingWidth is typically 18 or 16
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
    DecorationButtonPaletteGroup *group = active ? m_buttonPalette.active() : m_buttonPalette.inactive();
    QColor foregroundPress = group->foregroundPress;
    QColor foregroundHover = group->foregroundHover;
    QColor foregroundNormal = group->foregroundNormal;

    // active change state animation
    if (d->activeStateChangeAnimation()->state() == QAbstractAnimation::Running) {
        if (m_buttonPalette.active()->foregroundPress.isValid() && m_buttonPalette.inactive()->foregroundPress.isValid()) {
            foregroundPress = KColorUtils::mix(m_buttonPalette.inactive()->foregroundPress,
                                               m_buttonPalette.active()->foregroundPress,
                                               d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->foregroundPress.isValid() && !m_buttonPalette.inactive()->foregroundPress.isValid()) {
            foregroundPress = ColorTools::alphaMix(m_buttonPalette.active()->foregroundPress, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->foregroundPress.isValid() && m_buttonPalette.inactive()->foregroundPress.isValid()) {
            foregroundPress = ColorTools::alphaMix(m_buttonPalette.inactive()->foregroundPress, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette.active()->foregroundHover.isValid() && m_buttonPalette.inactive()->foregroundHover.isValid()) {
            foregroundHover = KColorUtils::mix(m_buttonPalette.inactive()->foregroundHover,
                                               m_buttonPalette.active()->foregroundHover,
                                               d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->foregroundHover.isValid() && !m_buttonPalette.inactive()->foregroundHover.isValid()) {
            foregroundHover = ColorTools::alphaMix(m_buttonPalette.active()->foregroundHover, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->foregroundHover.isValid() && m_buttonPalette.inactive()->foregroundHover.isValid()) {
            foregroundHover = ColorTools::alphaMix(m_buttonPalette.inactive()->foregroundHover, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette.active()->foregroundNormal.isValid() && m_buttonPalette.inactive()->foregroundNormal.isValid()) {
            foregroundNormal = KColorUtils::mix(m_buttonPalette.inactive()->foregroundNormal,
                                                m_buttonPalette.active()->foregroundNormal,
                                                d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->foregroundNormal.isValid() && !m_buttonPalette.inactive()->foregroundNormal.isValid()) {
            foregroundNormal = ColorTools::alphaMix(m_buttonPalette.active()->foregroundNormal, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->foregroundNormal.isValid() && m_buttonPalette.inactive()->foregroundNormal.isValid()) {
            foregroundNormal = ColorTools::alphaMix(m_buttonPalette.inactive()->foregroundNormal, (1.0 - d->activeStateChangeAnimationOpacity()));
        }
    }

    bool nonTranslucentTitlebarTextPinnedInversion =
        (d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::TitlebarText
         || d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)
        && !d->internalSettings()->translucentButtonBackgrounds(active)
        && !d->buttonBehaviour().drawBackgroundNormally; // inversion occuring for compatibility with breeze's circular pin on all desktops icon

    // return a variant of normal, hover and press colours, depending on state
    if (isPressed()) {
        return foregroundPress;
    } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade
                || (type() == DecorationButtonType::OnAllDesktops && !nonTranslucentTitlebarTextPinnedInversion))
               && isChecked()) {
        if (nonTranslucentTitlebarTextPinnedInversion) {
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
    DecorationButtonPaletteGroup *group = active ? m_buttonPalette.active() : m_buttonPalette.inactive();
    QColor backgroundPress = group->backgroundPress;
    QColor backgroundHover = group->backgroundHover;
    QColor backgroundNormal = group->backgroundNormal;

    // active change state animation
    if (d->activeStateChangeAnimation()->state() == QAbstractAnimation::Running && !getNonAnimatedColor) {
        if (m_buttonPalette.active()->backgroundPress.isValid() && m_buttonPalette.inactive()->backgroundPress.isValid()) {
            backgroundPress = KColorUtils::mix(m_buttonPalette.inactive()->backgroundPress,
                                               m_buttonPalette.active()->backgroundPress,
                                               d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->backgroundPress.isValid() && !m_buttonPalette.inactive()->backgroundPress.isValid()) {
            backgroundPress = ColorTools::alphaMix(m_buttonPalette.active()->backgroundPress, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->backgroundPress.isValid() && m_buttonPalette.inactive()->backgroundPress.isValid()) {
            backgroundPress = ColorTools::alphaMix(m_buttonPalette.inactive()->backgroundPress, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette.active()->backgroundHover.isValid() && m_buttonPalette.inactive()->backgroundHover.isValid()) {
            backgroundHover = KColorUtils::mix(m_buttonPalette.inactive()->backgroundHover,
                                               m_buttonPalette.active()->backgroundHover,
                                               d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->backgroundHover.isValid() && !m_buttonPalette.inactive()->backgroundHover.isValid()) {
            backgroundHover = ColorTools::alphaMix(m_buttonPalette.active()->backgroundHover, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->backgroundHover.isValid() && m_buttonPalette.inactive()->backgroundHover.isValid()) {
            backgroundHover = ColorTools::alphaMix(m_buttonPalette.inactive()->backgroundHover, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette.active()->backgroundNormal.isValid() && m_buttonPalette.inactive()->backgroundNormal.isValid()) {
            backgroundNormal = KColorUtils::mix(m_buttonPalette.inactive()->backgroundNormal,
                                                m_buttonPalette.active()->backgroundNormal,
                                                d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->backgroundNormal.isValid() && !m_buttonPalette.inactive()->backgroundNormal.isValid()) {
            backgroundNormal = ColorTools::alphaMix(m_buttonPalette.active()->backgroundNormal, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->backgroundNormal.isValid() && m_buttonPalette.inactive()->backgroundNormal.isValid()) {
            backgroundNormal = ColorTools::alphaMix(m_buttonPalette.inactive()->backgroundNormal, (1.0 - d->activeStateChangeAnimationOpacity()));
        }
    }

    bool nonTranslucentTitlebarTextPinnedInversion =
        (d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::TitlebarText
         || d->internalSettings()->buttonBackgroundColors(active) == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)
        && !d->internalSettings()->translucentButtonBackgrounds(active)
        && !d->buttonBehaviour().drawBackgroundNormally; // inversion occuring for compatibility with breeze's circular pin on all desktops icon

    // return a variant of normal, hover and press colours, depending on state
    if (isPressed()) {
        return backgroundPress;
    } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade
                || (type() == DecorationButtonType::OnAllDesktops && !nonTranslucentTitlebarTextPinnedInversion))
               && isChecked()) {
        return backgroundPress;
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
    DecorationButtonPaletteGroup *group = active ? m_buttonPalette.active() : m_buttonPalette.inactive();
    QColor outlinePress = group->outlinePress;
    QColor outlineHover = group->outlineHover;
    QColor outlineNormal = group->outlineNormal;

    // active change state animation
    if (d->activeStateChangeAnimation()->state() == QAbstractAnimation::Running && !getNonAnimatedColor) {
        if (m_buttonPalette.active()->outlinePress.isValid() && m_buttonPalette.inactive()->outlinePress.isValid()) {
            outlinePress =
                KColorUtils::mix(m_buttonPalette.inactive()->outlinePress, m_buttonPalette.active()->outlinePress, d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->outlinePress.isValid() && !m_buttonPalette.inactive()->outlinePress.isValid()) {
            outlinePress = ColorTools::alphaMix(m_buttonPalette.active()->outlinePress, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->outlinePress.isValid() && m_buttonPalette.inactive()->outlinePress.isValid()) {
            outlinePress = ColorTools::alphaMix(m_buttonPalette.inactive()->outlinePress, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette.active()->outlineHover.isValid() && m_buttonPalette.inactive()->outlineHover.isValid()) {
            outlineHover =
                KColorUtils::mix(m_buttonPalette.inactive()->outlineHover, m_buttonPalette.active()->outlineHover, d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->outlineHover.isValid() && !m_buttonPalette.inactive()->outlineHover.isValid()) {
            outlineHover = ColorTools::alphaMix(m_buttonPalette.active()->outlineHover, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->outlineHover.isValid() && m_buttonPalette.inactive()->outlineHover.isValid()) {
            outlineHover = ColorTools::alphaMix(m_buttonPalette.inactive()->outlineHover, (1.0 - d->activeStateChangeAnimationOpacity()));
        }

        if (m_buttonPalette.active()->outlineNormal.isValid() && m_buttonPalette.inactive()->outlineNormal.isValid()) {
            outlineNormal =
                KColorUtils::mix(m_buttonPalette.inactive()->outlineNormal, m_buttonPalette.active()->outlineNormal, d->activeStateChangeAnimationOpacity());
        } else if (m_buttonPalette.active()->outlineNormal.isValid() && !m_buttonPalette.inactive()->outlineNormal.isValid()) {
            outlineNormal = ColorTools::alphaMix(m_buttonPalette.active()->outlineNormal, d->activeStateChangeAnimationOpacity());
        } else if (!m_buttonPalette.active()->outlineNormal.isValid() && m_buttonPalette.inactive()->outlineNormal.isValid()) {
            outlineNormal = ColorTools::alphaMix(m_buttonPalette.inactive()->outlineNormal, (1.0 - d->activeStateChangeAnimationOpacity()));
        }
    }

    // return a variant of normal, hover and press colours, depending on state
    if (isPressed()) {
        return outlinePress;
    } else if ((isChecked()
                && (type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade))) {
        return outlinePress;
    } else if (type() == DecorationButtonType::OnAllDesktops && isChecked()) {
        return outlinePress;
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

//________________________________________________________________
void Button::reconfigure()
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;

    // animation
    m_animation->setDuration(d->animationsDuration());

    // button colours
    m_buttonPalette.reconfigure(d->internalSettings(),
                                &d->buttonBehaviour(),
                                d->decorationPalette().get(),
                                d->fontColor(true, true, true),
                                d->titleBarColor(true, true, true),
                                d->fontColor(true, true, false),
                                d->titleBarColor(true, true, false));

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
        QRectF innerRect;
        QRectF outerRect;

        qreal geometryShrinkOffsetVertical = geometryShrinkOffsetHorizontal;

        if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle) {
            // shrink the backgroundBoundingRect to make border more visible
            backgroundBoundingRect = QRectF(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal,
                                                                            geometryShrinkOffsetVertical,
                                                                            -geometryShrinkOffsetHorizontal,
                                                                            -geometryShrinkOffsetVertical));
            background.addRoundedRect(backgroundBoundingRect, d->scaledCornerRadius(), d->scaledCornerRadius());

        } else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle) {
            qreal halfPenWidth = penWidth / 2;
            geometryShrinkOffsetHorizontal = halfPenWidth;
            geometryShrinkOffsetVertical = qMax(0.0, d->internalSettings()->integratedRoundedRectangleBottomPadding() * s->smallSpacing() - penWidth);
            qreal geometryShrinkOffsetHorizontalOuter = geometryShrinkOffsetHorizontal - halfPenWidth;
            qreal geometryShrinkOffsetHorizontalInner = geometryShrinkOffsetHorizontal + halfPenWidth;
            qreal geometryShrinkOffsetVerticalOuter = geometryShrinkOffsetVertical - halfPenWidth;
            qreal geometryShrinkOffsetVerticalInner = geometryShrinkOffsetVertical + halfPenWidth;
            qreal extensionByCornerRadiusInnerOuter = d->scaledCornerRadius() + halfPenWidth;
            drawOutlineUsingPath = true;

            if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin()) { // right-most-right
                outerRect = backgroundBoundingRect.adjusted(0,
                                                            -extensionByCornerRadiusInnerOuter,
                                                            extensionByCornerRadiusInnerOuter,
                                                            -geometryShrinkOffsetVerticalOuter);
                innerRect = backgroundBoundingRect.adjusted(penWidth,
                                                            -extensionByCornerRadiusInnerOuter,
                                                            extensionByCornerRadiusInnerOuter,
                                                            -geometryShrinkOffsetVerticalInner);
                backgroundBoundingRect =
                    backgroundBoundingRect.adjusted(halfPenWidth, -d->scaledCornerRadius(), d->scaledCornerRadius(), -geometryShrinkOffsetVertical);
            } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin()) { // left-most-left
                outerRect = backgroundBoundingRect.adjusted(-extensionByCornerRadiusInnerOuter,
                                                            -extensionByCornerRadiusInnerOuter,
                                                            0,
                                                            -geometryShrinkOffsetVerticalOuter);
                innerRect = backgroundBoundingRect.adjusted(-extensionByCornerRadiusInnerOuter,
                                                            -extensionByCornerRadiusInnerOuter,
                                                            -penWidth,
                                                            -geometryShrinkOffsetVerticalInner);
                backgroundBoundingRect =
                    backgroundBoundingRect.adjusted(-d->scaledCornerRadius(), -d->scaledCornerRadius(), -halfPenWidth, -geometryShrinkOffsetVertical);
            } else {
                outerRect = backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalOuter,
                                                            -extensionByCornerRadiusInnerOuter,
                                                            -geometryShrinkOffsetHorizontalOuter,
                                                            -geometryShrinkOffsetVerticalOuter);
                innerRect = backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalInner,
                                                            -extensionByCornerRadiusInnerOuter,
                                                            -geometryShrinkOffsetHorizontalInner,
                                                            -geometryShrinkOffsetVerticalInner);
                backgroundBoundingRect = backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal,
                                                                         -d->scaledCornerRadius(),
                                                                         -geometryShrinkOffsetHorizontal,
                                                                         -geometryShrinkOffsetVertical);
            }

            qreal outerCornerRadius;
            if (d->scaledCornerRadius() >= 0.05)
                outerCornerRadius = d->scaledCornerRadius() + halfPenWidth;
            else
                outerCornerRadius = 0;
            qreal innerCornerRadius = qMax(0.0, d->scaledCornerRadius() - halfPenWidth);
            QPainterPath inner;
            inner.addRoundedRect(innerRect, innerCornerRadius, innerCornerRadius);
            outline.addRoundedRect(outerRect, outerCornerRadius, outerCornerRadius);
            outline = outline.subtracted(inner);
            background.addRoundedRect(backgroundBoundingRect, d->scaledCornerRadius(), d->scaledCornerRadius());
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
        if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle)
            background.addRoundedRect(backgroundBoundingRect, d->scaledCornerRadius(), d->scaledCornerRadius());

        else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle) {
            qreal geometryShrinkOffsetVertical = d->internalSettings()->integratedRoundedRectangleBottomPadding() * s->smallSpacing() - penWidth;
            if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin()) { // right-most-right
                backgroundBoundingRect = backgroundBoundingRect.adjusted(0, -d->scaledCornerRadius(), d->scaledCornerRadius(), -geometryShrinkOffsetVertical);
            } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin()) { // left-most-left
                backgroundBoundingRect = backgroundBoundingRect.adjusted(-d->scaledCornerRadius(), -d->scaledCornerRadius(), 0, -geometryShrinkOffsetVertical);
            } else {
                backgroundBoundingRect = backgroundBoundingRect.adjusted(0, -d->scaledCornerRadius(), 0, -geometryShrinkOffsetVertical);
            }
            background.addRoundedRect(backgroundBoundingRect, d->scaledCornerRadius(), d->scaledCornerRadius());
        } else // plain rectangle
            background.addRect(backgroundBoundingRect);
    }

    // clip the rounded corners using the windowPath
    if (!d->isMaximized() && (!(!m_backgroundColor.isValid() && m_outlineColor.isValid() && drawOutlineUsingPath)))
        background = background.intersected(*(d->windowPath()));

    if (m_outlineColor.isValid() && !drawOutlineUsingPath) {
        QPen pen(m_outlineColor);
        pen.setWidthF(m_standardScaledPenWidth);
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

    qreal penWidth = PenWidth::Symbol;
    if (KWindowSystem::isPlatformX11()) {
        penWidth *= m_devicePixelRatio;
    }

    if (m_outlineColor.isValid()) {
        QPen pen(m_outlineColor);
        if (m_isGtkCsdButton) { // kde-gtk-config GTK CSD button generator does not work properly with cosmetic pens
            pen.setWidthF(penWidth);
            pen.setCosmetic(false);
        } else { // standard case
            pen.setWidthF(m_standardScaledPenWidth); // this is a scaled pen width for use with drawing cosmetic pen outlines
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
        m_devicePixelRatio = d->systemScaleFactor();
    if (m_isGtkCsdButton)
        m_devicePixelRatio = d->systemScaleFactor();
}

void Button::setStandardScaledPenWidth()
{
    m_standardScaledPenWidth = PenWidth::Symbol;
    m_standardScaledPenWidth *= m_devicePixelRatio; // this is assuming you are going to use setCosmetic(true) for pen sizes
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
