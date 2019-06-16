#ifndef breezewidgetexplorer_h
#define breezewidgetexplorer_h

/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include <QEvent>
#include <QObject>
#include <QMap>
#include <QSet>
#include <QWidget>

namespace Breeze
{

    //* print widget's and parent's information on mouse click
    class WidgetExplorer: public QObject
    {

        Q_OBJECT

        public:

        //* constructor
        explicit WidgetExplorer( QObject* );

        //* enable
        bool enabled() const;

        //* enable
        void setEnabled( bool );

        //* widget rects
        void setDrawWidgetRects( bool value )
        { _drawWidgetRects = value; }

        //* event filter
        bool eventFilter( QObject*, QEvent* ) override;

        protected:

        //* event type
        QString eventType( const QEvent::Type& ) const;

        //* print widget information
        QString widgetInformation( const QWidget* ) const;

        private:

        //* enable state
        bool _enabled = false;

        //* widget rects
        bool _drawWidgetRects = false;

        //* map event types to string
        QMap<QEvent::Type, QString > _eventTypes;

    };

}

#endif
