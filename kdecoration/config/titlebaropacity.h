#ifndef TITLEBAROPACITY_H
#define TITLEBAROPACITY_H

/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

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
    explicit TitleBarOpacity(KSharedConfig::Ptr config, QWidget *parent = nullptr);
    ~TitleBarOpacity();

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

#endif // TITLEBAROPACITY_H
