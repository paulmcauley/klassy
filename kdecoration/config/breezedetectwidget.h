#ifndef breezedetectwidget_h
#define breezedetectwidget_h

//////////////////////////////////////////////////////////////////////////////
// breezedetectwidget.h
// Note: this class is a stripped down version of
// /kdebase/workspace/kwin/kcmkwin/kwinrules/detectwidget.h
// SPDX-FileCopyrightText: 2004 Lubos Lunak <l.lunak@kde.org>

// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
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
    explicit DetectDialog(QWidget *);

    //* read window properties or select one from mouse grab
    void detect(WId window);

    //* selected class
    QByteArray selectedClass() const;

    //* window information
    const KWindowInfo &windowInfo() const
    {
        return *(m_info.data());
    }

    //* exception type
    InternalSettings::EnumExceptionWindowPropertyType exceptionWindowPropertyType() const
    {
        if (m_ui.windowClassCheckBox->isChecked())
            return InternalSettings::ExceptionWindowClassName;
        else if (m_ui.windowTitleCheckBox->isChecked())
            return InternalSettings::ExceptionWindowTitle;
        else
            return InternalSettings::ExceptionWindowClassName;
    }

Q_SIGNALS:

    void detectionDone(bool);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    //* select window from grab
    void selectWindow();

    //* read window properties
    void readWindow(WId window);

    //* find window under cursor
    WId findWindow();

    //* execute
    void executeDialog();

    //* ui
    Ui::BreezeDetectWidget m_ui;

    //* invisible dialog used to grab mouse
    QDialog *m_grabber = nullptr;

    //* current window information
    QScopedPointer<KWindowInfo> m_info;

    //* wm state atom
    quint32 m_wmStateAtom = 0;
};

} // namespace

#endif
