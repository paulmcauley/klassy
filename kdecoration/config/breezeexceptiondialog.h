#ifndef breezeexceptiondialog_h
#define breezeexceptiondialog_h
//////////////////////////////////////////////////////////////////////////////
// breezeexceptiondialog.h
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "ui_breezeexceptiondialog.h"
#include "breeze.h"

#include <QCheckBox>
#include <QMap>

namespace Breeze
{

    class DetectDialog;

    //* breeze exceptions list
    class ExceptionDialog: public QDialog
    {

        Q_OBJECT

        public:

        //* constructor
        explicit ExceptionDialog( QWidget* parent );

        //* destructor
        virtual ~ExceptionDialog()
        {}

        //* set exception
        void setException( InternalSettingsPtr );

        //* save exception
        void save();

        //* true if changed
        virtual bool isChanged() const
        { return m_changed; }

        Q_SIGNALS:

        //* emitted when changed
        void changed( bool );

        protected:

        //* set changed state
        virtual void setChanged( bool value )
        {
            m_changed = value;
            emit changed( value );
        }

        protected Q_SLOTS:

        //* check whether configuration is changed and emit appropriate signal if yes
        virtual void updateChanged();

        private Q_SLOTS:

        //* select window properties from grabbed pointers
        void selectWindowProperties();

        //* read properties of selected window
        void readWindowProperties( bool );

        private:

        //* map mask and checkbox
        using CheckBoxMap=QMap< ExceptionMask, QCheckBox*>;

        Ui::BreezeExceptionDialog m_ui;

        //* map mask and checkbox
        CheckBoxMap m_checkboxes;

        //* internal exception
        InternalSettingsPtr m_exception;

        //* detection dialog
        DetectDialog* m_detectDialog = nullptr;

        //* changed state
        bool m_changed = false;

    };

}

#endif
