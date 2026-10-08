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
#include "appmenu/iconbutton.h"
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

namespace Klassy
{

class Decoration;
class AppMenuTextButton;
class AppMenuOverflowButton;
class AppMenuSearchButton;

enum class AppMenuBehaviour {
    AlwaysExpandOnHover,
    AlwaysTakeSpace,
    RevealOnHover,
    ReplaceTitleOnHover,
    SearchOnly,
};

enum class AppMenuPosition {
    Left,
    Center,
    CenterFullWidth,
    Right,
};

class AppMenuButtonGroup : public KDecoration3::DecorationButtonGroup
{
    Q_OBJECT

public:
    AppMenuButtonGroup(Decoration *decoration);
    ~AppMenuButtonGroup() override;

    void reconfigure();

    bool alwaysShow() const;

    inline bool takesSpace() const
    {
        return m_behaviour != AppMenuBehaviour::ReplaceTitleOnHover;
    };
    inline bool expandsOnHover() const
    {
        return m_behaviour == AppMenuBehaviour::AlwaysExpandOnHover || m_behaviour == AppMenuBehaviour::RevealOnHover;
    }
    inline bool risesOnHover() const
    {
        return m_behaviour == AppMenuBehaviour::ReplaceTitleOnHover;
    }

    QPointF visibleTopLeft() const;
    qreal visibleWidth() const;

    bool menuLoadedOnce() const;
    bool isWaitingForMenu() const;

    AppMenuModel *model()
    {
        return m_appMenuModel;
    }

    QRectF activeHoverArea()
    {
        return m_activeHoverArea;
    }
    void handleHoverMove(const QPointF &pos);

    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY animationDurationChanged)
    Q_PROPERTY(bool animationEnabled READ animationEnabled WRITE setAnimationEnabled NOTIFY animationEnabledChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(qreal expansionFraction READ expansionFraction WRITE setExpansionFraction NOTIFY expansionFractionChanged)
    Q_PROPERTY(bool hovered READ hovered WRITE setHovered NOTIFY hoveredChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(int overflowing READ overflowing WRITE setOverflowing NOTIFY overflowingChanged)
    Q_PROPERTY(AppMenuPosition position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool showing READ showing WRITE setShowing NOTIFY showingChanged)
    Q_PROPERTY(AppMenuBehaviour behaviour READ behaviour WRITE setBehaviour NOTIFY behaviourChanged)

    QVariantAnimation *animation() const
    {
        return m_animation;
    }

    int animationDuration() const
    {
        return m_animation->duration();
    }
    void setAnimationDuration(int value)
    {
        if (m_animation->duration() == value)
            return;
        m_animation->setDuration(value);
        Q_EMIT animationDurationChanged(value);
    }

    bool animationEnabled() const
    {
        return m_animationEnabled;
    }
    void setAnimationEnabled(bool value)
    {
        if (m_animationEnabled == value)
            return;
        m_animationEnabled = value;
        Q_EMIT animationEnabledChanged(value);
    }

    qreal expansionFraction() const
    {
        return m_expansionFraction;
    }
    void setExpansionFraction(qreal value)
    {
        if (qFuzzyCompare(m_expansionFraction, value))
            return;
        m_expansionFraction = value;
        Q_EMIT expansionFractionChanged(value);
    }

    void setHovered(bool value)
    {
        if (m_hovered == value)
            return;
        m_hovered = value;
        Q_EMIT hoveredChanged(value);
    }
    bool hovered() const
    {
        return m_hovered;
    }

    qreal opacity() const
    {
        return m_opacity;
    }
    void setOpacity(qreal value)
    {
        if (qFuzzyCompare(m_opacity, value))
            return;
        m_opacity = value;
        for (auto rawButton : buttons()) {
            if (auto button = qobject_cast<AppMenuButton *>(rawButton))
                button->setOpacity(m_opacity);
        }
        if (m_behaviour == AppMenuBehaviour::ReplaceTitleOnHover) {
            m_decoration->setCaptionOpacity(1 - value);
        }
        Q_EMIT opacityChanged(value);
    }

    void setOverflowing(bool set)
    {
        if (m_overflowing == set)
            return;
        m_overflowing = set;
        Q_EMIT overflowingChanged();
    }
    bool overflowing() const
    {
        return m_overflowing;
    }

    void setPosition(AppMenuPosition value)
    {
        if (m_position == value)
            return;
        m_position = value;
        Q_EMIT positionChanged(value);
    }
    AppMenuPosition position()
    {
        return m_position;
    };

    AppMenuBehaviour behaviour() const
    {
        return m_behaviour;
    }
    void setBehaviour(AppMenuBehaviour value)
    {
        if (m_behaviour == value)
            return;
        m_behaviour = value;
        Q_EMIT behaviourChanged(value);
    }

    qreal minimumWidth() const
    {
        return m_minimumWidth;
    }

    void updateAppMenuModel();
    void updateOverflow(QRectF availableRect);
    void updateAdjacencyFlags();
    void updateGeometry();
    void updateShowing();
    bool showing() const
    {
        return m_showing;
    }

    // Drag-from-buttons support
    void startDragMove(const QPoint &pos);
    void resetDragMove();
    bool dragMoveTick(const QPoint &pos);
    void sendFakeHoverEventAtLastKnownPosition();

private:
    void onMenuReadyForSearch();
    void onMenuAboutToHide();
    void onHitLeft();
    void onHitRight();
    void onHasApplicationMenuChanged(bool hasMenu);
    void onApplicationMenuChanged();
    void performDebouncedMenuUpdate();
    void onMenuUpdateThrottleTimeout();
    void onDelayedCacheTimerTimeout();
    void onHoverAnimationValueChanged(const QVariant &value);
    void onShowingChanged(bool hovered);
    void updateHoverAnimationState(bool hovered);
    void onSubMenuReady(QMenu *menu);

signals:
    void menuUpdated();
    void requestActivateOverflow();

    void animationEnabledChanged(bool);
    void animationDurationChanged(int);
    void currentIndexChanged();
    void expansionFractionChanged(qreal);
    void geometryAnimationChanged(qreal);
    void hoveredChanged(bool);
    void opacityChanged(qreal);
    void positionChanged(AppMenuPosition);
    void showingChanged(bool);
    void behaviourChanged(AppMenuBehaviour);
    void overflowingChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setCurrentIndex(int set)
    {
        if (m_currentIndex == set)
            return;
        m_currentIndex = set;
        Q_EMIT currentIndexChanged();
    }
    int currentIndex() const
    {
        return m_currentIndex;
    }

    void setShowing(bool value)
    {
        if (m_showing == value)
            return;
        m_showing = value;
        Q_EMIT showingChanged(value);
    }

    bool isMenuOpen() const;

    KDecoration3::DecorationButton *buttonAt(QPoint pos) const;

    void unpressAllButtons();

    void trigger(int index, bool immediateTransition);

    void resetButtons();
    AppMenuButton *getAppMenuButton(int index) const;
    int findNextVisibleButtonIndex(int currentIndex, bool forward) const;

    void popupMenu(QMenu *menu, int buttonIndex);
    void handleSearchTrigger();
    void handleOverflowTrigger();
    void handleMenuButtonTrigger(int buttonIndex);

    Decoration *m_decoration;
    AppMenuModel *m_appMenuModel;
    QPoint m_pressedPoint = QPoint(-1, -1);
    int m_currentIndex = -1;
    int m_overflowIndex = -1;
    int m_searchIndex = -1;
    bool m_overflowing = false;
    QRectF m_activeHoverArea;
    bool m_hovered = false;
    bool m_showing = true;
    bool m_animationEnabled = true;
    qreal m_expansionFraction = 0;
    AppMenuBehaviour m_behaviour = AppMenuBehaviour::AlwaysExpandOnHover;
    AppMenuPosition m_position = AppMenuPosition::Left;
    QVariantAnimation *m_animation;
    qreal m_opacity = 1;
    qreal m_minimumWidth = 0;
    qreal m_maximumWidth = 0;
    QPointer<QMenu> m_currentMenu;
    int m_buttonIndexWaitingForPopup = -1;
    int m_buttonIndexOfMenuToCache = -1;

    QPointer<QMenu> m_overflowMenu;
    QTimer *m_menuUpdateDebounceTimer;
    QTimer *m_delayedCacheTimer;
    QTimer *m_resetTimer;
    QTimer *m_menuLoadFallbackTimer;

    bool m_immediateTransition = false;

    bool m_isMenuUpdateThrottled = false;
    bool m_pendingMenuUpdate = false;
    bool m_menuLoadedOnce = false;

    QList<QPointer<AppMenuTextButton>> m_textButtons;
    QPointer<AppMenuIconButton> m_overflowButton;
    QPointer<AppMenuSearchButton> m_searchButton;

    QPointer<KDecoration3::DecorationButton> m_hoveredButton = nullptr;

    friend class AppMenuButton;
};

} // namespace Klassy
