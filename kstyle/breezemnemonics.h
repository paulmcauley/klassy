#ifndef breezemnemonics_h
#define breezemnemonics_h

//////////////////////////////////////////////////////////////////////////////
// breezemnemonics.h
// enable/disable mnemonics display
// -------------------
//
// Copyright (C) 2011 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License version 2 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////

#include <QEvent>
#include <QObject>
#include <QApplication>

#include "breezestyleconfigdata.h"

namespace Breeze
{

    class Mnemonics: public QObject
    {

        public:

        //! constructor
        explicit Mnemonics( QObject* parent ):
            QObject( parent ),
            _enabled( true )
            {}

        //! destructor
        virtual ~Mnemonics( void )
        {}

        //! set mode
        void setMode( int );

        //! event filter
        virtual bool eventFilter( QObject*, QEvent* );

        //! true if mnemonics are enabled
        const bool& enabled( void ) const
        { return _enabled; }

        //! alignment flag
        int textFlags( void ) const
        { return _enabled ? Qt::TextShowMnemonic : Qt::TextHideMnemonic; }

        protected:

        //! set enable state
        void setEnabled( bool );

        private:

        //! enable state
        bool _enabled;

    };

}

#endif
