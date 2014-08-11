#ifndef breezestyleconfig_h
#define breezestyleconfig_h

//////////////////////////////////////////////////////////////////////////////
// breezestyleconfig.h
// -------------------
//
// Copyright (C) 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "ui_breezestyleconfig.h"

namespace Breeze
{

    class StyleConfig: public QWidget, Ui::BreezeStyleConfig
    {

        Q_OBJECT

        public:

        //! constructor
        explicit StyleConfig(QWidget*);

        //! destructor
        virtual ~StyleConfig( void )
        {}

        Q_SIGNALS:

        //! emmited whenever one option is changed.
        void changed(bool);

        public Q_SLOTS:

        //! save current state
        void save( void );

        //! restore all default values
        void defaults( void );

        //! reset to saved configuration
        void reset( void );

        protected Q_SLOTS:

        //! update modified state when option is checked/unchecked
        void updateChanged( void );

        //! update options enable state based on selected drag mode
        void windowDragModeChanged( int );

        protected:

        //! load setup from config data
        void load( void );

        int windowDragMode( void ) const;

    };

}
#endif
