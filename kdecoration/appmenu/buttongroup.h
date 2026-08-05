/*
 * Copyright (C) 2025 Guido Iodice <guido[dot]iodice[at]gmail[dot]com>
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
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

#pragma once

// own
#include "appmenu/button.h"
#include "appmenu/menustyle.h"
#include "appmenu/model.h"

// KDecoration
#include <KDecoration3/DecorationButton>
#include <KDecoration3/DecorationButtonGroup>

// Qt
#include <QHash>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>

class QTimer;
class QStringMatcher;
class QVariantAnimation;

namespace Breeze
{

class Decoration;
class AppMenuTextButton;
class AppMenuOverflowButton;
class AppMenuSearchButton;

enum class AppMenuStyle {
    Always,
    RevealOnHover,
    ReplaceTitleOnHover,
    SearchOnly,
    Disabled,
};

class AppMenuButtonGroup : public KDecoration3::DecorationButtonGroup
{
    Q_OBJECT

public:
    AppMenuButtonGroup(Decoration *decoration);
    ~AppMenuButtonGroup() override;

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int overflowing READ overflowing WRITE setOverflowing NOTIFY overflowingChanged)
    Q_PROPERTY(bool hovered READ hovered WRITE setHovered NOTIFY hoveredChanged)
    Q_PROPERTY(bool showing READ showing WRITE setShowing NOTIFY showingChanged)
    Q_PROPERTY(bool animationEnabled READ animationEnabled WRITE setAnimationEnabled NOTIFY animationEnabledChanged)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY animationDurationChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(AppMenuStyle style READ style WRITE setStyle NOTIFY styleChanged)

    bool hovered() const;
    void setHovered(bool value);

    bool alwaysShow() const;

    bool animationEnabled() const;
    void setAnimationEnabled(bool value);

    int animationDuration() const;
    void setAnimationDuration(int duration);

    qreal opacity() const;
    void setOpacity(qreal value);

    AppMenuStyle style() const;
    void setStyle(AppMenuStyle value);

    inline bool takesSpace() const
    {
        return m_style != AppMenuStyle::ReplaceTitleOnHover;
    };

    qreal visibleWidth() const;

    bool menuLoadedOnce() const;
    bool isWaitingForMenu() const;

    void handleHoverMove(const QPointF &pos);

public:
    void updateAppMenuModel();
    void updateOverflow(QRectF availableRect);
    void updateShowing();

private:
    void onMenuReadyForSearch();
    void triggerOverflow();
    void onMenuAboutToHide();
    void onHitLeft();
    void onHitRight();
    void onHasApplicationMenuChanged(bool hasMenu);
    void onApplicationMenuChanged();
    void performDebouncedMenuUpdate();
    void onMenuUpdateThrottleTimeout();
    void onDelayedCacheTimerTimeout();
    void onShowingChanged(bool hovered);
    void updateHoverAnimationState(bool hovered);
    void filterMenu(const QString &text);
    void onSearchTimerTimeout();
    void onSubMenuReady(QMenu *menu);

signals:
    void menuUpdated();
    void requestActivateOverflow();

    void currentIndexChanged();
    void overflowingChanged();
    void hoveredChanged(bool);
    void showingChanged(bool);
    void animationEnabledChanged(bool);
    void animationDurationChanged(int);
    void opacityChanged(qreal);
    void styleChanged(AppMenuStyle);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    int currentIndex() const;
    void setCurrentIndex(int set);

    bool overflowing() const;
    void setOverflowing(bool set);

    bool showing() const;
    void setShowing(bool value);

    bool isMenuOpen() const;

    KDecoration3::DecorationButton *buttonAt(QPoint pos) const;

    void unPressAllButtons();

    void trigger(int index);

    struct ActionInfo {
        QString path;
        QString searchablePath;
        QString label;
        bool isEffectivelyEnabled;
    };

    struct SearchResult {
        QAction *action;
        ActionInfo info;

        bool operator==(const SearchResult &other) const
        {
            return action == other.action && info.isEffectivelyEnabled == other.info.isEffectivelyEnabled && info.path == other.info.path
                && (!action || (action->isChecked() == other.action->isChecked() && action->isCheckable() == other.action->isCheckable()));
        }
    };

    void resetButtons();
    QString getActionText(QAction *action) const;
    void setupSearchMenu();
    void repositionSearchMenu();
    void searchMenu(QMenu *menu,
                    const QStringMatcher &matcher,
                    QList<SearchResult> &results,
                    QSet<QMenu *> &visited,
                    bool ignoreTopLevel,
                    bool ignoreSubMenus,
                    QStringList &currentPath,
                    bool isParentEnabled = true,
                    bool parentMatched = false);
    AppMenuButton *getAppMenuButton(int index) const;
    int findNextVisibleButtonIndex(int currentIndex, bool forward) const;

    void popupMenu(QMenu *menu, int buttonIndex);
    void handleSearchTrigger();
    void handleOverflowTrigger();
    void handleMenuButtonTrigger(int buttonIndex);

    Decoration *m_decoration;
    QPointer<AppMenuMenuStyle> m_menuStyle;
    AppMenuModel *m_appMenuModel;
    int m_currentIndex;
    int m_overflowIndex;
    int m_searchIndex;
    bool m_overflowing;
    bool m_hovered;
    bool m_showing;
    bool m_animationEnabled;
    AppMenuStyle m_style;
    QVariantAnimation *m_animation;
    qreal m_opacity;
    qreal m_visibleWidth;
    QPointer<QMenu> m_currentMenu;
    int m_buttonIndexWaitingForPopup = -1;
    int m_buttonIndexOfMenuToCache = -1;

    QPointer<QMenu> m_searchMenu;
    QPointer<QMenu> m_overflowMenu;
    QPointer<QLineEdit> m_searchLineEdit;
    QTimer *m_searchDebounceTimer;
    QTimer *m_menuUpdateDebounceTimer;
    QTimer *m_delayedCacheTimer;
    QTimer *m_resetTimer;
    QTimer *m_menuLoadFallbackTimer;

    bool m_searchUiVisible = false;

    bool m_isMenuUpdateThrottled = false;
    bool m_pendingMenuUpdate = false;
    bool m_menuLoadedOnce = false;
    QString m_lastSearchQuery;
    QList<SearchResult> m_lastResults;

    QList<QPointer<AppMenuTextButton>> m_textButtons;
    QPointer<AppMenuOverflowButton> m_overflowButton;
    QPointer<AppMenuSearchButton> m_searchButton;

    mutable QHash<QString, QString> m_actionTextCache;

    QPointer<KDecoration3::DecorationButton> m_hoveredButton = nullptr;

    friend class AppMenuButton;
    friend class Decoration;
};

} // namespace Breeze
