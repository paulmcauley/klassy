/*
 * Copyright (C) 2025 Guido Iodice <guido[dot]iodice[at]gmail[dot]com>
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

// own
#include "appmenu/buttongroup.h"
#include "appmenu/iconbutton.h"
#include "appmenu/model.h"
#include "appmenu/navigablemenu.h"
#include "appmenu/searchbutton.h"
#include "appmenu/textbutton.h"
#include "breezedecoration.h"

// KDecoration
#include <KDecoration3/DecoratedWindow>

// KF
#include <KLocalizedString>
#include <KWindowSystem>

// Qt
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QSet>
#include <QStringMatcher>
#include <QTimer>
#include <QVariantAnimation>
#include <QWidgetAction>

#include <utility>

namespace Breeze
{

AppMenuButtonGroup::AppMenuButtonGroup(Decoration *decoration)
    : KDecoration3::DecorationButtonGroup(decoration)
    , m_decoration(decoration)
    , m_appMenuModel(new AppMenuModel(this, decoration))
    , m_animation(new QVariantAnimation(this))
{
    m_menuUpdateDebounceTimer = new QTimer(this);
    m_menuUpdateDebounceTimer->setInterval(100);
    m_menuUpdateDebounceTimer->setSingleShot(true);
    connect(m_menuUpdateDebounceTimer, &QTimer::timeout, this, &AppMenuButtonGroup::onMenuUpdateThrottleTimeout);

    m_delayedCacheTimer = new QTimer(this);
    m_delayedCacheTimer->setInterval(200);
    m_delayedCacheTimer->setSingleShot(true);
    connect(m_delayedCacheTimer, &QTimer::timeout, this, &AppMenuButtonGroup::onDelayedCacheTimerTimeout);

    m_resetTimer = new QTimer(this);
    m_resetTimer->setInterval(250);
    m_resetTimer->setSingleShot(true);
    connect(m_resetTimer, &QTimer::timeout, this, [this]() {
        resetButtons();
        m_menuLoadedOnce = true;
        Q_EMIT menuUpdated();
    });

    m_menuLoadFallbackTimer = new QTimer(this);
    m_menuLoadFallbackTimer->setInterval(0);
    m_menuLoadFallbackTimer->setSingleShot(true);
    connect(m_menuLoadFallbackTimer, &QTimer::timeout, this, [this]() {
        if (!m_menuLoadedOnce) {
            m_menuLoadedOnce = true;
            Q_EMIT menuUpdated();
        }
    });

    // Assign style, showing, and opacity before we bind the onShowingChanged animation
    // so that new windows do not animate.
    auto internalSettings = m_decoration->internalSettings();
    if (internalSettings->exceptionIntegratedMenuShowStyle()) {
        setStyle(static_cast<AppMenuStyle>(internalSettings->exceptionIntegratedMenuShowStyle() - 1));
    } else {
        setStyle(static_cast<AppMenuStyle>(internalSettings->integratedMenuShowStyle()));
    }
    updateShowing();
    setOpacity((m_showing || expandsOnHover()) ? 1 : 0);

    connect(this, &AppMenuButtonGroup::showingChanged, this, &AppMenuButtonGroup::onShowingChanged);
    connect(this, &AppMenuButtonGroup::hoveredChanged, this, &AppMenuButtonGroup::updateShowing);
    connect(this, &AppMenuButtonGroup::currentIndexChanged, this, &AppMenuButtonGroup::updateShowing);

    // Hover animation for integrated menu buttons
    connect(this, &AppMenuButtonGroup::hoveredChanged, this, [this](bool hovered) {
        if (!m_decoration->internalSettings()->unisonHovering()) {
            updateHoverAnimationState(hovered);
        }
    });

    m_animationEnabled = decoration->internalSettings()->animationsEnabled();
    m_animation->setDuration(decoration->animationsDuration());
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        if (m_style == AppMenuStyle::ReplaceTitleOnHover || m_style == AppMenuStyle::RevealOnHover) {
            setOpacity(value.toReal());
        } else if (expandsOnHover()) {
            setExpansionPercent(value.toReal());
        }
    });

    auto decoratedClient = decoration->window();
    connect(decoratedClient, &KDecoration3::DecoratedWindow::hasApplicationMenuChanged, this, &AppMenuButtonGroup::onHasApplicationMenuChanged);
    connect(decoratedClient, &KDecoration3::DecoratedWindow::applicationMenuChanged, this, &AppMenuButtonGroup::onApplicationMenuChanged);

    connect(m_appMenuModel, &AppMenuModel::modelReset, this, &AppMenuButtonGroup::updateAppMenuModel);
    connect(m_appMenuModel, &AppMenuModel::menuReadyForSearch, this, &AppMenuButtonGroup::onMenuReadyForSearch);
    connect(m_appMenuModel, &AppMenuModel::subMenuReady, this, &AppMenuButtonGroup::onSubMenuReady);

    if (decoratedClient->hasApplicationMenu()) {
        onHasApplicationMenuChanged(true);
    } else {
        // Wait the next loop if not modal.
        if (decoratedClient->isModal()) {
            m_menuLoadedOnce = true;
        } else {
            m_menuLoadFallbackTimer->start();
        }
    }
}

AppMenuButtonGroup::~AppMenuButtonGroup()
{
    // explicit destruction even
    // if it already is Qt::WA_DeleteOnClose,
    // deal whit the corner-case in which the window
    // is closed while the m_overflowMenu is open
    if (m_overflowMenu) {
        m_overflowMenu->deleteLater();
    }
}

void AppMenuButtonGroup::reconfigure()
{
    auto internalSettings = m_decoration->internalSettings();
    if (internalSettings->exceptionIntegratedMenuShowStyle()) {
        setStyle(static_cast<AppMenuStyle>(internalSettings->exceptionIntegratedMenuShowStyle() - 1));
    } else {
        setStyle(static_cast<AppMenuStyle>(internalSettings->integratedMenuShowStyle()));
    }
    setPosition(static_cast<AppMenuPosition>(internalSettings->integratedMenuPosition()));
    setUnisonHoveringType(static_cast<AppMenuUnisonHovering>(internalSettings->integratedMenuUnisonHovering()));

    if (!internalSettings->integratedMenuSearchEnabled() && m_searchButton) {
        removeButton(m_searchButton);
        m_searchButton->deleteLater();
        m_searchButton = nullptr;
        m_searchIndex = -1;
    }

    m_animation->setDuration(m_decoration->animationsDuration());

    for (KDecoration3::DecorationButton *button : this->buttons()) {
        if (auto appMenuButton = qobject_cast<AppMenuButton *>(button))
            appMenuButton->reconfigure();
    }
}

bool AppMenuButtonGroup::alwaysShow() const
{
    // NOTE: AlwaysExpandOnHover is excluded because it's not 'fully' shown until hovered.
    return m_style != AppMenuStyle::AlwaysExpandOnHover && m_style != AppMenuStyle::ReplaceTitleOnHover && m_style != AppMenuStyle::RevealOnHover;
}

KDecoration3::DecorationButton *AppMenuButtonGroup::buttonAt(QPoint pos) const
{
    for (auto &tb : std::as_const(m_textButtons)) {
        if (tb && tb->isVisible() && tb->geometry().contains(pos)) {
            return tb;
        }
    }
    if (m_overflowButton && m_overflowButton->isVisible() && m_overflowButton->geometry().contains(pos)) {
        return m_overflowButton;
    }
    if (m_searchButton && m_searchButton->isVisible() && m_searchButton->geometry().contains(pos)) {
        return m_searchButton;
    }
    return nullptr;
}

void AppMenuButtonGroup::startDragMove(const QPoint &pos)
{
    m_pressedPoint = pos;
}

void AppMenuButtonGroup::resetDragMove()
{
    m_pressedPoint = QPoint();
}

bool AppMenuButtonGroup::dragMoveTick(const QPoint &pos)
{
    if (m_pressedPoint.isNull()) {
        return false;
    }

    const QPoint diff = pos - m_pressedPoint;
    if (diff.manhattanLength() >= QApplication::startDragDistance()) {
        resetDragMove();
        return true;
    }
    return false;
}

void AppMenuButtonGroup::resetButtons()
{
    if (buttons().isEmpty()) {
        return;
    }
    setCurrentIndex(-1);
    m_currentMenu = nullptr;
    m_textButtons.clear();
    m_overflowButton = nullptr;
    m_searchButton = nullptr;
    m_overflowIndex = -1;
    m_searchIndex = -1;

    if (m_overflowMenu) {
        m_overflowMenu->deleteLater();
    }

    // Create a copy of the button pointers before removing them from the group.
    const auto allButtons = buttons();

    // This removes all buttons with the "Custom" type from the group's list,
    // but does not delete the button widgets themselves.
    removeButton(KDecoration3::DecorationButtonType::Custom);

    // Now, immediately delete the button widgets we took ownership of.
    // This is necessary to prevent layout race conditions when recreating buttons
    // in the same event loop cycle.
    qDeleteAll(allButtons);

    Q_EMIT menuUpdated();
}

void AppMenuButtonGroup::onMenuReadyForSearch()
{
    if (m_searchButton && m_searchButton->isUiVisible())
        m_searchButton->filterToLastSearch();
}

void AppMenuButtonGroup::onHasApplicationMenuChanged(bool hasMenu)
{
    if (hasMenu) {
        m_resetTimer->stop();
        m_menuLoadedOnce = false;
        m_menuLoadFallbackTimer->start(500);
        Q_EMIT menuUpdated();

        if (m_isMenuUpdateThrottled) {
            m_pendingMenuUpdate = true;
            return;
        }
        performDebouncedMenuUpdate();
        m_isMenuUpdateThrottled = true;
        m_menuUpdateDebounceTimer->start();
    } else {
        m_menuUpdateDebounceTimer->stop();
        m_isMenuUpdateThrottled = false;
        m_pendingMenuUpdate = false;
        m_menuLoadFallbackTimer->stop();

        // Defer reset to avoid flicker during window closure
        m_resetTimer->start();
        m_menuLoadedOnce = false;
        Q_EMIT menuUpdated();
    }
}

void AppMenuButtonGroup::onApplicationMenuChanged()
{
    if (m_isMenuUpdateThrottled) {
        m_pendingMenuUpdate = true;
        return;
    }
    performDebouncedMenuUpdate();
    m_isMenuUpdateThrottled = true;
    m_menuUpdateDebounceTimer->start();
}

void AppMenuButtonGroup::onMenuUpdateThrottleTimeout()
{
    m_isMenuUpdateThrottled = false;
    if (m_pendingMenuUpdate) {
        m_pendingMenuUpdate = false;
        onApplicationMenuChanged();
    }
}

void AppMenuButtonGroup::onDelayedCacheTimerTimeout()
{
    if (!m_appMenuModel) {
        return;
    }

    if (m_buttonIndexOfMenuToCache == m_searchIndex) {
        m_appMenuModel->startDeepCaching();
    }
}

void AppMenuButtonGroup::performDebouncedMenuUpdate()
{
    auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return;
    }
    auto decoratedClient = deco->window();
    if (m_appMenuModel && decoratedClient->hasApplicationMenu()) {
        const QString serviceName = decoratedClient->applicationMenuServiceName();
        const QString menuObjectPath = decoratedClient->applicationMenuObjectPath();
        if (!serviceName.isEmpty() && !menuObjectPath.isEmpty()) {
            m_appMenuModel->updateApplicationMenu(serviceName, menuObjectPath);
        } else {
            m_resetTimer->start();
        }
    }
}

void AppMenuButtonGroup::updateAppMenuModel()
{
    auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return;
    }
    auto decoratedClient = deco->window();

    // Don't display AppMenu in modal windows.
    if (decoratedClient->isModal()) {
        m_resetTimer->start();
        return;
    }

    if (!decoratedClient->hasApplicationMenu()) {
        // Defer reset to avoid flicker during window closure
        m_resetTimer->start();
        return;
    }

    if (m_appMenuModel) {
        QMenu *menu = m_appMenuModel->menu();
        if (!menu) {
            // Defer reset to avoid flicker during window closure
            m_resetTimer->start();
            return;
        }

        const auto actions = menu->actions();
        const int menuActionCount = actions.count();

        // Preserve current menu state if possible
        const bool wasSearchOpen = (m_currentIndex == m_searchIndex && m_searchIndex != -1);
        const bool wasOverflowOpen = (m_currentIndex == m_overflowIndex && m_overflowIndex != -1);
        QPointer<QMenu> previousMenu = m_currentMenu;

        // Try in-place update if possible to reduce flicker and object churn
        const bool searchEnabled = deco->internalSettings()->integratedMenuSearchEnabled();
        const bool searchStateMatches = (m_searchButton.isNull() == !searchEnabled);

        if (m_textButtons.count() == menuActionCount && !m_textButtons.isEmpty() && searchStateMatches) {
            if (m_searchButton)
                m_searchButton->clearCache();
            int actionIdx = 0;
            for (auto &textButton : std::as_const(m_textButtons)) {
                if (!textButton) {
                    actionIdx++;
                    continue;
                }
                QAction *itemAction = actions.at(actionIdx++);
                textButton->setAction(itemAction);
                textButton->setText(itemAction->text().trimmed());
                // Skip items with empty labels (The first item in a Gtk app)
                if (itemAction->text().isEmpty()) {
                    textButton->setEnabled(false);
                    textButton->setVisible(false);
                } else {
                    textButton->setEnabled(itemAction->isEnabled());
                    textButton->setVisible(true);
                }
            }

            if (wasSearchOpen) {
                m_searchButton->filterToLastSearch();
            }
        } else {
            resetButtons(); // Immediate reset is intended here for structural changes

            // Populate
            for (int i = 0; i < menuActionCount; ++i) {
                QAction *itemAction = actions.at(i);
                const QString itemLabel = itemAction->text().trimmed();

                AppMenuTextButton *b = new AppMenuTextButton(deco, i, this);
                b->setText(itemLabel);
                b->setAction(itemAction);
                b->setOpacity(m_opacity);

                // Skip items with empty labels (The first item in a Gtk app)
                if (itemLabel.isEmpty()) {
                    b->setEnabled(false);
                    b->setVisible(false);
                } else {
                    b->setEnabled(itemAction->isEnabled());
                }

                m_textButtons.append(b);
                addButton(QPointer<KDecoration3::DecorationButton>(b));
            }

            if (menuActionCount > 0) {
                m_overflowIndex = menuActionCount;
                m_overflowButton = new AppMenuIconButton(DecorationButtonType::CustomIntegratedMenuOverflow, deco, m_overflowIndex, this);
                addButton(QPointer<KDecoration3::DecorationButton>(m_overflowButton));

                if (searchEnabled) {
                    m_searchIndex = menuActionCount + 1;
                    m_searchButton = new AppMenuSearchButton(deco, m_searchIndex, this);
                    addButton(QPointer<KDecoration3::DecorationButton>(m_searchButton));
                }
            }
        }

        // Restore state
        int indexToRestore = -1;
        if (wasSearchOpen && m_searchIndex != -1) {
            indexToRestore = m_searchIndex;
        } else if (wasOverflowOpen && m_overflowIndex != -1) {
            indexToRestore = m_overflowIndex;
        }

        if (indexToRestore != -1) {
            setCurrentIndex(indexToRestore);
            m_currentMenu = previousMenu;

            if (AppMenuButton *b = getAppMenuButton(m_currentIndex)) {
                b->setChecked(true);
            }
        }

        if (menuActionCount > 0) {
            m_resetTimer->stop();
            m_menuLoadFallbackTimer->stop();
            m_menuLoadedOnce = true;
        }

        Q_EMIT menuUpdated();
    }
}

// NOTE: fuzzyLessThanOrEqual was needed in updateOverflow to prevent flickering due to small rounding errors
bool fuzzyLessThanOrEqual(qreal a, qreal b)
{
    return a <= b || qFuzzyCompare(a, b);
}

void AppMenuButtonGroup::updateOverflow(QRectF availableRect)
{
    const qreal availableExpandedWidth = availableRect.width();
    if (m_style == AppMenuStyle::AlwaysExpandOnHover) {
        const QPair<QRectF, Qt::Alignment> captionRect = m_decoration->captionRect(false, true);
        const QRectF maxCaptionSize = m_decoration->getMaxCaptionSize();
        const qreal captionOffset = qMin(captionRect.first.width(), maxCaptionSize.width()) + m_decoration->internalSettings()->titleBarLeftMargin()
            + m_decoration->internalSettings()->titleBarRightMargin();
        if (m_position == AppMenuPosition::Left) {
            availableRect.adjust(0, 0, -captionOffset, 0);
        } else {
            availableRect.adjust(captionOffset, 0, 0, 0);
        }
    }
    const qreal availableUnexpandedWidth = availableRect.width();

    const qreal overflowBtnWidth = m_overflowButton ? m_overflowButton->geometry().width() + spacing() : 0;
    const qreal searchBtnWidth = m_searchButton ? m_searchButton->geometry().width() + spacing() : 0;

    qreal minVisibleWidth = searchBtnWidth;
    qreal maxVisibleWidth = searchBtnWidth;

    if (m_style == AppMenuStyle::SearchOnly) {
        for (auto &tb : std::as_const(m_textButtons)) {
            if (tb)
                tb->setVisible(false);
        }
    } else {
        // First pass: check if all enabled text buttons fit without overflow button.
        // We perform this pass without side effects to avoid layout thrashing in Qt.
        qreal totalTextWidth = 0;
        bool allFit = true;
        for (auto &tb : std::as_const(m_textButtons)) {
            if (tb->isEnabled()) {
                totalTextWidth += tb->geometry().width() + spacing();
                if (searchBtnWidth + totalTextWidth > availableUnexpandedWidth) {
                    allFit = false;
                    break;
                }
            }
        }

        // NOTE: If no buttons are enabled, then totalTextWidth is 0 & allFit is true
        if (allFit) {
            minVisibleWidth += totalTextWidth;
            maxVisibleWidth += totalTextWidth;
            // Second pass: apply visibility
            for (auto &tb : std::as_const(m_textButtons)) {
                if (tb) {
                    tb->setVisible(tb->isEnabled());
                    tb->setExpansionOpacity(1);
                }
            }
            m_overflowButton->setVisible(false);
            if (m_searchButton) {
                m_searchButton->setVisible(true);
                m_searchButton->setExpansionOpacity(1);
            }
        } else if (expandsOnHover()) {
            qreal remainingMaxWidth = availableExpandedWidth - searchBtnWidth - overflowBtnWidth;
            qreal remainingMinWidth = availableUnexpandedWidth - searchBtnWidth - overflowBtnWidth;
            qreal remainingExpandedWidth = remainingMinWidth + (availableExpandedWidth - availableUnexpandedWidth) * m_expansionPercent;

            const bool isUnexpanded = qFuzzyCompare(m_expansionPercent, 0);

            // Second pass: apply visibility and calculate final width
            bool fitsInMin = true;
            bool fitsInExpanded = true;
            bool fitsInCurrentExpansion = true;
            bool partialButtonExists = false;
            for (auto &tb : std::as_const(m_textButtons)) {
                if (!tb) {
                    continue;
                }
                if (!tb->isEnabled()) {
                    tb->setVisible(false);
                    continue;
                }
                const qreal w = tb->geometry().width() + spacing();
                bool visible = false;
                qreal opacity = 0;
                if (fitsInMin && fuzzyLessThanOrEqual(w, remainingMinWidth)) {
                    visible = true;
                    opacity = 1;
                    remainingMinWidth -= w;
                } else {
                    fitsInMin = false;
                }

                if (fitsInCurrentExpansion && fuzzyLessThanOrEqual(w, remainingExpandedWidth)) {
                    visible = true;
                    opacity = qMax(opacity, m_expansionPercent);
                    remainingExpandedWidth -= w;
                    minVisibleWidth += w;
                } else if (fitsInCurrentExpansion && fuzzyLessThanOrEqual(w, remainingMaxWidth)) {
                    if (!isUnexpanded) {
                        visible = true;
                        opacity = qMax(opacity, remainingExpandedWidth / w * m_expansionPercent);
                    }
                    partialButtonExists = true;
                    fitsInCurrentExpansion = false;
                } else {
                    fitsInCurrentExpansion = false;
                }

                if (fitsInExpanded && fuzzyLessThanOrEqual(w, remainingMaxWidth)) {
                    // If visible has been set, don't override the current opacity
                    if (!visible)
                        opacity = 0;
                    visible = true;
                    remainingMaxWidth -= w;
                    maxVisibleWidth += w;
                } else {
                    fitsInExpanded = false;
                }

                tb->setVisible(visible);
                tb->setExpansionOpacity(opacity);
            }

            remainingExpandedWidth += ((fitsInExpanded ? 0 : overflowBtnWidth) + searchBtnWidth) * m_expansionPercent;
            m_overflowButton->setVisible(!fitsInExpanded);
            if (!fitsInExpanded) {
                maxVisibleWidth += overflowBtnWidth;
                if (!partialButtonExists) {
                    m_overflowButton->setExpansionOpacity(qMin(1.0, remainingExpandedWidth / overflowBtnWidth) * m_expansionPercent);
                    remainingExpandedWidth -= overflowBtnWidth;
                    partialButtonExists |= !fuzzyLessThanOrEqual(overflowBtnWidth, remainingExpandedWidth);
                } else {
                    m_overflowButton->setExpansionOpacity(0);
                }
            }
            if (m_searchButton) {
                maxVisibleWidth += searchBtnWidth;
                m_searchButton->setVisible(true);
                if (!partialButtonExists) {
                    m_searchButton->setExpansionOpacity(qMin(1.0, remainingExpandedWidth / overflowBtnWidth) * m_expansionPercent);
                } else {
                    m_searchButton->setExpansionOpacity(0);
                }
            }
        } else {
            qreal remainingWidth = availableExpandedWidth - searchBtnWidth - overflowBtnWidth;

            // Second pass: apply visibility and calculate final width
            bool fits = true;
            for (auto &tb : std::as_const(m_textButtons)) {
                if (!tb) {
                    continue;
                }
                if (fits && tb->isEnabled()) {
                    const qreal w = tb->geometry().width() + spacing();
                    if (w <= remainingWidth) {
                        tb->setVisible(true);
                        minVisibleWidth += w;
                        remainingWidth -= w;
                        continue;
                    }
                    fits = false;
                }
                tb->setVisible(false);
            }
            if (m_overflowButton) {
                m_overflowButton->setVisible(true);
                minVisibleWidth += overflowBtnWidth;
            }
            maxVisibleWidth = minVisibleWidth;
        }
    }

    setOverflowing(m_overflowButton && m_overflowButton->isVisible());

    updateAdjacencyFlags();

    if (!(qFuzzyCompare(m_minimumWidth, minVisibleWidth) && qFuzzyCompare(m_maximumWidth, maxVisibleWidth))) {
        m_minimumWidth = minVisibleWidth;
        m_maximumWidth = maxVisibleWidth;
        Q_EMIT menuUpdated();
    }
}

void AppMenuButtonGroup::updateAdjacencyFlags()
{
    AppMenuButton *firstVisible = nullptr;
    AppMenuButton *lastVisible = nullptr;
    for (auto tb : buttons()) {
        if (auto button = qobject_cast<AppMenuButton *>(tb)) {
            if (!firstVisible && button->isVisible()) {
                firstVisible = button;
            }
            if (button->isVisible())
                lastVisible = button;
            button->setLeftmostVisible(firstVisible == button);
            button->setRightmostVisible(false);
        }
    }
    if (lastVisible)
        lastVisible->setRightmostVisible(true);
}

void AppMenuButtonGroup::updateGeometry()
{
    if (buttons().isEmpty())
        return;
    const auto internalSettings = m_decoration->internalSettings();
    const qreal scale = m_decoration->window()->nextScale();
    const qreal leftOffset = m_decoration->leftButtons()->geometry().right() + internalSettings->buttonSpacingLeft() * scale;
    const qreal rightOffset = m_decoration->rightButtons()->geometry().width() + internalSettings->buttonSpacingRight() * scale;

    const qreal titleBarSeparatorHeight = m_decoration->titleBarSeparatorHeight(scale);
    qreal scaledTitleBarTopMargin, scaledTitleBarBottomMargin, scaledIntegratedRoundedRectangleBottomPadding;
    m_decoration->scaledTitleBarTopBottomMargins(scale, scaledTitleBarTopMargin, scaledTitleBarBottomMargin, scaledIntegratedRoundedRectangleBottomPadding);
    const qreal captionHeight = m_decoration->captionHeight(true, scaledTitleBarTopMargin, scaledTitleBarBottomMargin);

    const int buttonShape = internalSettings->integratedMenuButtonShape();
    const bool isFullHeight = AppMenuButton::isShapeFullHeight(buttonShape);
    const qreal baseSize = qMax(m_decoration->smallButtonPaddedSize(), captionHeight);
    const qreal verticalIconOffsetNormal = isFullHeight
        ? scaledTitleBarTopMargin + qreal(captionHeight - baseSize - scaledIntegratedRoundedRectangleBottomPadding) / 2
        : scaledTitleBarTopMargin + qreal(captionHeight - baseSize) / 2;
    const qreal topOffset = isFullHeight ? 0 : verticalIconOffsetNormal;
    const qreal contentOffset = isFullHeight ? verticalIconOffsetNormal : 0;

    qreal baseButtonHeight;
    if (isFullHeight) {
        baseButtonHeight = qMax(m_decoration->nextState()->borders().top() - titleBarSeparatorHeight, 0.0);
        if (buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::IntegratedRoundedRectangle
            || buttonShape == InternalSettings::EnumIntegratedMenuButtonShape::IntegratedRoundedRectangleGrouped) {
            baseButtonHeight = qMax(baseButtonHeight - scaledIntegratedRoundedRectangleBottomPadding, 0.0);
        }
    } else {
        baseButtonHeight = m_decoration->smallButtonBackgroundSize();
    }
    // Handle the case where the button size is smaller than the height we need for text buttons
    const qreal realButtonHeight = qMax(captionHeight, baseButtonHeight);
    const qreal iconTranslation = (m_decoration->smallButtonPaddedSize() - m_decoration->iconSize()) / 2;
    const QPointF iconOffset = {iconTranslation, iconTranslation + (realButtonHeight - contentOffset - m_decoration->smallButtonPaddedSize()) / 2};
    QRectF availableRect(leftOffset, topOffset, m_decoration->size().width() - leftOffset - rightOffset, m_decoration->titleBarHeight() + contentOffset);

    for (auto *button : buttons()) {
        AppMenuButton *appMenuButton;
        if (auto *textButton = qobject_cast<AppMenuTextButton *>(button)) {
            appMenuButton = textButton;
            textButton->setHorizontalPadding(internalSettings->integratedMenuButtonHorizontalPadding());
        } else if (auto *iconButton = qobject_cast<AppMenuIconButton *>(button)) {
            appMenuButton = iconButton;
            iconButton->setIconOffset(iconOffset);
        } else {
            continue;
        }

        appMenuButton->setVerticalContentOffset(contentOffset);
        appMenuButton->setButtonHeight(realButtonHeight);
    }

    setSpacing(internalSettings->integratedMenuButtonHorizontalMargin());
    updateOverflow(availableRect);

    const bool isReplaceStyle = m_style == AppMenuStyle::ReplaceTitleOnHover;
    if (isReplaceStyle && m_position == AppMenuPosition::Center) {
        const qreal x = (availableRect.width() - visibleWidth()) / 2 + leftOffset;
        setPos(QPointF(x, availableRect.y()));
    } else if (isReplaceStyle && m_position == AppMenuPosition::CenterFullWidth) {
        const qreal x = (m_decoration->size().width() - visibleWidth()) / 2;
        setPos(QPointF(x, availableRect.y()));
    } else if (m_position == AppMenuPosition::Right) {
        setPos(availableRect.topRight() - QPointF(visibleWidth(), 0));
    } else {
        setPos(availableRect.topLeft());
    }
}

qreal AppMenuButtonGroup::visibleWidth() const
{
    return m_minimumWidth + (m_maximumWidth - m_minimumWidth) * m_expansionPercent;
}

bool AppMenuButtonGroup::menuLoadedOnce() const
{
    return m_menuLoadedOnce;
}

bool AppMenuButtonGroup::isWaitingForMenu() const
{
    return (m_menuLoadFallbackTimer && m_menuLoadFallbackTimer->isActive()) || (m_resetTimer && m_resetTimer->isActive());
}

void AppMenuButtonGroup::popupMenu(QMenu *menu, int buttonIndex)
{
    // Stop any caching that may be in progress from a previously opened menu.
    m_appMenuModel->stopCaching();

    auto *deco = qobject_cast<Decoration *>(decoration());
    AppMenuButton *button = getAppMenuButton(buttonIndex);

    if (!menu || !deco || !button) {
        return;
    }

    QPointer<QMenu> oldMenu = m_currentMenu;
    AppMenuButton *oldButton = getAppMenuButton(m_currentIndex);

    // 1. Set the new internal state. This must happen before popup for positioning.
    setCurrentIndex(buttonIndex);
    button->setChecked(true);
    if (m_immediateTransition)
        button->transitionFully();
    m_currentMenu = menu;

    // 2. Calculate position and show the new menu. This must happen before hiding the old one to prevent flicker.
    if (auto navMenu = qobject_cast<NavigableMenu *>(menu)) {
        connect(navMenu, &NavigableMenu::hitLeft, this, &AppMenuButtonGroup::onHitLeft, Qt::UniqueConnection);
        connect(navMenu, &NavigableMenu::hitRight, this, &AppMenuButtonGroup::onHitRight, Qt::UniqueConnection);
    }
    if (m_searchButton && menu != m_searchButton->menu()) {
        menu->installEventFilter(this);
    }

    KDecoration3::Positioner positioner;
    positioner.setAnchorRect(button->geometry());
    deco->popup(positioner, menu);

    if (buttonIndex == m_searchIndex) {
        m_searchButton->setLineFocus();
    }

    // 3. Connect the hide signal for the new menu.
    connect(menu, &QMenu::aboutToHide, this, &AppMenuButtonGroup::onMenuAboutToHide, Qt::UniqueConnection);

    // 4. Clean up the old menu and button state.
    if (oldMenu && oldMenu != menu) {
        if (auto oldNavMenu = qobject_cast<NavigableMenu *>(oldMenu)) {
            disconnect(oldNavMenu, &NavigableMenu::hitLeft, this, &AppMenuButtonGroup::onHitLeft);
            disconnect(oldNavMenu, &NavigableMenu::hitRight, this, &AppMenuButtonGroup::onHitRight);
        }
        disconnect(oldMenu, &QMenu::aboutToHide, this, &AppMenuButtonGroup::onMenuAboutToHide);
        oldMenu->hide();
    }
    if (oldButton && oldButton != button) {
        oldButton->setChecked(false);
    }

    // When the search menu is shown, trigger deep caching after a short delay
    // to make subsequent searches faster.
    m_delayedCacheTimer->stop();
    m_buttonIndexOfMenuToCache = buttonIndex;
    m_delayedCacheTimer->start();
}

void AppMenuButtonGroup::handleMenuButtonTrigger(int buttonIndex)
{
    if (!m_appMenuModel || !m_appMenuModel->menu() || buttonIndex >= m_appMenuModel->menu()->actions().count()) {
        return; // Index out of bounds
    }

    QAction *itemAction = m_appMenuModel->menu()->actions().at(buttonIndex);
    if (itemAction && itemAction->menu()) {
        QMenu *actionMenu = itemAction->menu();
        // If the menu is empty, we need to load it just-in-time.
        if (actionMenu->actions().isEmpty()) {
            // If we are already waiting for a different menu, cancel the old one.
            if (m_buttonIndexWaitingForPopup != -1 && m_buttonIndexWaitingForPopup != buttonIndex) {
                m_buttonIndexWaitingForPopup = -1;
            }

            // If we are already waiting for this menu, do nothing.
            if (m_buttonIndexWaitingForPopup == buttonIndex) {
                return;
            }

            m_buttonIndexWaitingForPopup = buttonIndex;
            m_appMenuModel->loadSubMenu(actionMenu);
            return; // Abort the trigger; the onSubMenuReady slot will re-trigger later.
        } else {
            // Menu is already loaded, show it.
            popupMenu(actionMenu, buttonIndex);
        }
    }
}

void AppMenuButtonGroup::handleSearchTrigger()
{
    if (m_currentIndex == m_searchIndex) {
        m_searchButton->menu()->hide();
        return;
    }
    popupMenu(m_searchButton->menu(), m_searchIndex);
}

void AppMenuButtonGroup::handleOverflowTrigger()
{
    // A latent bug would cause this to show a menu with all items if triggered
    // while the overflow button is invisible. This guard prevents that.
    if (!overflowing()) {
        return;
    }

    if (m_overflowMenu) {
        m_overflowMenu->deleteLater();
    }

    auto *actionMenu = new NavigableMenu(nullptr, m_decoration);
    actionMenu->setAttribute(Qt::WA_DeleteOnClose);
    m_overflowMenu = actionMenu;

    if (m_appMenuModel && m_appMenuModel->menu()) {
        int overflowStartsAt = 0;
        // Find the first non-visible button to determine where the overflow starts
        for (auto &textButton : std::as_const(m_textButtons)) {
            if (textButton && textButton->isEnabled() && !textButton->isVisible()) {
                overflowStartsAt = textButton->buttonIndex();
                break;
            }
        }

        const auto actions = m_appMenuModel->menu()->actions();
        for (int i = overflowStartsAt; i < actions.count(); ++i) {
            actionMenu->addAction(actions.at(i));
        }
    }

    popupMenu(actionMenu, m_overflowIndex);
}

void AppMenuButtonGroup::trigger(int buttonIndex, bool immediateTransition)
{
    // The button is checked in popupMenu, but we need to check it here
    // for the case where the menu is not yet loaded.
    AppMenuButton *button = getAppMenuButton(buttonIndex);

    if (!button) {
        return;
    }

    m_immediateTransition = immediateTransition;
    if (buttonIndex == m_searchIndex) {
        handleSearchTrigger();
    } else if (buttonIndex == m_overflowIndex) {
        handleOverflowTrigger();
    } else {
        handleMenuButtonTrigger(buttonIndex);
    }
}

// FIXME TODO doesn't work on submenu
bool AppMenuButtonGroup::eventFilter(QObject *watched, QEvent *event)
{
    auto *menu = qobject_cast<QMenu *>(watched);

    if (!menu) {
        return KDecoration3::DecorationButtonGroup::eventFilter(watched, event);
    }

    if (event->type() == QEvent::MouseMove) {
        if (KWindowSystem::isPlatformX11()) {
            auto *e = static_cast<QMouseEvent *>(event);
            auto *deco = const_cast<Decoration *>(qobject_cast<const Decoration *>(decoration()));
            if (deco) {
                // Forward a constructed HoverEvent to the decoration to handle.
                // This is a workaround for X11 where the decoration does not receive
                // hover events while a menu is open.
                QPointF localPos = e->globalPosition() - deco->windowPos();
                localPos.setY(localPos.y() + deco->titleBarHeight());

                QHoverEvent hoverEvent(QEvent::HoverMove,
                                       localPos,
                                       e->globalPosition(),
                                       localPos,
                                       e->modifiers(),
                                       static_cast<const QPointingDevice *>(e->device()));
                QApplication::sendEvent(deco, &hoverEvent);
            }
        }
        return false;
    }

    return KDecoration3::DecorationButtonGroup::eventFilter(watched, event);
}

bool AppMenuButtonGroup::isMenuOpen() const
{
    return 0 <= m_currentIndex;
}

void AppMenuButtonGroup::unpressAllButtons()
{
    for (auto &tb : std::as_const(m_textButtons)) {
        if (tb)
            tb->setChecked(false);
    }
    if (m_overflowButton) {
        m_overflowButton->setChecked(false);
    }
    if (m_searchButton) {
        m_searchButton->setChecked(false);
    }
}

void AppMenuButtonGroup::updateShowing()
{
    setShowing(alwaysShow() || m_hovered || isMenuOpen());
}

void AppMenuButtonGroup::onMenuAboutToHide()
{
    QMenu *menu = qobject_cast<QMenu *>(sender());
    if (!menu) {
        return;
    }

    if (auto navMenu = qobject_cast<NavigableMenu *>(menu)) {
        disconnect(navMenu, &NavigableMenu::hitLeft, this, &AppMenuButtonGroup::onHitLeft);
        disconnect(navMenu, &NavigableMenu::hitRight, this, &AppMenuButtonGroup::onHitRight);
    }

    if (m_searchButton && menu != m_searchButton->menu()) {
        menu->removeEventFilter(this);
    }

    if (AppMenuButton *currentButton = getAppMenuButton(m_currentIndex)) {
        currentButton->setChecked(false);
    }
    setCurrentIndex(-1);
    m_currentMenu = nullptr;
    m_hoveredButton = nullptr;
}

void AppMenuButtonGroup::onHitLeft()
{
    int desiredIndex = findNextVisibleButtonIndex(m_currentIndex, false);
    trigger(desiredIndex, true);
}

void AppMenuButtonGroup::onHitRight()
{
    int desiredIndex = findNextVisibleButtonIndex(m_currentIndex, true);
    trigger(desiredIndex, true);
}

void AppMenuButtonGroup::onShowingChanged(bool showing)
{
    if (m_animationEnabled) {
        const QAbstractAnimation::Direction dir = showing ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        m_animation->setDirection(dir);
        if (m_animation->state() != QAbstractAnimation::Running) {
            m_animation->start();
        }
    } else {
        setOpacity((showing || expandsOnHover()) ? 1 : 0);
        setExpansionPercent(showing ? 1 : 0);
    }
}

void AppMenuButtonGroup::updateHoverAnimationState(bool hovered)
{
    if (!(m_decoration && m_decoration->animationsDuration() > 0)) {
        return;
    }
    // Buttons are always visible — no hover fade
    if (alwaysShow()) {
        return;
    }
    // Don't fade out buttons while hovered or a menu is open
    if (!hovered && isMenuOpen()) {
        return;
    }

    const QAbstractAnimation::Direction dir = hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;

    m_animation->setDirection(dir);
    if (m_animation->state() != QAbstractAnimation::Running) {
        m_animation->resume();
    }
}

void AppMenuButtonGroup::onSubMenuReady(QMenu *menu)
{
    if (m_searchButton) {
        m_searchButton->clearCache();
        if (m_searchButton->isUiVisible()) {
            m_searchButton->startDebounceIfHasLastQuery();
        }
    }

    if (m_buttonIndexWaitingForPopup == -1 || !m_appMenuModel || !m_appMenuModel->menu()) {
        return;
    }

    const auto actions = m_appMenuModel->menu()->actions();
    if (m_buttonIndexWaitingForPopup >= actions.count()) {
        m_buttonIndexWaitingForPopup = -1;
        return;
    }

    QAction *action = actions.at(m_buttonIndexWaitingForPopup);
    if (action && action->menu() == menu) {
        // The menu we were waiting for is now ready.
        // We can now trigger the button again to pop it up.
        // It is crucial to reset the waiting index *before* calling trigger
        // to prevent an infinite loop.
        const int buttonIndex = m_buttonIndexWaitingForPopup;
        m_buttonIndexWaitingForPopup = -1;

        if (menu->actions().isEmpty()) {
            popupMenu(menu, buttonIndex);
        } else {
            trigger(buttonIndex, m_immediateTransition);
        }
    }
}

AppMenuButton *AppMenuButtonGroup::getAppMenuButton(int index) const
{
    if (index == m_searchIndex) {
        return m_searchButton;
    } else if (index == m_overflowIndex) {
        return m_overflowButton;
    } else if (index >= 0 && index < m_textButtons.count()) {
        return m_textButtons.at(index);
    }
    return nullptr;
}

int AppMenuButtonGroup::findNextVisibleButtonIndex(int currentIndex, bool forward) const
{
    int maxIndex = m_textButtons.count() - 1;
    if (m_overflowIndex > maxIndex)
        maxIndex = m_overflowIndex;
    if (m_searchIndex > maxIndex)
        maxIndex = m_searchIndex;

    if (maxIndex < 0) {
        return -1;
    }

    bool isRtl = QGuiApplication::layoutDirection() == Qt::RightToLeft;
    // In RTL, the "next" button visually (forward=true) is at a lower index.
    int step = (forward ^ isRtl) ? 1 : -1;

    // Start from the next button, not the current one
    int newIndex = currentIndex + step;

    for (int i = 0; i <= maxIndex; ++i) {
        // Wrap around logic
        if (newIndex < 0) {
            newIndex = maxIndex;
        } else if (newIndex > maxIndex) {
            newIndex = 0;
        }

        if (AppMenuButton *button = getAppMenuButton(newIndex)) {
            if (button->isVisible() && button->isEnabled()) {
                return newIndex;
            }
        }

        newIndex += step;
    }

    return currentIndex; // Fallback to current index if no other visible button is found
}

void AppMenuButtonGroup::handleHoverMove(const QPointF &pos)
{
    if (!isMenuOpen()) {
        return;
    }

    KDecoration3::DecorationButton *newHoveredButton = buttonAt(pos.toPoint());

    if (m_hoveredButton != newHoveredButton) {
        m_hoveredButton = newHoveredButton;

        if (m_hoveredButton) {
            // All buttons in this group are AppMenuButtons
            auto *appMenuButton = qobject_cast<AppMenuButton *>(m_hoveredButton.data());
            if (appMenuButton && m_currentIndex != appMenuButton->buttonIndex() && appMenuButton->isVisible() && appMenuButton->isEnabled()) {
                trigger(appMenuButton->buttonIndex(), false);
            }
        }
    }
}

} // namespace Breeze
