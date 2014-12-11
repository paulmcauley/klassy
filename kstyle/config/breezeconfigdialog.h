#ifndef breezeconfigdialog_h
#define breezeconfigdialog_h

//////////////////////////////////////////////////////////////////////////////
// breezeconfigdialog.h
// breeze configuration dialog
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "ui_breezeconfigdialog.h"

namespace Breeze
{

    class ConfigDialog: public QDialog, Ui::BreezeConfigDialog
    {
        Q_OBJECT

        public:

        //! constructor
        explicit ConfigDialog( QWidget* parent = 0 );

        //! destructor
        virtual ~ConfigDialog( void )
        {}

        Q_SIGNALS:

        //! save local changes
        void pluginSave( void );

        public Q_SLOTS:

        //! save local changes
        virtual void save( void );

        protected Q_SLOTS:

        // update decoration changed state
        void updateStyleChanged( bool state = true )
        { _styleChanged = state; }

        // update decoration changed state
        void updateDecorationChanged( bool state = true )
        { _decorationChanged = state; }

        // handle configuration modifications
        virtual void updateChanged( void );

        // update window title based on selected page
        virtual void updateWindowTitle( KPageWidgetItem* = 0 );

        private:

        //! true if configuration changed
        bool changed( void ) const
        { return _styleChanged || _decorationChanged; }

        //! load style config widget from plugin
        KPageWidgetItem* loadStyleConfig( void );

        //! load decoration config widget from plugin
        KPageWidgetItem* loadDecorationConfig( void );

        //! style plugin widget
        QObject *_stylePluginObject;

        //! decoration plugin widget
        QObject *_decorationPluginObject;

        bool _styleChanged;
        bool _decorationChanged;

    };

}

#endif
