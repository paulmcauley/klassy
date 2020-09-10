#ifndef breezeexceptiondialog_h
#define breezeexceptiondialog_h
//////////////////////////////////////////////////////////////////////////////
// breezeexceptiondialog.h
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
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
