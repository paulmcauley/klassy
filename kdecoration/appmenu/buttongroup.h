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

namespace Breeze
{

class Decoration;
class AppMenuTextButton;
class AppMenuOverflowButton;
class AppMenuSearchButton;

enum class AppMenuStyle {
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

enum class AppMenuUnisonHovering {
    Disabled,
    Separate,
    Together,
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
        return m_style != AppMenuStyle::ReplaceTitleOnHover;
    };
    inline bool expandsOnHover() const
    {
        return m_style == AppMenuStyle::AlwaysExpandOnHover || m_style == AppMenuStyle::RevealOnHover;
    }

    qreal visibleWidth() const;

    bool menuLoadedOnce() const;
    bool isWaitingForMenu() const;

    AppMenuModel *model()
    {
        return m_appMenuModel;
    }

    void handleHoverMove(const QPointF &pos);

    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY animationDurationChanged)
    Q_PROPERTY(bool animationEnabled READ animationEnabled WRITE setAnimationEnabled NOTIFY animationEnabledChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(qreal expansionPercent READ expansionPercent WRITE setExpansionPercent NOTIFY expansionPercentChanged)
    Q_PROPERTY(bool hovered READ hovered WRITE setHovered NOTIFY hoveredChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(int overflowing READ overflowing WRITE setOverflowing NOTIFY overflowingChanged)
    Q_PROPERTY(AppMenuPosition position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool showing READ showing WRITE setShowing NOTIFY showingChanged)
    Q_PROPERTY(AppMenuStyle style READ style WRITE setStyle NOTIFY styleChanged)
    Q_PROPERTY(bool unisonHovered READ unisonHovered WRITE setUnisonHovered NOTIFY unisonHoveredChanged)
    Q_PROPERTY(AppMenuUnisonHovering unisonHoveringType READ unisonHoveringType WRITE setUnisonHoveringType NOTIFY unisonHoveringTypeChanged)

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

    qreal expansionPercent() const
    {
        return m_expansionPercent;
    }
    void setExpansionPercent(qreal value)
    {
        if (qFuzzyCompare(m_expansionPercent, value))
            return;
        m_expansionPercent = value;
        Q_EMIT expansionPercentChanged(value);
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
        if (m_style == AppMenuStyle::ReplaceTitleOnHover) {
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

    AppMenuStyle style() const
    {
        return m_style;
    }
    void setStyle(AppMenuStyle value)
    {
        if (m_style == value)
            return;
        m_style = value;
        Q_EMIT styleChanged(value);
    }

    bool unisonHovered() const
    {
        return m_unisonHovered;
    }
    void setUnisonHovered(bool value)
    {
        if (m_unisonHovered == value)
            return;
        m_unisonHovered = value;
        Q_EMIT(unisonHoveredChanged(value));
    }

    AppMenuUnisonHovering unisonHoveringType() const
    {
        return m_unisonHoveringType;
    }
    void setUnisonHoveringType(AppMenuUnisonHovering value)
    {
        if (m_unisonHoveringType == value)
            return;
        if (value == AppMenuUnisonHovering::Disabled)
            setUnisonHovered(false);
        m_unisonHoveringType = value;
        Q_EMIT unisonHoveringTypeChanged(value);
    }

    qreal minimumWidth() const
    {
        return m_minimumWidth;
    }

public:
    void updateAppMenuModel();
    void updateOverflow(QRectF availableRect);
    void updateAdjacencyFlags();
    void updateGeometry();
    void updateShowing();

    // Drag-from-buttons support
    void startDragMove(const QPoint &pos);
    void resetDragMove();
    bool dragMoveTick(const QPoint &pos);

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
    void expansionPercentChanged(qreal);
    void hoveredChanged(bool);
    void opacityChanged(qreal);
    void positionChanged(AppMenuPosition);
    void showingChanged(bool);
    void styleChanged(AppMenuStyle);
    void overflowingChanged();
    void unisonHoveringTypeChanged(AppMenuUnisonHovering);
    void unisonHoveredChanged(bool);

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
    bool showing() const
    {
        return m_showing;
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
    QPoint m_pressedPoint;
    int m_currentIndex = -1;
    int m_overflowIndex = -1;
    int m_searchIndex = -1;
    bool m_overflowing = false;
    bool m_hovered = false;
    bool m_showing = true;
    bool m_animationEnabled = true;
    bool m_unisonHovered = false;
    qreal m_expansionPercent = 0;
    AppMenuStyle m_style = AppMenuStyle::AlwaysExpandOnHover;
    AppMenuPosition m_position = AppMenuPosition::Left;
    AppMenuUnisonHovering m_unisonHoveringType = AppMenuUnisonHovering::Disabled;
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

} // namespace Breeze
