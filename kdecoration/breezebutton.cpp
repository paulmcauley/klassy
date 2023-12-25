/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezebutton.h"
#include "colortools.h"
#include "renderdecorationbuttonicon.h"
#include "stylesystemicontheme.h"

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
    connect(decoration->settings().data(), &KDecoration2::DecorationSettings::reconfigured, this, &Button::reconfigure);
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

    QColor backgroundColorToContrastWithForeground;
    bool negativeBackgroundClosePalette;
    m_backgroundColor = this->backgroundColor(backgroundColorToContrastWithForeground, negativeBackgroundClosePalette);
    m_foregroundColor = this->foregroundColor(backgroundColorToContrastWithForeground, m_backgroundColor.isValid() && negativeBackgroundClosePalette);

    m_outlineColor = this->outlineColor();

    // cache colours for future animations
    if (m_animation->state() != QAbstractAnimation::Running && !isHovered()) {
        m_preAnimationForegroundColor = m_foregroundColor;
        m_preAnimationBackgroundColor = m_backgroundColor;
        m_preAnimationOutlineColor = m_outlineColor;
    }

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
    QPointF deviceOffsetTitleBarTopLeftToIconTopLeft;
    QPointF topLeftPaddedButtonDeviceGeometry = painter->deviceTransform().map(geometry().topLeft());

    // get top-left geometry relative to the titlebar top-left as is the best reference position available that is most likely to be a whole pixel
    //(on button hover sometimes the painter gives geometry relative to the button rather than to titlebar, so this is also why this is necessary)
    QPointF titleBarTopLeftDeviceGeometry = painter->deviceTransform().map(QRectF(d->titleBar()).topLeft());
    deviceOffsetTitleBarTopLeftToIconTopLeft = topLeftPaddedButtonDeviceGeometry - titleBarTopLeftDeviceGeometry;

    painter->translate(geometry().topLeft());

    // translate from icon offset -- translates to the edge of smallButtonPaddedWidth
    painter->translate(m_iconOffset);
    deviceOffsetTitleBarTopLeftToIconTopLeft += (m_iconOffset * painter->device()->devicePixelRatioF());

    const qreal smallButtonPaddedWidth(m_smallButtonPaddedSize.width());
    qreal iconWidth(m_iconSize.width());
    if (d->buttonBackgroundType() == ButtonBackgroundType::Small || isStandAlone() || m_isGtkCsdButton)
        paintSmallSizedButtonBackground(painter);

    // translate to draw icon in the centre of smallButtonPaddedWidth (smallButtonPaddedWidth has additional padding)
    qreal iconTranslationOffset = (smallButtonPaddedWidth - iconWidth) / 2;
    painter->translate(iconTranslationOffset, iconTranslationOffset);
    deviceOffsetTitleBarTopLeftToIconTopLeft += (QPointF(iconTranslationOffset, iconTranslationOffset) * painter->device()->devicePixelRatioF());

    qreal scaleFactor = 1;
    if (!m_renderSystemIcon) {
        scaleFactor = iconWidth / 18;
        /*
        scale painter so that all further rendering is preformed inside QRect( 0, 0, 18, 18 )
        */
        painter->scale(scaleFactor, scaleFactor);
    }

    // render mark
    if (m_foregroundColor.isValid()) {
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
            RenderStyleSystemIconTheme iconRenderer(painter, iconWidth, systemIconName, d->internalSettings(), m_devicePixelRatio);
            iconRenderer.renderIcon();
        } else {
            std::unique_ptr<RenderDecorationButtonIcon18By18> iconRenderer =
                RenderDecorationButtonIcon18By18::factory(d->internalSettings(),
                                                          painter,
                                                          false,
                                                          m_boldButtonIcons,
                                                          m_devicePixelRatio,
                                                          deviceOffsetTitleBarTopLeftToIconTopLeft);

            switch (type()) {
            case DecorationButtonType::Close:
                iconRenderer->renderCloseIcon();
                break;

            case DecorationButtonType::Maximize:
                if (isChecked()) {
                    iconRenderer->renderRestoreIcon();
                } else {
                    iconRenderer->renderMaximizeIcon();
                }
                break;

            case DecorationButtonType::Minimize:
                iconRenderer->renderMinimizeIcon();
                break;

            case DecorationButtonType::OnAllDesktops:
                if (isChecked()) {
                    iconRenderer->renderPinnedOnAllDesktopsIcon();
                } else {
                    iconRenderer->renderPinOnAllDesktopsIcon();
                }
                break;

            case DecorationButtonType::Shade:
                if (isChecked()) {
                    iconRenderer->renderUnShadeIcon();
                } else {
                    iconRenderer->renderShadeIcon();
                }
                break;

            case DecorationButtonType::KeepBelow:
                iconRenderer->renderKeepBehindIcon();
                break;

            case DecorationButtonType::KeepAbove:
                iconRenderer->renderKeepInFrontIcon();
                break;

            case DecorationButtonType::ApplicationMenu:
                iconRenderer->renderApplicationMenuIcon();
                break;

            case DecorationButtonType::ContextHelp:
                iconRenderer->renderContextHelpIcon();
                break;

            default:
                break;
            }
        }
    }
}

//__________________________________________________________________
QColor Button::foregroundColor(const QColor &backgroundContrastedColor, const bool negativeBackgroundColor) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return QColor();

    enum struct ForegroundColorState { none, normal, animatedHover, hover, focus };
    ForegroundColorState foregroundColorState(ForegroundColorState::none);
    QColor normalForeground;
    QColor hoverForeground;
    QColor focusForeground;
    QColor fontColorContrastBoosted;

    if (backgroundContrastedColor.isValid() && d->internalSettings()->blackWhiteIconOnPoorContrast()) {
        ColorTools::getHigherContrastForegroundColor(d->fontColor(), backgroundContrastedColor, 2.3, fontColorContrastBoosted);

        // if(adjustedForContrastForeground) qDebug() << "adjustedForContrastForeground" << adjustedForContrastForeground << "for type" << int(type()) <<
        // "original foreground colour was" << d->fontColor() << "background is" << backgroundContrastedColor <<  "now foreground is" <<
        // fontColorContrastBoosted;
    } else {
        fontColorContrastBoosted = d->fontColor();
    }

    // determine the button colour state
    if (isPressed() && d->m_buttonBehaviouralParameters.drawIconOnFocus) {
        foregroundColorState = ForegroundColorState::focus;
    } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade)
               && isChecked()) {
        foregroundColorState = ForegroundColorState::focus;
    } else if (type() == DecorationButtonType::OnAllDesktops && isChecked()) {
        if ((d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarText
             || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)
            && !d->internalSettings()->translucentButtonBackgrounds() && !isHovered())
            foregroundColorState = ForegroundColorState::normal;
        else
            foregroundColorState = ForegroundColorState::focus;
    } else if (isPressed() && d->m_buttonBehaviouralParameters.drawIconOnFocus) {
        foregroundColorState = ForegroundColorState::focus;
    } else if (m_animation->state() == QAbstractAnimation::Running && d->m_buttonBehaviouralParameters.drawIconOnHover) {
        foregroundColorState = ForegroundColorState::animatedHover;
    } else if (isHovered() && d->m_buttonBehaviouralParameters.drawIconOnHover) {
        foregroundColorState = ForegroundColorState::hover;
    } else if (d->m_buttonBehaviouralParameters.drawIconAlways) {
        foregroundColorState = ForegroundColorState::normal;
    }

    // get the colour palette to use
    if (type() == DecorationButtonType::Close && negativeBackgroundColor
        && d->internalSettings()->closeIconNegativeBackground() != InternalSettings::EnumCloseIconNegativeBackground::Same) {
        if (d->internalSettings()->closeIconNegativeBackground() == InternalSettings::EnumCloseIconNegativeBackground::White) {
            normalForeground = Qt::GlobalColor::white;
            hoverForeground = Qt::GlobalColor::white;
            focusForeground = Qt::GlobalColor::white;
        } else { // White on hovered pressed
            hoverForeground = Qt::GlobalColor::white;
            focusForeground = Qt::GlobalColor::white;

            // get normalForeground
            if ((d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarText
                 || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)
                && !d->internalSettings()->translucentButtonBackgrounds()) {
                if (d->m_buttonBehaviouralParameters.drawCloseBackgroundAlways) {
                    normalForeground = d->titleBarColor();
                } else {
                    normalForeground = d->fontColor();
                }
            } else {
                normalForeground = fontColorContrastBoosted;
            }
        }
    } else if (type() == DecorationButtonType::Close) {
        if ((d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarText
             || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)
            && !d->internalSettings()->translucentButtonBackgrounds()) {
            if (d->m_buttonBehaviouralParameters.drawCloseBackgroundAlways) {
                normalForeground = d->titleBarColor();
                hoverForeground = d->titleBarColor();
                focusForeground = d->titleBarColor();
            } else {
                normalForeground = d->fontColor();
                hoverForeground = d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover ? d->titleBarColor() : d->fontColor();
                focusForeground = d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus ? d->titleBarColor() : d->fontColor();
            }
        } else {
            normalForeground = fontColorContrastBoosted;
            hoverForeground = fontColorContrastBoosted;
            focusForeground = fontColorContrastBoosted;
        }
    } else { // non-close button
        if ((d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarText
             || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)
            && !d->internalSettings()->translucentButtonBackgrounds()) {
            if (d->m_buttonBehaviouralParameters.drawBackgroundAlways) {
                normalForeground = d->titleBarColor();
                hoverForeground = d->titleBarColor();
                focusForeground = d->titleBarColor();
            } else {
                normalForeground = d->fontColor();
                hoverForeground = d->m_buttonBehaviouralParameters.drawBackgroundOnHover ? d->titleBarColor() : d->fontColor();
                focusForeground = d->m_buttonBehaviouralParameters.drawBackgroundOnFocus ? d->titleBarColor() : d->fontColor();
            }
        } else {
            normalForeground = fontColorContrastBoosted;
            hoverForeground = fontColorContrastBoosted;
            focusForeground = fontColorContrastBoosted;
        }
    }

    // return the appropriate palette colour for each state
    switch (foregroundColorState) {
    case ForegroundColorState::normal:
        return normalForeground;
    case ForegroundColorState::animatedHover:
        return m_preAnimationForegroundColor.isValid() ? KColorUtils::mix(m_preAnimationForegroundColor, hoverForeground, m_opacity)
                                                       : ColorTools::alphaMix(hoverForeground, m_opacity);
    case ForegroundColorState::hover:
        return hoverForeground;
    case ForegroundColorState::focus:
        return focusForeground;
    case ForegroundColorState::none:
    default:
        return QColor();
    }
}

//__________________________________________________________________
QColor Button::backgroundColor(const bool getNonAnimatedColor) const
{
    QColor color;
    bool neg;
    return backgroundColor(color, neg, getNonAnimatedColor);
}

QColor Button::backgroundColor(QColor &foregroundContrastedColor, bool &negativeClosePalette, bool getNonAnimatedColor) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return QColor();
    }

    QColor buttonNormalColor;
    QColor buttonHoverColor;
    QColor buttonFocusColor;
    foregroundContrastedColor = QColor(); // for contrast heuristic between foreground and background of button when not translucent

    // heuristic for contrast detection between background and foreground is only enabled for system accent colours from the system because system colour
    // schemes can be imperfect. Not enabled for translucent because usually the translucencty alleviates any contrast problems
    bool analyseContrastWithForeground =
        (!d->internalSettings()->translucentButtonBackgrounds()
         && (d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::Accent
             || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
             || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights));

    const bool negativeCloseCategory( // whether the button background colour is in the general negative category as selected in the "Background & Outline
                                      // Colours" combobox
        d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
        || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights);

    bool negativeNormalClosePalette(false);
    bool negativeHoverClosePalette(false);
    bool negativeFocusClosePalette(false);

    bool defaultButton =
        true; // flag indicates the button has standard colours for the behaviour and selected colour (i.e. is not a close/max/min with special colours)
    bool *drawButtonAlwaysParameter = (type() == DecorationButtonType::Close) ? &d->m_buttonBehaviouralParameters.drawCloseBackgroundAlways
                                                                              : &d->m_buttonBehaviouralParameters.drawBackgroundAlways;
    bool *drawButtonOnHoverParameter = (type() == DecorationButtonType::Close) ? &d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover
                                                                               : &d->m_buttonBehaviouralParameters.drawBackgroundOnHover;
    bool *drawButtonOnFocusParameter = (type() == DecorationButtonType::Close) ? &d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus
                                                                               : &d->m_buttonBehaviouralParameters.drawBackgroundOnFocus;

    negativeClosePalette =
        false; // whether the button in this specific button state is negative (factoring "Negative close background on hover/press only" checkBox)

    // set normal, hover and focus colours
    if (d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::Accent
        || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        if (d->internalSettings()->translucentButtonBackgrounds()) {
            if (type() == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawCloseBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        negativeNormalClosePalette = true;
                        buttonNormalColor = d->decorationColors()->negativeReducedOpacityBackground();
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->fullySaturatedNegative();
                        }
                    } else {
                        buttonNormalColor = d->decorationColors()->buttonReducedOpacityBackground();
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->fullySaturatedNegative();
                        }
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                        negativeHoverClosePalette = true;
                        buttonHoverColor = d->decorationColors()->negativeReducedOpacityBackground();
                    }
                    if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                        negativeFocusClosePalette = true;
                        buttonFocusColor = d->decorationColors()->negativeReducedOpacityOutline();
                    }
                }
            } else if (type() == DecorationButtonType::Minimize
                       && d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonNormalColor = d->decorationColors()->neutralReducedOpacityBackground();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                            buttonHoverColor = d->decorationColors()->neutralReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                            buttonFocusColor = d->decorationColors()->neutral();
                    } else {
                        buttonNormalColor = d->decorationColors()->buttonReducedOpacityBackground();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                            buttonHoverColor = d->decorationColors()->neutralReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                            buttonFocusColor = d->decorationColors()->neutral();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                        buttonHoverColor = d->decorationColors()->neutralReducedOpacityBackground();
                    if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                        buttonFocusColor = d->decorationColors()->neutralReducedOpacityOutline();
                }
            } else if (type() == DecorationButtonType::Maximize
                       && d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonNormalColor = d->decorationColors()->positiveReducedOpacityBackground();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                            buttonHoverColor = d->decorationColors()->positiveReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                            buttonFocusColor = d->decorationColors()->positive();
                    } else {
                        buttonNormalColor = d->decorationColors()->buttonReducedOpacityBackground();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                            buttonHoverColor = d->decorationColors()->positiveReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                            buttonFocusColor = d->decorationColors()->positive();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                        buttonHoverColor = d->decorationColors()->positiveReducedOpacityBackground();
                    if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                        buttonFocusColor = d->decorationColors()->positiveReducedOpacityOutline();
                }
            }

            if (defaultButton) {
                if (*drawButtonAlwaysParameter) {
                    buttonNormalColor = d->decorationColors()->buttonReducedOpacityBackground();
                    if (*drawButtonOnHoverParameter)
                        buttonHoverColor = d->decorationColors()->buttonReducedOpacityOutline();
                    if (*drawButtonOnFocusParameter)
                        buttonFocusColor = d->decorationColors()->buttonFocus();
                } else {
                    if (*drawButtonOnHoverParameter)
                        buttonHoverColor = d->decorationColors()->buttonReducedOpacityBackground();
                    if (*drawButtonOnFocusParameter)
                        buttonFocusColor = d->decorationColors()->buttonReducedOpacityOutline();
                }
            }
        } else { // accent but not translucent
            if (type() == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawCloseBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        negativeNormalClosePalette = true;
                        buttonNormalColor = d->decorationColors()->negative();
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeSaturated();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->negativeLessSaturated();
                        }
                    } else {
                        buttonNormalColor = d->m_buttonBehaviouralParameters.drawBackgroundAlways
                            ? KColorUtils::mix(d->titleBarColor(), d->decorationColors()->buttonHover(), 0.8)
                            : d->decorationColors()->buttonHover();
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeSaturated();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->negativeLessSaturated();
                        }
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                        negativeHoverClosePalette = true;
                        buttonHoverColor = d->decorationColors()->negative();
                    }
                    if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                        negativeFocusClosePalette = true;
                        buttonFocusColor = d->decorationColors()->negativeSaturated();
                    }
                }
            } else if (type() == DecorationButtonType::Minimize
                       && d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonNormalColor = d->decorationColors()->neutralLessSaturated();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                            buttonHoverColor = d->decorationColors()->neutral();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                            buttonFocusColor = d->decorationColors()->neutralSaturated();
                    } else {
                        buttonNormalColor = KColorUtils::mix(d->titleBarColor(), d->decorationColors()->buttonHover(), 0.8);
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                            buttonHoverColor = d->decorationColors()->neutral();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                            buttonFocusColor = d->decorationColors()->neutralSaturated();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                        buttonHoverColor = d->decorationColors()->neutralLessSaturated();
                    if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                        buttonFocusColor = d->decorationColors()->neutral();
                }
            } else if (type() == DecorationButtonType::Maximize
                       && d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonNormalColor = d->decorationColors()->positiveLessSaturated();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                            buttonHoverColor = d->decorationColors()->positive();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                            buttonFocusColor = d->decorationColors()->positiveSaturated();
                    } else {
                        buttonNormalColor = KColorUtils::mix(d->titleBarColor(), d->decorationColors()->buttonHover(), 0.8);
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                            buttonHoverColor = d->decorationColors()->positive();
                        if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                            buttonFocusColor = d->decorationColors()->positiveSaturated();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawBackgroundOnHover)
                        buttonHoverColor = d->decorationColors()->positiveLessSaturated();
                    if (d->m_buttonBehaviouralParameters.drawBackgroundOnFocus)
                        buttonFocusColor = d->decorationColors()->positive();
                }
            }

            if (defaultButton) {
                if (*drawButtonAlwaysParameter) {
                    buttonNormalColor = KColorUtils::mix(d->titleBarColor(), d->decorationColors()->buttonHover(), 0.8);
                    if (*drawButtonOnHoverParameter)
                        buttonHoverColor = d->decorationColors()->buttonHover();
                    if (*drawButtonOnFocusParameter)
                        buttonFocusColor = d->decorationColors()->buttonFocus();
                } else {
                    if (*drawButtonOnHoverParameter)
                        buttonHoverColor = d->decorationColors()->buttonHover();
                    if (*drawButtonOnFocusParameter)
                        buttonFocusColor = d->decorationColors()->buttonFocus();
                }
            }
        }

    } else {
        if (d->internalSettings()->translucentButtonBackgrounds()) { // titlebar text color, translucent
            if (type() == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawCloseBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        negativeNormalClosePalette = true;
                        buttonNormalColor = d->decorationColors()->negativeReducedOpacityBackground();
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->negativeReducedOpacityLessSaturatedBackground();
                        }
                    } else {
                        buttonNormalColor = ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.3, 1.0));
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->negativeReducedOpacityBackground();
                        }
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                        negativeHoverClosePalette = true;
                        buttonHoverColor = d->decorationColors()->negativeReducedOpacityBackground();
                    }
                    if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                        negativeFocusClosePalette = true;
                        buttonFocusColor = d->decorationColors()->negativeReducedOpacityOutline();
                    }
                }
            }

            if (defaultButton) {
                if (*drawButtonAlwaysParameter) {
                    buttonNormalColor = ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.3, 1.0));
                    if (*drawButtonOnHoverParameter)
                        buttonHoverColor = ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                    if (*drawButtonOnFocusParameter)
                        buttonFocusColor = ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.7, 1.0));
                } else {
                    if (*drawButtonOnHoverParameter)
                        buttonHoverColor = ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.3, 1.0));
                    if (*drawButtonOnFocusParameter)
                        buttonFocusColor = ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                }
            }
        } else { // titlebar text color, not translucent
            if (type() == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        negativeNormalClosePalette = true;
                        buttonNormalColor = d->decorationColors()->negative();
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeSaturated();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->negativeLessSaturated();
                        }
                    } else {
                        buttonNormalColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeSaturated();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->negativeLessSaturated();
                        }
                    }
                } else if (d->m_buttonBehaviouralParameters.drawCloseBackgroundAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        negativeNormalClosePalette = true;
                        buttonNormalColor = d->decorationColors()->negative();
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeSaturated();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->negativeLessSaturated();
                        }
                    } else {
                        buttonNormalColor = d->fontColor();
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                            negativeHoverClosePalette = true;
                            buttonHoverColor = d->decorationColors()->negativeSaturated();
                        }
                        if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                            negativeFocusClosePalette = true;
                            buttonFocusColor = d->decorationColors()->negativeLessSaturated();
                        }
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnHover) {
                        ;
                        negativeHoverClosePalette = true;
                        buttonHoverColor = d->decorationColors()->negative();
                    }
                    if (d->m_buttonBehaviouralParameters.drawCloseBackgroundOnFocus) {
                        negativeFocusClosePalette = true;
                        buttonFocusColor = d->decorationColors()->negativeSaturated();
                    }
                }
            }

            if (defaultButton) {
                if (*drawButtonAlwaysParameter) {
                    buttonNormalColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                    if (*drawButtonOnHoverParameter)
                        buttonHoverColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.6);
                    if (*drawButtonOnFocusParameter)
                        buttonFocusColor = d->fontColor();
                } else {
                    if (*drawButtonOnHoverParameter)
                        buttonHoverColor = d->fontColor();
                    if (*drawButtonOnFocusParameter)
                        buttonFocusColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                }
            }
        }
    }

    // low contrast correction between background and titlebar
    if (d->internalSettings()->adjustBackgroundColorOnPoorContrast()) {
        if (buttonNormalColor.isValid() && KColorUtils::contrastRatio(buttonNormalColor, d->titleBarColor()) < 1.3) {
            buttonNormalColor = KColorUtils::mix(buttonNormalColor, d->fontColor(), 0.3);
        }

        if (buttonHoverColor.isValid() && KColorUtils::contrastRatio(buttonHoverColor, d->titleBarColor()) < 1.3) {
            buttonHoverColor = KColorUtils::mix(buttonHoverColor, d->fontColor(), 0.3);
        }
        if (buttonFocusColor.isValid() && KColorUtils::contrastRatio(buttonFocusColor, d->titleBarColor()) < 1.3) {
            buttonFocusColor = KColorUtils::mix(buttonFocusColor, d->fontColor(), 0.3);
        }
    }

    // return a variant of normal, hover and focus colours, depending on state
    if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade) && isChecked()) {
        if (d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::Accent
            || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
            || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
            if (analyseContrastWithForeground)
                foregroundContrastedColor = buttonFocusColor;
            return buttonFocusColor;
        } else {
            if (analyseContrastWithForeground)
                foregroundContrastedColor = buttonHoverColor;
            return buttonHoverColor;
        }
    } else if (type() == DecorationButtonType::OnAllDesktops && isChecked()
               && !( // not when titlebar-text inversion occurs (for Breeze compatibility with circular pin icon)
                   !d->m_buttonBehaviouralParameters.drawBackgroundAlways
                   && ((d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarText
                        && !d->internalSettings()->translucentButtonBackgrounds())
                       || (d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
                           && !d->internalSettings()->translucentButtonBackgrounds())))) {
        if (analyseContrastWithForeground)
            foregroundContrastedColor = buttonFocusColor;
        return buttonFocusColor;
    } else if (isPressed()) {
        if (analyseContrastWithForeground)
            foregroundContrastedColor = buttonFocusColor;
        negativeClosePalette = negativeFocusClosePalette;
        return buttonFocusColor;
    } else if (m_animation->state() == QAbstractAnimation::Running && !getNonAnimatedColor) {
        if (m_preAnimationBackgroundColor.isValid() && buttonHoverColor.isValid()) {
            if (analyseContrastWithForeground)
                foregroundContrastedColor = buttonHoverColor;
            negativeClosePalette = negativeHoverClosePalette;
            return KColorUtils::mix(m_preAnimationBackgroundColor, buttonHoverColor, m_opacity);
        } else if (buttonHoverColor.isValid()) {
            if (analyseContrastWithForeground)
                foregroundContrastedColor = buttonHoverColor;
            negativeClosePalette = negativeHoverClosePalette;
            return ColorTools::alphaMix(buttonHoverColor, m_opacity);
        } else
            return QColor();
    } else if (isHovered()) {
        if (analyseContrastWithForeground)
            foregroundContrastedColor = buttonHoverColor;
        negativeClosePalette = negativeHoverClosePalette;
        return buttonHoverColor;
    } else {
        if (analyseContrastWithForeground)
            foregroundContrastedColor = buttonNormalColor;
        negativeClosePalette = negativeNormalClosePalette;
        return buttonNormalColor;
    }
}

// Returns a colour if an outline is to be drawn around the button
QColor Button::outlineColor(bool getNonAnimatedColor) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return QColor();

    QColor buttonOutlineNormalColor;
    QColor buttonOutlineHoverColor;
    QColor buttonOutlineFocusColor;

    const bool negativeClose(d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
                             || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
                             || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights);

    bool defaultButton =
        true; // flag indicates the button has standard colours for the behaviour and selected colour (i.e. is not a close/max/min with special colours)
    bool *drawOutlineAlwaysParameter = (type() == DecorationButtonType::Close) ? &d->m_buttonBehaviouralParameters.drawCloseOutlineAlways
                                                                               : &d->m_buttonBehaviouralParameters.drawOutlineAlways;
    bool *drawOutlineOnHoverParameter = (type() == DecorationButtonType::Close) ? &d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover
                                                                                : &d->m_buttonBehaviouralParameters.drawOutlineOnHover;
    bool *drawOutlineOnFocusParameter = (type() == DecorationButtonType::Close) ? &d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus
                                                                                : &d->m_buttonBehaviouralParameters.drawOutlineOnFocus;

    // set normal, hover and focus colours
    if (d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::Accent
        || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        if (d->internalSettings()->translucentButtonBackgrounds()) {
            if (type() == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawCloseOutlineAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonOutlineNormalColor =
                            d->decorationColors()->negativeReducedOpacityOutline(); // may want to change these to be distinct colours in the future
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->negativeReducedOpacityOutline();
                    } else {
                        buttonOutlineNormalColor = d->decorationColors()->buttonReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->negativeReducedOpacityOutline();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                        buttonOutlineHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                    if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                        buttonOutlineFocusColor = d->decorationColors()->negativeReducedOpacityOutline();
                }
            } else if (type() == DecorationButtonType::Minimize
                       && d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawOutlineAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonOutlineNormalColor = d->decorationColors()->neutralReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->neutralReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->neutralReducedOpacityOutline();
                    } else {
                        buttonOutlineNormalColor = d->decorationColors()->buttonReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->neutralReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->neutralReducedOpacityOutline();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                        buttonOutlineHoverColor = d->decorationColors()->neutralReducedOpacityOutline();
                    if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                        buttonOutlineFocusColor = d->decorationColors()->neutralReducedOpacityOutline();
                }
            } else if (type() == DecorationButtonType::Maximize
                       && d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawOutlineAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonOutlineNormalColor = d->decorationColors()->positiveReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->positiveReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->positiveReducedOpacityOutline();
                    } else {
                        buttonOutlineNormalColor = d->decorationColors()->buttonReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->positiveReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->positiveReducedOpacityOutline();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                        buttonOutlineHoverColor = d->decorationColors()->positiveReducedOpacityOutline();
                    if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                        buttonOutlineFocusColor = d->decorationColors()->positiveReducedOpacityOutline();
                }
            }

            if (defaultButton) {
                if (*drawOutlineAlwaysParameter) {
                    buttonOutlineNormalColor = d->decorationColors()->buttonReducedOpacityOutline();
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineHoverColor = d->decorationColors()->buttonReducedOpacityOutline();
                    if (*drawOutlineOnFocusParameter)
                        buttonOutlineFocusColor = d->decorationColors()->buttonReducedOpacityOutline();
                } else {
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineHoverColor = d->decorationColors()->buttonReducedOpacityOutline();
                    if (*drawOutlineOnFocusParameter)
                        buttonOutlineFocusColor = d->decorationColors()->buttonReducedOpacityOutline();
                }
            }
        } else { // non-translucent accent colours
            if (type() == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawCloseOutlineAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonOutlineNormalColor = d->decorationColors()->negativeSaturated();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->negativeSaturated();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->negativeSaturated();
                    } else {
                        buttonOutlineNormalColor = d->decorationColors()->buttonFocus();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->negativeSaturated();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->negativeSaturated();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                        buttonOutlineHoverColor = d->decorationColors()->negativeSaturated();
                    if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                        buttonOutlineFocusColor = d->decorationColors()->negativeSaturated();
                }
            } else if (type() == DecorationButtonType::Minimize
                       && d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawOutlineAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonOutlineNormalColor = d->decorationColors()->neutral();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->neutral();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->neutral();
                    } else {
                        buttonOutlineNormalColor = d->decorationColors()->buttonFocus();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->neutral();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->neutral();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                        buttonOutlineHoverColor = d->decorationColors()->neutral();
                    if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                        buttonOutlineFocusColor = d->decorationColors()->neutral();
                }
            } else if (type() == DecorationButtonType::Maximize
                       && d->internalSettings()->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawOutlineAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonOutlineNormalColor = d->decorationColors()->positive();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->positive();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->positive();
                    } else {
                        buttonOutlineNormalColor = d->decorationColors()->buttonFocus();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->positive();
                        if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->positive();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawOutlineOnHover)
                        buttonOutlineHoverColor = d->decorationColors()->positive();
                    if (d->m_buttonBehaviouralParameters.drawOutlineOnFocus)
                        buttonOutlineFocusColor = d->decorationColors()->positive();
                }
            }

            if (defaultButton) {
                if (*drawOutlineAlwaysParameter) {
                    buttonOutlineNormalColor = d->decorationColors()->buttonFocus();
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineHoverColor = d->decorationColors()->buttonFocus();
                    if (*drawOutlineOnFocusParameter)
                        buttonOutlineFocusColor = d->decorationColors()->buttonFocus();
                } else {
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineHoverColor = d->decorationColors()->buttonFocus();
                    if (*drawOutlineOnFocusParameter)
                        buttonOutlineFocusColor = d->decorationColors()->buttonFocus();
                }
            }
        }

    } else { // titlebar text colour, translucent
        if (d->internalSettings()->translucentButtonBackgrounds()) {
            if (type() == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawCloseOutlineAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonOutlineNormalColor = d->decorationColors()->negativeReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->negativeReducedOpacityOutline();
                    } else {
                        buttonOutlineNormalColor =
                            ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->negativeReducedOpacityOutline();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                        buttonOutlineHoverColor = d->decorationColors()->negativeReducedOpacityOutline();
                    if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                        buttonOutlineFocusColor = d->decorationColors()->negativeReducedOpacityOutline();
                }
            }
            if (defaultButton) {
                if (*drawOutlineAlwaysParameter) {
                    buttonOutlineNormalColor =
                        ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineHoverColor =
                            ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                    if (*drawOutlineOnFocusParameter)
                        buttonOutlineFocusColor =
                            ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                } else {
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineHoverColor =
                            ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                    if (*drawOutlineOnFocusParameter)
                        buttonOutlineFocusColor =
                            ColorTools::alphaMix(d->fontColor(), qMin(d->internalSettings()->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                }
            }
        } else { // titlebar text colour, non-translucent
            if (type() == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (d->m_buttonBehaviouralParameters.drawCloseOutlineAlways) {
                    if (!d->internalSettings()->negativeCloseBackgroundHoverPress()) {
                        buttonOutlineNormalColor = d->decorationColors()->negativeSaturated();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->negativeSaturated();
                        ;
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->negativeSaturated();
                    } else {
                        buttonOutlineNormalColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                            buttonOutlineHoverColor = d->decorationColors()->negativeSaturated();
                        if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                            buttonOutlineFocusColor = d->decorationColors()->negativeSaturated();
                    }
                } else {
                    if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnHover)
                        buttonOutlineHoverColor = d->decorationColors()->negativeSaturated();
                    if (d->m_buttonBehaviouralParameters.drawCloseOutlineOnFocus)
                        buttonOutlineFocusColor = d->decorationColors()->negativeSaturated();
                }
            }
            if (defaultButton) {
                if (*drawOutlineAlwaysParameter) {
                    buttonOutlineNormalColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineHoverColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                    if (*drawOutlineOnFocusParameter)
                        buttonOutlineFocusColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                } else {
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineHoverColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                    if (*drawOutlineOnHoverParameter)
                        buttonOutlineFocusColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
                }
            }
        }
    }

    // low contrast correction between outline and titlebar
    if (d->internalSettings()->adjustBackgroundColorOnPoorContrast()) {
        if (buttonOutlineNormalColor.isValid() && KColorUtils::contrastRatio(buttonOutlineNormalColor, d->titleBarColor()) < 1.3) {
            buttonOutlineNormalColor = KColorUtils::mix(buttonOutlineNormalColor, d->fontColor(), 0.4);
        }

        if (buttonOutlineHoverColor.isValid() && KColorUtils::contrastRatio(buttonOutlineHoverColor, d->titleBarColor()) < 1.3) {
            buttonOutlineHoverColor = KColorUtils::mix(buttonOutlineHoverColor, d->fontColor(), 0.4);
        }
        if (buttonOutlineFocusColor.isValid() && KColorUtils::contrastRatio(buttonOutlineFocusColor, d->titleBarColor()) < 1.3) {
            buttonOutlineFocusColor = KColorUtils::mix(buttonOutlineFocusColor, d->fontColor(), 0.4);
        }
    }

    // return a variant of normal, hover and focus colours, depending on state
    if ((isChecked() && (type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade))) {
        return buttonOutlineFocusColor;
    } else if (type() == DecorationButtonType::OnAllDesktops && isChecked()) {
        return buttonOutlineFocusColor;
    } else if (isPressed()) {
        return buttonOutlineFocusColor;
    } else if (m_animation->state() == QAbstractAnimation::Running && !getNonAnimatedColor) {
        if (m_preAnimationOutlineColor.isValid() && buttonOutlineHoverColor.isValid()) {
            return KColorUtils::mix(m_preAnimationOutlineColor, buttonOutlineHoverColor, m_opacity);
        } else if (buttonOutlineHoverColor.isValid()) {
            return ColorTools::alphaMix(buttonOutlineHoverColor, m_opacity);
        } else
            return QColor();
    } else if (isHovered()) {
        return buttonOutlineHoverColor;
    } else {
        return buttonOutlineNormalColor;
    }
}

//________________________________________________________________
void Button::reconfigure()
{
    // animation
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;
    m_animation->setDuration(d->animationsDuration());

    configureSystemIcons();
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

// When "Use system icon theme" is selected for the icons then not all icons are available as a window-*-symbolic icon
// sets m_systemIconName and m_systemIconCheckedName
void Button::configureSystemIcons()
{
    m_systemIconName.clear();
    m_systemIconCheckedName.clear();

    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;
    if (d->internalSettings()->buttonIconStyle() != InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme)
        return;

    switch (type()) {
    case DecorationButtonType::Close:
        m_systemIconName = RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-close-symbolic"), QStringLiteral("window-close"));
        m_systemIconCheckedName = m_systemIconName;
        break;

    case DecorationButtonType::Maximize:
        m_systemIconCheckedName =
            RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-restore-symbolic"), QStringLiteral("window-restore"));
        m_systemIconName = RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-maximize-symbolic"), QStringLiteral("window-maximize"));
        break;

    case DecorationButtonType::Minimize:
        m_systemIconName = RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-minimize-symbolic"), QStringLiteral("window-minimize"));
        m_systemIconCheckedName = m_systemIconName;
        break;

    case DecorationButtonType::OnAllDesktops:
        m_systemIconCheckedName =
            RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-unpin-symbolic"), QStringLiteral("window-unpin"));
        m_systemIconName = RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-pin-symbolic"), QStringLiteral("window-pin"));
        break;

    case DecorationButtonType::Shade:
        m_systemIconCheckedName =
            RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-unshade-symbolic"), QStringLiteral("window-unshade"));
        m_systemIconName = RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-shade-symbolic"), QStringLiteral("window-shade"));
        break;

    case DecorationButtonType::KeepBelow:
        m_systemIconName =
            RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-keep-below-symbolic"), QStringLiteral("window-keep-below"));
        m_systemIconCheckedName = m_systemIconName;
        break;

    case DecorationButtonType::KeepAbove:
        m_systemIconName =
            RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("window-keep-above-symbolic"), QStringLiteral("window-keep-above"));
        m_systemIconCheckedName = m_systemIconName;
        break;

    case DecorationButtonType::ApplicationMenu:
        m_systemIconName =
            RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("application-menu-symbolic"), QStringLiteral("application-menu"));
        m_systemIconCheckedName = m_systemIconName;
        break;

    case DecorationButtonType::ContextHelp:
        m_systemIconName = RenderStyleSystemIconTheme::isSystemIconNameAvailable(QStringLiteral("help-contextual-symbolic"), QStringLiteral("help-contextual"));
        m_systemIconCheckedName = m_systemIconName;
        break;

    default:
        break;
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
