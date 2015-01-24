#ifndef breezepalettehelper_h
#define breezepalettehelper_h

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

#include <QObject>
#include <QPalette>
#include <QSet>
#include <QWidget>

namespace Breeze
{

    //* forward declaration
    class Helper;

    //* handle palette change
    class PaletteHelper: public QObject
    {

        Q_OBJECT

        public:

        //* constructor
        PaletteHelper( QObject*, Helper& );

        //* destructor
        virtual ~PaletteHelper( void );

        //* register widget
        virtual bool registerWidget( QWidget* );

        //* event filter
        virtual bool eventFilter( QObject*, QEvent* event );

        public Q_SLOTS:

        //* unregister widget
        void unregisterWidget( QObject* );

        //* adjust palettes
        void adjustPalettes( const QPalette& );

        protected:

        //* adjust widget palette
        bool adjustPalette( QWidget*, const QPalette& ) const;

        private:

        //* helper
        const Helper& _helper;

        //* dummy widget
        /** it is used to keep track of application palette changes */
        QWidget* _widget = nullptr;

        //* widget set
        QSet<QObject*> _registeredWidgets;

    };

}

#endif
