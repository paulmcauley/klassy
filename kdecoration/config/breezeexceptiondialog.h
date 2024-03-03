//////////////////////////////////////////////////////////////////////////////
// breezeexceptiondialog.h
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "breeze.h"
#include "ui_breezeexceptiondialog.h"

#include <KSharedConfig>
#include <QCheckBox>
#include <QMap>

namespace Breeze
{

class DetectDialog;

//* breeze exceptions list
class ExceptionDialog : public QDialog
{
    Q_OBJECT

public:
    //* constructor
    explicit ExceptionDialog(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent);

    //* destructor
    virtual ~ExceptionDialog()
    {
    }

    //* set exception
    void setException(InternalSettingsPtr);

    //* save exception
    void save();

    //* true if changed
    virtual bool isChanged() const
    {
        return m_changed;
    }

Q_SIGNALS:

    //* emitted when changed
    void changed(bool);

protected:
    //* set changed state
    virtual void setChanged(bool value)
    {
        m_changed = value;
        Q_EMIT changed(value);
    }

protected Q_SLOTS:

    //* check whether configuration is changed and emit appropriate signal if yes
    virtual void updateChanged();

private Q_SLOTS:

    //* select window properties from grabbed pointers
    void selectWindowProperties();

    //* read properties of selected window
    void readWindowProperties(bool);

    void onOpaqueTitleBarToggled(bool);

private:
    Ui::BreezeExceptionDialog m_ui;

    //* internal exception
    InternalSettingsPtr m_exception;

    //* kconfiguration object
    KSharedConfig::Ptr m_configuration;

    //* presets kconfiguration object
    KSharedConfig::Ptr m_presetsConfiguration;

    //* detection dialog
    DetectDialog *m_detectDialog = nullptr;

    //* changed state
    bool m_changed = false;

    //* if is in the process of loading
    bool m_loading = false;
};
}
