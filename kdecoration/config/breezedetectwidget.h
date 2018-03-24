#ifndef breezedetectwidget_h
#define breezedetectwidget_h

//////////////////////////////////////////////////////////////////////////////
// breezedetectwidget.h
// Note: this class is a stripped down version of
// /kdebase/workspace/kwin/kcmkwin/kwinrules/detectwidget.h
// Copyright (c) 2004 Lubos Lunak <l.lunak@kde.org>

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

#include "breezesettings.h"
#include "ui_breezedetectwidget.h"

#include <QByteArray>
#include <QCheckBox>
#include <QDialog>
#include <QEvent>
#include <QLabel>

#include <kwindowsystem.h>

namespace Breeze
{

    class DetectDialog : public QDialog
    {

        Q_OBJECT

        public:

        //* constructor
        explicit DetectDialog( QWidget* );

        //* read window properties or select one from mouse grab
        void detect( WId window );

        //* selected class
        QByteArray selectedClass() const;

        //* window information
        const KWindowInfo& windowInfo() const
        { return *(m_info.data()); }

        //* exception type
        InternalSettings::EnumExceptionType exceptionType() const
        {
            if( m_ui.windowClassCheckBox->isChecked() ) return InternalSettings::ExceptionWindowClassName;
            else if( m_ui.windowTitleCheckBox->isChecked() ) return InternalSettings::ExceptionWindowTitle;
            else return InternalSettings::ExceptionWindowClassName;
        }

        Q_SIGNALS:

        void detectionDone( bool );

        protected:

        bool eventFilter( QObject* o, QEvent* e ) override;

        private:

        //* select window from grab
        void selectWindow();

        //* read window properties
        void readWindow( WId window );

        //* find window under cursor
        WId findWindow();

        //* execute
        void executeDialog();

        //* ui
        Ui::BreezeDetectWidget m_ui;

        //* invisible dialog used to grab mouse
        QDialog* m_grabber = nullptr;

        //* current window information
        QScopedPointer<KWindowInfo> m_info;

        //* wm state atom
        quint32 m_wmStateAtom = 0;

    };

} // namespace

#endif
