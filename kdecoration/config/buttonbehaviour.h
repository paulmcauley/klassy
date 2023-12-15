#ifndef BUTTONBEHAVIOUR_H
#define BUTTONBEHAVIOUR_H

/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "breeze.h"
#include "breezesettings.h"
#include "ui_buttonbehaviour.h"
#include <QDialog>

namespace Breeze
{

class ButtonBehaviour : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit ButtonBehaviour(KSharedConfig::Ptr config, QWidget *parent = nullptr);
    ~ButtonBehaviour();

    void loadMain(const QString loadPreset = QString());
    void save(const bool reloadKwinConfig = true);
    void defaults();

public Q_SLOTS:
    void load()
    {
        loadMain();
    }

private Q_SLOTS:
    void accept() override;
    void updateChanged();
    void saveAndReloadKWinConfig()
    {
        save(true);
    }
    void setApplyButtonState(const bool on);

Q_SIGNALS:
    void changed(bool);

private:
    void setChanged(bool value);

    Ui_ButtonBehaviour *m_ui;

    InternalSettingsPtr m_internalSettings;
    KSharedConfig::Ptr m_configuration;

    //* changed state
    bool m_changed;

    //* defaults clicked
    bool m_defaultsPressed = false;

    bool m_loading = false;
    bool m_loaded = false;
    bool m_processingDefaults = false;
};

}

#endif // BUTTONBEHAVIOUR_H
