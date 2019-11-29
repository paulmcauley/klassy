#ifndef BREEZE_DECORATION_H
#define BREEZE_DECORATION_H

/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "breeze.h"
#include "breezesettings.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>

#include <QPalette>
#include <QVariant>

class QVariantAnimation;

namespace KDecoration2
{
    class DecorationButton;
    class DecorationButtonGroup;
}

namespace Breeze
{
    class SizeGrip;
    class Decoration : public KDecoration2::Decoration
    {
        Q_OBJECT

        public:

        //* constructor
        explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());

        //* destructor
        virtual ~Decoration();

        //* paint
        void paint(QPainter *painter, const QRect &repaintRegion) override;

        //* internal settings
        InternalSettingsPtr internalSettings() const
        { return m_internalSettings; }

        //* caption height
        int captionHeight() const;

        //* button height
        int buttonHeight() const;

        //*@name active state change animation
        //@{
        void setOpacity( qreal );

        qreal opacity() const
        { return m_opacity; }

        //@}

        //*@name colors
        //@{
        QColor titleBarColor() const;
        QColor outlineColor() const;
        QColor fontColor() const;
        //@}

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

        public Q_SLOTS:
        void init() override;

        private Q_SLOTS:
        void reconfigure();
        void recalculateBorders();
        void updateButtonsGeometry();
        void updateButtonsGeometryDelayed();
        void updateTitleBar();
        void updateAnimationState();
        void updateSizeGripVisibility();

        private:

        //* return the rect in which caption will be drawn
        QPair<QRect,Qt::Alignment> captionRect() const;

        void createButtons();
        void paintTitleBar(QPainter *painter, const QRect &repaintRegion);
        void createShadow();

        //*@name border size
        //@{
        int borderSize(bool bottom = false) const;
        inline bool hasBorders() const;
        inline bool hasNoBorders() const;
        inline bool hasNoSideBorders() const;
        //@}

        //*@name size grip
        //@{
        void createSizeGrip();
        void deleteSizeGrip();
        SizeGrip* sizeGrip() const
        { return m_sizeGrip; }
        //@}

        InternalSettingsPtr m_internalSettings;
        KDecoration2::DecorationButtonGroup *m_leftButtons = nullptr;
        KDecoration2::DecorationButtonGroup *m_rightButtons = nullptr;

        //* size grip widget
        SizeGrip *m_sizeGrip = nullptr;

        //* active state change animation
        QVariantAnimation *m_animation;

        //* active state change opacity
        qreal m_opacity = 0;

    };

    bool Decoration::hasBorders() const
    {
        if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() > InternalSettings::BorderNoSides;
        else return settings()->borderSize() > KDecoration2::BorderSize::NoSides;
    }

    bool Decoration::hasNoBorders() const
    {
        if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() == InternalSettings::BorderNone;
        else return settings()->borderSize() == KDecoration2::BorderSize::None;
    }

    bool Decoration::hasNoSideBorders() const
    {
        if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() == InternalSettings::BorderNoSides;
        else return settings()->borderSize() == KDecoration2::BorderSize::NoSides;
    }

    bool Decoration::isMaximized() const
    { return client().data()->isMaximized() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isMaximizedHorizontally() const
    { return client().data()->isMaximizedHorizontally() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isMaximizedVertically() const
    { return client().data()->isMaximizedVertically() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isLeftEdge() const
    { return (client().data()->isMaximizedHorizontally() || client().data()->adjacentScreenEdges().testFlag( Qt::LeftEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isRightEdge() const
    { return (client().data()->isMaximizedHorizontally() || client().data()->adjacentScreenEdges().testFlag( Qt::RightEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isTopEdge() const
    { return (client().data()->isMaximizedVertically() || client().data()->adjacentScreenEdges().testFlag( Qt::TopEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isBottomEdge() const
    { return (client().data()->isMaximizedVertically() || client().data()->adjacentScreenEdges().testFlag( Qt::BottomEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::hideTitleBar() const
    { return m_internalSettings->hideTitleBar() && !client().data()->isShaded(); }

}

#endif
