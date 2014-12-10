#ifndef BREEZE_BUTTONS_H
#define BREEZE_BUTTONS_H

/*
* Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
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
#include <KDecoration2/DecorationButton>
#include "breezedecoration.h"

#include <QHash>
#include <QImage>
#include <QPropertyAnimation>

namespace Breeze
{

    class Button : public KDecoration2::DecorationButton
    {
        Q_OBJECT

        //* declare active state opacity
        Q_PROPERTY( qreal opacity READ opacity WRITE setOpacity )

        public:

        //* constructor
        explicit Button(QObject *parent, const QVariantList &args);

        //* destructor
        virtual ~Button();

        //* render
        void paint(QPainter *painter, const QRect &repaintRegion) override;

        //* create
        static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

        //* standalone buttons
        bool isStandAlone() const { return m_standalone; }

        //*@name active state change animation
        //@{
        void setOpacity( qreal value )
        {
            if( m_opacity == value ) return;
            m_opacity = value;
            update();
        }

        qreal opacity( void ) const
        { return m_opacity; }

        //@}

        private Q_SLOTS:
        void updateAnimationState(bool);

        private:

        //* private constructor
        explicit Button(KDecoration2::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

        //* draw button icon
        void drawIcon( QPainter *) const;

        //*@name colors
        //@{
        QColor foregroundColor( void ) const;
        QColor backgroundColor( void ) const;
        //@}

        bool m_standalone = false;

        //* active state change animation
        QPropertyAnimation *m_animation;

        //* active state change opacity
        qreal m_opacity;
    };

} // namespace

#endif
