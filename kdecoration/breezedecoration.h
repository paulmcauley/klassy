/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021-2025 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breeze.h"

#include "breezesettings.h"
#include "colortools.h"
#include "decorationcolors.h"

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationSettings>
#include <KSharedConfig>

#include <QPainterPath>
#include <QPalette>
#include <QVariant>
#include <QVariantAnimation>

#include <memory>

namespace KDecoration3
{
class DecorationButton;
class DecorationButtonGroup;
}

namespace Klassy
{

enum struct ButtonBackgroundType {
    Small,
    FullHeight,
};

extern Side g_taskManagerSide;
extern TaskManagerType g_taskManagerType;

class Decoration : public KDecoration3::Decoration
{
    Q_OBJECT

public:
    //* constructor
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~Decoration() override;

    //* paint
    void paint(QPainter *painter, const QRectF &repaintRegion) override;

    //* internal settings
    InternalSettingsPtr internalSettings() const
    {
        return m_internalSettings;
    }

    qreal animationsDuration() const
    {
        return m_animation->duration();
    }

    //* caption height
    qreal captionHeight() const;
    void setCaptionOpacity(qreal);
    qreal captionOpacity() const
    {
        return m_captionOpacity;
    }
    QRectF getMaxCaptionSize() const;
    //* return the rect in which caption will be drawn
    QPair<QRectF, Qt::Alignment> captionRect(bool minimumAppMenuBar) const;

    //*@name active state change animation
    //@{
    void setOpacity(qreal);

    qreal opacity() const
    {
        return m_opacity;
    }
    //@}

    //*@name colors
    //@{
    DecorationColors *decorationColors()
    {
        return m_decorationColors.get();
    }

    QColor titleBarColor(bool returnNonAnimatedColor = false) const;
    QColor titleBarSeparatorColor() const;
    QColor fontColor(bool returnNonAnimatedColor = false) const;
    QColor overriddenOutlineColorAnimateIn() const;
    QColor overriddenOutlineColorAnimateOut(const QColor &destinationColor);
    //@}

    //*@name TitleBarAppMenuBar
    //@{
    void updateAppMenuBar();
    qreal titleBarHeight() const;
    QPoint windowPos() const;
    //@}

    //
    //*@name maximization modes
    //@{
    inline bool isMaximized() const;
    inline bool isMaximizedHorizontally() const;
    inline bool isMaximizedVertically() const;

    inline bool isLeftEdge() const;
    inline bool isRightEdge() const;
    inline bool isTopEdge() const;
    inline bool isBottomEdge() const;

    inline bool hideTitleBar() const;
    //@}

    void setWindowOutlineOverrideColor(const bool on, const QColor &color);

    QPainterPath *titleBarPath()
    {
        return &m_titleBarPath;
    }
    QPainterPath *windowPath()
    {
        return &m_windowPath;
    }
    qreal systemScaleFactorX11() const
    {
        return m_systemScaleFactorX11;
    }
    qreal scaledTitleBarRightMargin(const bool nextScale) const
    {
        if (isMaximizedHorizontally()) {
            return nextScale ? m_scaledTitleBarRightMarginMaximizedHorizontallyNext : m_scaledTitleBarRightMarginMaximizedHorizontally;
        } else {
            return nextScale ? m_scaledTitleBarRightMarginNext : m_scaledTitleBarRightMargin;
        }
    }

    qreal scaledTitleBarLeftMargin(const bool nextScale) const
    {
        if (isMaximizedHorizontally()) {
            return nextScale ? m_scaledTitleBarLeftMarginMaximizedHorizontallyNext : m_scaledTitleBarLeftMarginMaximizedHorizontally;
        } else {
            return nextScale ? m_scaledTitleBarLeftMarginNext : m_scaledTitleBarLeftMargin;
        }
    }
    ButtonBackgroundType buttonBackgroundType()
    {
        return m_buttonBackgroundType;
    }
    qreal scaledSmallButtonPaddedSize()
    {
        return m_scaledSmallButtonPaddedSize;
    }
    qreal scaledIconSize()
    {
        return m_scaledIconSize;
    }
    qreal scaledSmallButtonBackgroundSize()
    {
        return m_scaledSmallButtonBackgroundSize;
    }
    qreal scaledCornerRadius()
    {
        return m_scaledCornerRadius;
    }

    qreal scaledButtonSpacingLeft()
    {
        return m_scaledButtonSpacingLeft;
    }

    qreal scaledButtonSpacingRight()
    {
        return m_scaledButtonSpacingRight;
    }

    qreal smallSpacing()
    {
        return m_smallSpacing;
    }

    qreal x11Scale()
    {
        return m_x11Scale;
    }

    bool isRightToLeft()
    {
        return m_isRightToLeft;
    }

    KDecoration3::DecorationButtonGroup *leftButtons()
    {
        return m_leftButtons;
    }

    KDecoration3::DecorationButtonGroup *rightButtons()
    {
        return m_rightButtons;
    }

    QVariantAnimation *activeStateChangeAnimation()
    {
        return m_animation;
    }

    qreal activeStateChangeAnimationOpacity()
    {
        return m_opacity;
    }

    bool buttonUnisonHovered() const // for unison hovering
    {
        return m_buttonUnisonHovered;
    }

    void scaledTitleBarTopBottomMargins(qreal scale,
                                        qreal &scaledTitleBarTopMargin,
                                        qreal &scaledTitleBarBottomMargin,
                                        qreal &scaledIntegratedRoundedRectangleBottomPadding) const;
    qreal titleBarSeparatorHeight(qreal scale) const;

Q_SIGNALS:
    void reconfigured();
    void buttonUnisonHoveredChanged(bool); // for unison hovering

public Q_SLOTS:
    bool init() override;
    void setButtonUnisonHovered(bool value);

private Q_SLOTS:
    void reconfigure()
    {
        reconfigureMain(false);
    }
    void reconfigureWithNoShadowUpdate()
    {
        reconfigureMain(true);
    }
    void generateDecorationColorsOnClientPaletteUpdate(const QPalette &clientPalette);
    void generateDecorationColorsOnDecorationColorSettingsUpdate(QByteArray uuid);
    void generateDecorationColorsOnSystemColorSettingsUpdate(QByteArray uuid);
    void updateTaskManagerTypeAndSide(QByteArray uuid = QByteArray());
    void recalculateBorders();
    void updateOpaque();
    void updateBlur();
    void updateButtonsGeometry();
    void updateButtonsGeometryDelayed();
    void updateTitleBar();
    void updateAnimationState();
    void updateShadowOnChange()
    {
        updateShadow();
    }
    void updateShadowOnChangeNoCache()
    {
        updateShadow(false, true);
    }
    void updateScale();
    void updateNextScale();

protected:
    void hoverMoveEvent(QHoverEvent *event) override; // override decoration hover events for Unison hovering
    void hoverLeaveEvent(QHoverEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void reconfigureMain(const bool noUpdateShadow = false);
    void updateDecorationColors(const QPalette &clientPalette, QByteArray uuid = "");
    void createButtons();
    void calculateWindowShape();
    void calculateTitleBarShape();
    void paintTitleBar(QPainter *painter, const QRectF &repaintRegion);
    void updateShadow(const bool forceUpdateCache = false, bool noCache = false);
    std::shared_ptr<KDecoration3::DecorationShadow> createShadowObject(QColor shadowColor);
    void updateWindowOutline(bool override = false);
    void setScaledCornerRadius();
    void setScaledTitleBarMargins(const bool nextScale);

    //* icon + padding sizes
    void setScaledIconSizes(const bool nextScale);

    void setScaledButtonDimensions();

    //*@name border size
    //@{
    void setScaledBorderSizes(const bool nextScale);
    inline bool hasBorders() const;
    inline bool hasNoBorders() const;
    inline bool hasNoSideBorders() const;

    inline bool windowOutlineNone() const;
    //@}

    bool isOpaqueTitleBar();
    qreal scaledTitleBarSeparatorHeight(const bool nextScale) const;
    qreal devicePixelRatio(QPainter *painter) const;

    //* override thin window outline colour from button colour animation update
    void updateOverrideWindowOutlineFromButtonAnimationState();

    //* calculates and sets m_windowOutline
    void setWindowOutlineColor();

    void setGlobalLookAndFeelOptions(QString lookAndFeelPackageName);

    static KSharedConfig::Ptr s_kdeGlobalConfig;
    InternalSettingsPtr m_internalSettings;
    bool m_isRightToLeft = false;
    KDecoration3::DecorationButtonGroup *m_leftButtons = nullptr;
    KDecoration3::DecorationButtonGroup *m_rightButtons = nullptr;
    class AppMenuButtonGroup *m_appMenuBarButtons = nullptr;

    TaskManagerType m_taskManagerType = TaskManagerType::IconsAndTextTaskManager;
    Side m_taskManagerSide = SideBottom;

    //* Object to return decoration palette colours
    std::unique_ptr<DecorationColors> m_decorationColors;

    //* active state change animation
    QVariantAnimation *m_animation;
    //* shadow animation
    QVariantAnimation *m_shadowAnimation;
    //*window outline animation when "Colourize with highlighted button'a colour ticked"
    QVariantAnimation *m_overrideWindowOutlineFromButtonAnimation;

    //* active state change animation opacity
    qreal m_opacity = 0;
    //* shadow change animation opacity
    qreal m_shadowOpacity = 0;
    //* overridden thin window outline change animation progress
    qreal m_overrideOutlineAnimationProgress = 0;
    //* titlebar caption opacity
    qreal m_captionOpacity = 1;

    //* frame corner radius, scaled for x11
    qreal m_scaledCornerRadius = 3.0;

    //* border sizes, scaled for x11, snapped
    qreal m_scaledBorderLeftRight = 0;
    qreal m_scaledBorderLeftRightNext = 0;
    qreal m_scaledBorderBottom = 0;
    qreal m_scaledBorderBottomNext = 0;

    //* titebar margins, scaled for x11, snapped
    qreal m_scaledIntegratedRoundedRectangleBottomPadding = 0;
    qreal m_scaledIntegratedRoundedRectangleBottomPaddingNext = 0;
    qreal m_scaledTitleBarTopMargin = 0;
    qreal m_scaledTitleBarTopMarginNext = 0;
    qreal m_scaledTitleBarBottomMargin = 0;
    qreal m_scaledTitleBarBottomMarginNext = 0;
    qreal m_scaledTitleBarLeftMargin = 0;
    qreal m_scaledTitleBarLeftMarginNext = 0;
    qreal m_scaledTitleBarRightMargin = 0;
    qreal m_scaledTitleBarRightMarginNext = 0;
    qreal m_scaledTitleBarLeftMarginMaximizedHorizontally = 0;
    qreal m_scaledTitleBarLeftMarginMaximizedHorizontallyNext = 0;
    qreal m_scaledTitleBarRightMarginMaximizedHorizontally = 0;
    qreal m_scaledTitleBarRightMarginMaximizedHorizontallyNext = 0;
    qreal m_scaledTitleBarSeparatorHeight = 0;
    qreal m_scaledTitleBarSeparatorHeightNext = 0;

    //* button dimensions, scaled for x11, snapped
    qreal m_scaledButtonSpacingLeft = 0;
    qreal m_scaledButtonSpacingRight = 0;
    qreal m_scaledButtonWidthMarginLeft = 0;
    qreal m_scaledButtonWidthMarginRight = 0;

    bool m_darkTheme = false;

    //* Rectangular area of titlebar without clipped corners
    QRectF m_titleRect;

    //* Exact titlebar path, with clipped rounded corners
    QPainterPath m_titleBarPath = QPainterPath();
    //* Exact window path, with clipped rounded corners
    QPainterPath m_windowPath = QPainterPath();

    qreal m_systemScaleFactorX11 = 1.0;
    qreal m_x11Scale = 1.0;
    qreal m_smallSpacing = 2.0;
    qreal m_gridUnit = 10.0;

    ButtonBackgroundType m_buttonBackgroundType = ButtonBackgroundType::Small;
    qreal m_scaledSmallButtonPaddedSize = 0;
    qreal m_scaledSmallButtonPaddedSizeNext = 0;
    qreal m_scaledSmallButtonBackgroundSize = 0;
    qreal m_scaledSmallButtonBackgroundSizeNext = 0;
    qreal m_scaledIconSize = 0;
    qreal m_scaledIconSizeNext = 0;

    bool m_colorSchemeHasHeaderColor = true;
    bool m_toolsAreaWillBeDrawn = true;

    //*the actual thin window outline colour to output
    QColor m_windowOutline = QColor();
    //*colour to override thin window outline with, set from decoration button
    QColor m_windowOutlineOverride = QColor();
    //*buffered existing thin window outline colours in case the above override colour is set (needed for animations)
    QColor m_originalWindowOutlineActivePreOverride = QColor();
    QColor m_originalWindowOutlineInactivePreOverride = QColor();
    //*flag to animate out an overridden thin window outline
    bool m_animateOutOverriddenWindowOutline = false;

    bool m_buttonUnisonHovered = false; // for unison hovering
};

bool Decoration::hasBorders() const
{
    if (m_internalSettings && m_internalSettings->exceptionBorder()) {
        return m_internalSettings->borderSize() > InternalSettings::EnumBorderSize::NoSides;
    } else {
        return settings()->borderSize() > KDecoration3::BorderSize::NoSides;
    }
}

bool Decoration::hasNoBorders() const
{
    if (m_internalSettings && m_internalSettings->exceptionBorder()) {
        return m_internalSettings->borderSize() == InternalSettings::EnumBorderSize::None;
    } else {
        return settings()->borderSize() == KDecoration3::BorderSize::None;
    }
}

bool Decoration::hasNoSideBorders() const
{
    if (m_internalSettings && m_internalSettings->exceptionBorder()) {
        return m_internalSettings->borderSize() == InternalSettings::EnumBorderSize::NoSides;
    } else {
        return settings()->borderSize() == KDecoration3::BorderSize::NoSides;
    }
}

bool Decoration::windowOutlineNone() const
{
    auto c = window();

    // determine when a window outline does not need to be drawn (even when set to none, sometimes needs to be drawn if there is an animation)
    return ((m_internalSettings->windowOutlineStyle(true) == InternalSettings::EnumWindowOutlineStyle::None
             && m_internalSettings->windowOutlineStyle(false) == InternalSettings::EnumWindowOutlineStyle::None)
            || (m_animation->state() != QAbstractAnimation::Running
                && ((c->isActive() && m_internalSettings->windowOutlineStyle(true) == InternalSettings::EnumWindowOutlineStyle::None)
                    || (!c->isActive() && m_internalSettings->windowOutlineStyle(false) == InternalSettings::EnumWindowOutlineStyle::None))))
        && (!(c->isKeepAbove() && m_internalSettings->colorizeWindowOutlineWithButton()));
}

bool Decoration::isMaximized() const
{
    auto c = window();
    return c->isMaximized() && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isMaximizedHorizontally() const
{
    auto c = window();
    return c->isMaximizedHorizontally() && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isMaximizedVertically() const
{
    auto c = window();
    return c->isMaximizedVertically() && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isLeftEdge() const
{
    auto c = window();
    return (c->isMaximizedHorizontally() || c->adjacentScreenEdges().testFlag(Qt::LeftEdge)) && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isRightEdge() const
{
    auto c = window();

    return (c->isMaximizedHorizontally() || c->adjacentScreenEdges().testFlag(Qt::RightEdge)) && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isTopEdge() const
{
    auto c = window();

    return (c->isMaximizedVertically() || c->adjacentScreenEdges().testFlag(Qt::TopEdge)) && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isBottomEdge() const
{
    auto c = window();

    return (c->isMaximizedVertically() || c->adjacentScreenEdges().testFlag(Qt::BottomEdge)) && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::hideTitleBar() const
{
    bool hide = false;
    auto c = window();

    switch (m_internalSettings->hideTitleBar()) {
    default:
    case InternalSettings::EnumHideTitleBar::Never:
        break;
    case InternalSettings::EnumHideTitleBar::Always:
        hide = !c->isShaded();
        break;
    case InternalSettings::EnumHideTitleBar::Maximized:
        hide = !c->isShaded() && isMaximized();
        break;
    case InternalSettings::EnumHideTitleBar::AnyMaximization:
        hide = !c->isShaded() && (isMaximized() || isMaximizedVertically() || isLeftEdge() || isRightEdge());
        break;
    case InternalSettings::EnumHideTitleBar::KeptBehind:
        hide = !c->isShaded() && c->isKeepBelow();
        break;
    case InternalSettings::EnumHideTitleBar::AnyMaximizationOrKeptBehind:
        hide = !c->isShaded() && (c->isKeepBelow() || isMaximized() || isMaximizedVertically() || isLeftEdge() || isRightEdge());
        break;
    }
    return hide;
}

} // end Klassy namespace
