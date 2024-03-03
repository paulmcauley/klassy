/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "breeze.h"
#include "breezesettings.h"
#include "ui_systemicongeneration.h"
#include <QDialog>

namespace Breeze
{

class SystemIconGeneration : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit SystemIconGeneration(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent = nullptr);
    ~SystemIconGeneration();

    void save(const bool reloadKwinConfig = true);
    void defaults();

public Q_SLOTS:
    void load();

private Q_SLOTS:
    void accept() override;
    void reject() override;
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
    bool isDefaults();
    void populateComboBoxesWithSystemIconThemes();
    Ui_SystemIconGeneration *m_ui;

    InternalSettingsPtr m_internalSettings;
    KSharedConfig::Ptr m_configuration;
    KSharedConfig::Ptr m_presetsConfiguration;
    QObject *m_parent;

    //* changed state
    bool m_changed;

    //* defaults clicked
    bool m_defaultsPressed = false;

    bool m_loading = false;
    bool m_loaded = false;
    bool m_processingDefaults = false;
};

}
