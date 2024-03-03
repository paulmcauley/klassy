/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "breeze.h"
#include "breezesettings.h"
#include "ui_titlebaropacity.h"
#include <QDialog>

namespace Breeze
{

class TitleBarOpacity : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit TitleBarOpacity(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent = nullptr);
    ~TitleBarOpacity();

    void loadMain(const bool assignUiValuesOnly = false);
    void save(const bool reloadKwinConfig = true);
    void defaults();

    bool event(QEvent *ev) override;

public Q_SLOTS:
    void load()
    {
        loadMain();
    }

private Q_SLOTS:
    void accept() override;
    void reject() override;
    void updateChanged();
    void saveAndReloadKWinConfig()
    {
        save(true);
    }
    void setApplyButtonState(const bool on);
    void setEnabledTransparentTitlebarOptions();

Q_SIGNALS:
    void changed(bool);

private:
    void setChanged(bool value);
    bool isDefaults();

    // system colour scheme alpha settings
    void getTitlebarOpacityFromColorScheme();

    Ui_TitleBarOpacity *m_ui;

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

    bool m_translucentActiveSchemeColor = false;
    bool m_translucentInactiveSchemeColor = false;
    qreal m_activeSchemeColorAlpha = 1;
    qreal m_inactiveSchemeColorAlpha = 1;
};

}
