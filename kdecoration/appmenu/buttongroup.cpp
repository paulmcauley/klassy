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
#include "appmenu/model.h"
#include "appmenu/navigablemenu.h"
#include "appmenu/overflowbutton.h"
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

static constexpr int MAX_SEARCH_RESULTS = 100;

namespace Breeze
{

AppMenuButtonGroup::AppMenuButtonGroup(Decoration *decoration)
    : KDecoration3::DecorationButtonGroup(decoration)
    , m_decoration(decoration)
    , m_menuStyle(new AppMenuMenuStyle(QApplication::style(), decoration))
    , m_appMenuModel(new AppMenuModel(this, m_menuStyle))
    , m_currentIndex(-1)
    , m_overflowIndex(-1)
    , m_searchIndex(-1)
    , m_hamburgerMenu(false)
    , m_hovered(false)
    , m_showing(true)
    , m_alwaysShow(true)
    , m_animationEnabled(false)
    , m_animation(new QVariantAnimation(this))
    , m_opacity(1)
    , m_visibleWidth(0)
    , m_searchMenu(nullptr)
    , m_searchLineEdit(nullptr)
    , m_searchDebounceTimer(nullptr)
    , m_searchUiVisible(false)
{
    m_searchDebounceTimer = new QTimer(this);
    m_searchDebounceTimer->setInterval(150);
    m_searchDebounceTimer->setSingleShot(true);
    connect(m_searchDebounceTimer, &QTimer::timeout, this, &AppMenuButtonGroup::onSearchTimerTimeout);

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

    // Assign showing and opacity before we bind the onShowingChanged animation
    // so that new windows do not animate.
    setAlwaysShow(decoration->internalSettings()->menuAlwaysShow());
    updateShowing();
    setOpacity(m_showing ? 1 : 0);

    connect(this, &AppMenuButtonGroup::showingChanged, this, &AppMenuButtonGroup::onShowingChanged);
    connect(this, &AppMenuButtonGroup::hoveredChanged, this, &AppMenuButtonGroup::updateShowing);
    connect(this, &AppMenuButtonGroup::alwaysShowChanged, this, &AppMenuButtonGroup::updateShowing);
    connect(this, &AppMenuButtonGroup::currentIndexChanged, this, &AppMenuButtonGroup::updateShowing);

    m_animationEnabled = decoration->internalSettings()->animationsEnabled();
    m_animation->setDuration(decoration->animationsDuration());
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setOpacity(value.toReal());
    });

    auto decoratedClient = decoration->window();
    connect(decoratedClient, &KDecoration3::DecoratedWindow::hasApplicationMenuChanged, this, &AppMenuButtonGroup::onHasApplicationMenuChanged);
    connect(decoratedClient, &KDecoration3::DecoratedWindow::applicationMenuChanged, this, &AppMenuButtonGroup::onApplicationMenuChanged);

    connect(this, &AppMenuButtonGroup::requestActivateOverflow, this, &AppMenuButtonGroup::triggerOverflow);

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
    if (m_searchMenu) {
        m_searchMenu->deleteLater();
    }

    // explicit destruction even
    // if it already is Qt::WA_DeleteOnClose,
    // deal whit the corner-case in which the window
    // is closed while the m_overflowMenu is open
    if (m_overflowMenu) {
        m_overflowMenu->deleteLater();
    }
}

void AppMenuButtonGroup::setupSearchMenu()
{
    m_searchMenu = new NavigableMenu(nullptr, m_menuStyle);
    m_searchLineEdit = new QLineEdit(m_searchMenu);
    m_searchLineEdit->setMinimumWidth(200);

    auto *searchAction = new QWidgetAction(m_searchMenu);
    searchAction->setDefaultWidget(m_searchLineEdit);
    m_searchMenu->addAction(searchAction);
    m_searchMenu->addSeparator();

    m_searchMenu->installEventFilter(this);

    connect(m_searchLineEdit, &QLineEdit::textChanged, m_searchDebounceTimer, qOverload<>(&QTimer::start));

    m_searchLineEdit->installEventFilter(this);
    m_searchLineEdit->setFocusPolicy(Qt::StrongFocus);
    m_searchLineEdit->setPlaceholderText(i18nd("plasma_applet_org.kde.plasma.appmenu", "Search") + QStringLiteral("…"));
    m_searchLineEdit->setClearButtonEnabled(false);
}

void AppMenuButtonGroup::repositionSearchMenu()
{
    if (!m_searchMenu || !m_searchMenu->isVisible()) {
        return;
    }

    auto *deco = qobject_cast<Decoration *>(decoration());
    KDecoration3::DecorationButton *button = m_searchButton;
    if (!deco || !button) {
        return;
    }

    KDecoration3::Positioner positioner;
    positioner.setAnchorRect(button->geometry());
    deco->popup(positioner, m_searchMenu);
    m_searchMenu->popup(m_searchMenu->pos()); // HACK without this the scrollbar remain even if not necessary
}

int AppMenuButtonGroup::currentIndex() const
{
    return m_currentIndex;
}

void AppMenuButtonGroup::setCurrentIndex(int set)
{
    if (m_currentIndex != set) {
        m_currentIndex = set;
        Q_EMIT currentIndexChanged();
    }
}

bool AppMenuButtonGroup::overflowing() const
{
    return m_overflowing;
}

void AppMenuButtonGroup::setOverflowing(bool set)
{
    if (m_overflowing != set) {
        m_overflowing = set;
        Q_EMIT overflowingChanged();
    }
}

bool AppMenuButtonGroup::hovered() const
{
    return m_hovered;
}

void AppMenuButtonGroup::setHovered(bool value)
{
    if (m_hovered != value) {
        m_hovered = value;
        Q_EMIT hoveredChanged(value);
    }
}

bool AppMenuButtonGroup::showing() const
{
    return m_showing;
}

void AppMenuButtonGroup::setShowing(bool value)
{
    if (m_showing != value) {
        m_showing = value;
        Q_EMIT showingChanged(value);
    }
}

bool AppMenuButtonGroup::alwaysShow() const
{
    return m_alwaysShow;
}

void AppMenuButtonGroup::setAlwaysShow(bool value)
{
    if (m_alwaysShow != value) {
        m_alwaysShow = value;
        Q_EMIT alwaysShowChanged(value);
    }
}

bool AppMenuButtonGroup::animationEnabled() const
{
    return m_animationEnabled;
}

void AppMenuButtonGroup::setAnimationEnabled(bool value)
{
    if (m_animationEnabled != value) {
        m_animationEnabled = value;
        Q_EMIT animationEnabledChanged(value);
    }
}

int AppMenuButtonGroup::animationDuration() const
{
    return m_animation->duration();
}

void AppMenuButtonGroup::setAnimationDuration(int value)
{
    if (m_animation->duration() != value) {
        m_animation->setDuration(value);
        Q_EMIT animationDurationChanged(value);
    }
}

qreal AppMenuButtonGroup::opacity() const
{
    return m_opacity;
}

void AppMenuButtonGroup::setOpacity(qreal value)
{
    if (m_opacity != value) {
        m_opacity = value;

        for (auto &tb : std::as_const(m_textButtons)) {
            if (tb)
                tb->setOpacity(m_opacity);
        }
        if (m_overflowButton) {
            m_overflowButton->setOpacity(m_opacity);
        }
        if (m_searchButton) {
            m_searchButton->setOpacity(m_opacity);
        }

        Q_EMIT opacityChanged(value);
    }
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

void AppMenuButtonGroup::resetButtons()
{
    if (buttons().isEmpty()) {
        return;
    }
    setCurrentIndex(-1);
    m_currentMenu = nullptr;
    m_lastResults.clear();
    m_lastSearchQuery.clear();
    m_actionTextCache.clear();
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
    if (!m_lastSearchQuery.isEmpty() && m_searchUiVisible) {
        filterMenu(m_lastSearchQuery);
    }
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
        const bool searchEnabled = deco->internalSettings()->integratedSearchEnabled();
        const bool searchStateMatches = (m_searchButton.isNull() == !searchEnabled);

        if (m_textButtons.count() == menuActionCount && !m_textButtons.isEmpty() && searchStateMatches) {
            m_actionTextCache.clear();
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

            if (wasSearchOpen && !m_lastSearchQuery.isEmpty()) {
                filterMenu(m_lastSearchQuery);
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
                m_overflowButton = new AppMenuOverflowButton(deco, m_overflowIndex, this);
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

void AppMenuButtonGroup::setHamburgerMenu(bool value)
{
    if (m_hamburgerMenu != value) {
        m_hamburgerMenu = value;
    }
}

void AppMenuButtonGroup::updateOverflow(QRectF availableRect)
{
    const qreal availableWidth = availableRect.width();

    qreal fixedWidth = 0;
    if (m_searchButton && m_searchButton->isVisible()) {
        fixedWidth += m_searchButton->geometry().width();
    }

    bool showOverflow = m_hamburgerMenu;
    qreal currentVisibleWidth = fixedWidth;

    if (m_hamburgerMenu) {
        for (auto &tb : std::as_const(m_textButtons)) {
            if (tb)
                tb->setVisible(false);
        }
        showOverflow = true;
    } else {
        // First pass: check if all enabled text buttons fit without overflow button.
        // We perform this pass without side effects to avoid layout thrashing in Qt.
        qreal totalTextWidth = 0;
        int enabledCount = 0;
        bool allFit = true;
        for (auto &tb : std::as_const(m_textButtons)) {
            if (tb && tb->isEnabled()) {
                totalTextWidth += tb->geometry().width();
                enabledCount++;
                if (fixedWidth + totalTextWidth > availableWidth) {
                    allFit = false;
                    break;
                }
            }
        }

        if (allFit && enabledCount > 0) {
            showOverflow = false;
            currentVisibleWidth += totalTextWidth;
            // Second pass: apply visibility
            for (auto &tb : std::as_const(m_textButtons)) {
                if (tb) {
                    tb->setVisible(tb->isEnabled());
                }
            }
        } else if (enabledCount > 0) {
            showOverflow = true;
            const qreal overflowBtnWidth = m_overflowButton ? m_overflowButton->geometry().width() : 0;
            qreal remainingWidth = availableWidth - fixedWidth - overflowBtnWidth;

            // Second pass: apply visibility and calculate final width
            bool fits = true;
            for (auto &tb : std::as_const(m_textButtons)) {
                if (!tb) {
                    continue;
                }
                if (fits && tb->isEnabled()) {
                    const qreal w = tb->geometry().width();
                    if (w <= remainingWidth) {
                        tb->setVisible(true);
                        currentVisibleWidth += w;
                        remainingWidth -= w;
                        continue;
                    }
                    fits = false;
                }
                tb->setVisible(false);
            }
        } else {
            showOverflow = false;
            for (auto &tb : std::as_const(m_textButtons)) {
                if (tb)
                    tb->setVisible(false);
            }
        }
    }

    if (m_overflowButton) {
        m_overflowButton->setVisible(showOverflow);
        if (showOverflow) {
            currentVisibleWidth += m_overflowButton->geometry().width();
        }
    }
    setOverflowing(showOverflow);

    if (m_visibleWidth != currentVisibleWidth) {
        m_visibleWidth = currentVisibleWidth;
        Q_EMIT menuUpdated();
    }
}

qreal AppMenuButtonGroup::visibleWidth() const
{
    return m_visibleWidth;
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
    m_currentMenu = menu;

    // 2. Calculate position and show the new menu. This must happen before hiding the old one to prevent flicker.
    if (auto navMenu = qobject_cast<NavigableMenu *>(menu)) {
        connect(navMenu, &NavigableMenu::hitLeft, this, &AppMenuButtonGroup::onHitLeft, Qt::UniqueConnection);
        connect(navMenu, &NavigableMenu::hitRight, this, &AppMenuButtonGroup::onHitRight, Qt::UniqueConnection);
    }
    if (menu != m_searchMenu) {
        menu->installEventFilter(this);
    }

    KDecoration3::Positioner positioner;
    positioner.setAnchorRect(button->geometry());
    deco->popup(positioner, menu);

    if (buttonIndex == m_searchIndex && m_searchLineEdit) {
        m_searchLineEdit->setFocus();
        m_searchUiVisible = true;
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
        if (m_searchMenu && oldMenu == m_searchMenu) {
            m_searchUiVisible = false;
        }
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
        if (m_searchMenu)
            m_searchMenu->hide();
        return;
    }
    if (!m_searchMenu) {
        setupSearchMenu();
    }
    popupMenu(m_searchMenu, m_searchIndex);
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

    auto *actionMenu = new NavigableMenu(nullptr, m_menuStyle);
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

void AppMenuButtonGroup::trigger(int buttonIndex)
{
    // The button is checked in popupMenu, but we need to check it here
    // for the case where the menu is not yet loaded.
    AppMenuButton *button = getAppMenuButton(buttonIndex);

    if (!button) {
        return;
    }

    if (buttonIndex == m_searchIndex) {
        handleSearchTrigger();
    } else if (buttonIndex == m_overflowIndex) {
        handleOverflowTrigger();
    } else {
        handleMenuButtonTrigger(buttonIndex);
    }
}

void AppMenuButtonGroup::triggerOverflow()
{
    trigger(m_overflowIndex);
}

// FIXME TODO doesn't work on submenu
bool AppMenuButtonGroup::eventFilter(QObject *watched, QEvent *event)
{
    // Event handling for the search bar's QLineEdit
    if (watched == m_searchLineEdit) {
        if (event->type() == QEvent::KeyPress) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);

            // Forward Left/Right key events to m_searchMenu when at the line boundaries
            if ((keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) && keyEvent->modifiers() == Qt::NoModifier) {
                bool atBoundary = (keyEvent->key() == Qt::Key_Left && m_searchLineEdit->cursorPosition() == 0)
                    || (keyEvent->key() == Qt::Key_Right && m_searchLineEdit->cursorPosition() == m_searchLineEdit->text().length());

                if (atBoundary) {
                    QApplication::sendEvent(m_searchMenu, keyEvent);
                    return true;
                }
                return false;
            }
        }
        return false;
    }

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

void AppMenuButtonGroup::unPressAllButtons()
{
    for (auto &tb : std::as_const(m_textButtons)) {
        if (tb)
            tb->forceUnpress();
    }
    if (m_overflowButton) {
        m_overflowButton->forceUnpress();
    }
    if (m_searchButton) {
        m_searchButton->forceUnpress();
    }
}

void AppMenuButtonGroup::updateShowing()
{
    setShowing(m_alwaysShow || m_hovered || isMenuOpen());
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

    if (menu != m_searchMenu) {
        menu->removeEventFilter(this);
    }

    if (menu == m_searchMenu && m_searchLineEdit) {
        m_searchLineEdit->clear();
        m_searchUiVisible = false;
        m_lastResults.clear();
        m_lastSearchQuery.clear();
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
    trigger(desiredIndex);
}

void AppMenuButtonGroup::onHitRight()
{
    int desiredIndex = findNextVisibleButtonIndex(m_currentIndex, true);
    trigger(desiredIndex);
}

void AppMenuButtonGroup::onShowingChanged(bool showing)
{
    if (m_animationEnabled) {
        const QAbstractAnimation::Direction dir = showing ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        if (m_animation->state() == QAbstractAnimation::Running && m_animation->direction() != dir) {
            m_animation->stop();
        }
        m_animation->setDirection(dir);
        if (m_animation->state() != QAbstractAnimation::Running) {
            m_animation->start();
        }
    } else {
        setOpacity(showing ? 1 : 0);
    }
}

void AppMenuButtonGroup::filterMenu(const QString &text)
{
    if (!m_searchMenu) {
        return;
    }
    m_lastSearchQuery = text;

    // Clear results if search text is too short
    if (text.length() < 3) {
        const auto actions = m_searchMenu->actions();
        if (actions.count() > 2) {
            for (int i = actions.count() - 1; i >= 2; --i) {
                QAction *action = actions.at(i);
                m_searchMenu->removeAction(action);
                action->deleteLater();
            }
        }
        m_lastResults.clear();
        repositionSearchMenu();

        if (text.isEmpty()) {
            m_searchLineEdit->setClearButtonEnabled(false);
            m_searchLineEdit->setPlaceholderText(i18nd("plasma_applet_org.kde.plasma.appmenu", "Search") + QStringLiteral("…"));
            return;
        }
        m_searchLineEdit->setClearButtonEnabled(true);
        return;
    } else {
        m_searchLineEdit->setClearButtonEnabled(true);
    }

    if (!m_appMenuModel) {
        return;
    }

    // Find results
    QList<SearchResult> results;
    if (m_appMenuModel) {
        QMenu *rootMenu = m_appMenuModel->menu();
        if (rootMenu) {
            QSet<QMenu *> visited;
            const auto *deco = qobject_cast<const Decoration *>(decoration());
            const bool ignoreTopLevel = deco && deco->internalSettings()->integratedSearchIgnoreTopLevel();
            const bool ignoreSubMenus = deco && deco->internalSettings()->integratedSearchIgnoreSubMenus();
            QStringList currentPath;
            QStringMatcher matcher(text, Qt::CaseInsensitive);
            searchMenu(rootMenu, matcher, results, visited, ignoreTopLevel, ignoreSubMenus, currentPath);
        }
    }

    // If results are the same as last time, do nothing to prevent the freeze.
    if (m_lastResults == results) {
        return;
    }

    m_searchMenu->setUpdatesEnabled(false);

    m_lastResults = results;

    // Clear previous results
    const auto actions = m_searchMenu->actions();
    for (int i = actions.count() - 1; i >= 2; --i) {
        QAction *action = actions.at(i);
        m_searchMenu->removeAction(action);
        action->deleteLater();
    }

    // Add new results
    const auto *deco = qobject_cast<const Decoration *>(decoration());
    if (!deco) {
        m_searchMenu->setUpdatesEnabled(true);
        return;
    }

    int resultCount = 0;
    for (const SearchResult &result : std::as_const(results)) {
        if (resultCount >= MAX_SEARCH_RESULTS) { // stop after 100 results
            break;
        }

        const ActionInfo &info = result.info;
        QAction *action = result.action;
        if (!info.isEffectivelyEnabled && !deco->internalSettings()->integratedSearchShowDisabledActions()) {
            continue;
        }
        QAction *newAction = new QAction(action->icon(), info.path, m_searchMenu);
        newAction->setEnabled(info.isEffectivelyEnabled);
        newAction->setCheckable(action->isCheckable());
        newAction->setChecked(action->isChecked());

        if (action->actionGroup() && action->actionGroup()->isExclusive()) {
            auto *group = new QActionGroup(newAction);
            group->setExclusive(true);
            group->addAction(newAction);
        }

        QPointer<QAction> safeAction = action;
        connect(newAction, &QAction::triggered, this, [safeAction, searchMenu = m_searchMenu]() {
            if (safeAction) {
                safeAction->trigger();
            }
            if (searchMenu) {
                searchMenu->hide();
            }
        });
        m_searchMenu->addAction(newAction);
        resultCount++;
    }

    repositionSearchMenu();
    m_searchMenu->setUpdatesEnabled(true);
    // qCDebug(category) << "[AppMenuButtonGroup] filterMenu(" << text << ") ended";
}

void AppMenuButtonGroup::onSubMenuReady(QMenu *menu)
{
    m_actionTextCache.clear();

    if (m_searchUiVisible && !m_lastSearchQuery.isEmpty()) {
        if (!m_searchDebounceTimer->isActive()) {
            m_searchDebounceTimer->start();
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
            trigger(buttonIndex);
        }
    }
}

void AppMenuButtonGroup::onSearchTimerTimeout()
{
    if (m_searchLineEdit) {
        filterMenu(m_searchLineEdit->text());
    }
}

QString AppMenuButtonGroup::getActionText(QAction *action) const
{
    if (!action) {
        return QString();
    }
    const QString rawText = action->text();
    auto it = m_actionTextCache.find(rawText);
    if (it != m_actionTextCache.end()) {
        return it.value();
    }
    const QString cleanedText = KLocalizedString::removeAcceleratorMarker(rawText.trimmed());
    m_actionTextCache.insert(rawText, cleanedText);
    return cleanedText;
}

void AppMenuButtonGroup::searchMenu(QMenu *menu,
                                    const QStringMatcher &matcher,
                                    QList<SearchResult> &results,
                                    QSet<QMenu *> &visited,
                                    bool ignoreTopLevel,
                                    bool ignoreSubMenus,
                                    QStringList &currentPath,
                                    bool isParentEnabled,
                                    bool parentMatched)
{
    if (results.size() >= MAX_SEARCH_RESULTS || !menu || visited.contains(menu)) {
        return;
    }
    visited.insert(menu);

    QAction *menuAction = menu->menuAction();
    bool isCurrentEnabled = isParentEnabled;
    bool addedToPath = false;
    bool currentMatched = parentMatched;

    if (menuAction) {
        if (!menuAction->isEnabled()) {
            isCurrentEnabled = false;
        }
        const QString menuText = getActionText(menuAction);
        if (!menuText.isEmpty()) {
            currentPath.append(menuText);
            addedToPath = true;

            if (!currentMatched && (!ignoreTopLevel || currentPath.size() > 1)) {
                if (matcher.indexIn(menuText) != -1) {
                    currentMatched = true;
                }
            }
        }
    }

    for (QAction *action : menu->actions()) {
        if (results.size() >= MAX_SEARCH_RESULTS) {
            break;
        }
        if (action->isSeparator()) {
            continue;
        }
        if (action->menu()) {
            searchMenu(action->menu(), matcher, results, visited, ignoreTopLevel, ignoreSubMenus, currentPath, isCurrentEnabled, currentMatched);
        } else {
            const QString itemText = getActionText(action);
            bool match = currentMatched;
            if (ignoreSubMenus) {
                match = matcher.indexIn(itemText) != -1;
            } else {
                // Check the text of the action
                if (!match && (!ignoreTopLevel || !currentPath.isEmpty())) {
                    if (matcher.indexIn(itemText) != -1) {
                        match = true;
                    }
                }
            }

            if (match) {
                ActionInfo info;
                info.label = itemText;
                info.isEffectivelyEnabled = isCurrentEnabled && action->isEnabled();

                currentPath.append(itemText);
                info.path = currentPath.join(QStringLiteral(" » "));
                info.searchablePath = (currentPath.size() > 1) ? currentPath.mid(1).join(QStringLiteral(" » ")) : itemText;
                currentPath.removeLast();

                results.append({action, info});
            }
        }
    }

    if (addedToPath) {
        currentPath.removeLast();
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
                trigger(appMenuButton->buttonIndex());
            }
        }
    }
}

} // namespace Breeze
