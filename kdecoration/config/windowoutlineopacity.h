#ifndef WINDOWOUTLINEOPACITY_H
#define WINDOWOUTLINEOPACITY_H

//////////////////////////////////////////////////////////////////////////////
// windowoutlineopacity.h
// -------------------
//
// SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breeze.h"
#include "breezesettings.h"
#include "ui_windowoutlineopacity.h"
#include <QDialog>

namespace Breeze
{

class WindowOutlineOpacity : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit WindowOutlineOpacity(QWidget *parent = nullptr);
    ~WindowOutlineOpacity();

    void load();
    void save(const bool reloadKwinConfig = true);
    void defaults();

private slots:
    void accept() override;
    void updateChanged();
    void saveAndReloadKWinConfig()
    {
        save(true);
    }
    void setApplyButtonState(const bool on);

signals:
    void changed(bool);

private:
    void setChanged(bool value);

    Ui_WindowOutlineOpacity *m_ui;

    InternalSettingsPtr m_internalSettings;

    //* changed state
    bool m_changed;

    //* defaults clicked
    bool m_defaultsPressed = false;

    bool m_loading = false;
    bool m_loaded = false;
    bool m_processingDefaults = false;
};

}

#endif // WINDOWOUTLINEOPACITY_H
