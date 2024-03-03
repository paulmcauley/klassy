/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "breeze.h"
#include "breezesettings.h"
#include "ui_titlebarspacing.h"
#include <QDialog>

namespace Breeze
{

class TitleBarSpacing : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit TitleBarSpacing(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent = nullptr);
    ~TitleBarSpacing();

    void loadMain(const QString loadPreset = QString());
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

    void titleBarTopMarginChanged();
    void titleBarBottomMarginChanged();
    void titleBarLeftMarginChanged();
    void titleBarRightMarginChanged();

    void updateLockIcons();

Q_SIGNALS:
    void changed(bool);

private:
    void setChanged(bool value);
    bool isDefaults();

    Ui_TitleBarSpacing *m_ui;

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
