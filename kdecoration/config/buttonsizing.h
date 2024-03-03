/*
 * SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "breeze.h"
#include "breezesettings.h"
#include "ui_buttonsizing.h"
#include <QDialog>

namespace Breeze
{

class ButtonSizing : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit ButtonSizing(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent = nullptr);
    ~ButtonSizing();

    void loadMain(const bool assignUiValuesOnly = false);
    void save(const bool reloadKwinConfig = true);
    void defaults();

public Q_SLOTS:
    void load()
    {
        loadMain();
    }

private Q_SLOTS:
    void accept() override;
    void reject() override;
    void updateChanged();
    void fullHeightButtonWidthMarginLeftChanged();
    void fullHeightButtonWidthMarginRightChanged();
    void buttonSpacingLeftChanged();
    void buttonSpacingRightChanged();
    void fullHeightButtonSpacingLeftChanged();
    void fullHeightButtonSpacingRightChanged();
    void saveAndReloadKWinConfig()
    {
        save(true);
    }
    void setApplyButtonState(const bool on);
    void updateLockIcons();
    void setButtonCustomCornerRadiusVisible();

Q_SIGNALS:
    void changed(bool);

private:
    void setChanged(bool value);
    bool isDefaults();
    void setVisibleUiElements();

    Ui_ButtonSizing m_ui;

    InternalSettingsPtr m_internalSettings;
    KSharedConfig::Ptr m_configuration;
    KSharedConfig::Ptr m_presetsConfiguration;
    QObject *m_parent;

    //* changed state
    bool m_changed;

    //* defaults clicked
    bool m_defaultsPressed = false;

    bool m_loading = false;
    bool m_loaded;
    bool m_processingDefaults = false;

    int m_buttonShape = 0;
};

}
