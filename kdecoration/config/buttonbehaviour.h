#ifndef BUTTONBEHAVIOUR_H
#define BUTTONBEHAVIOUR_H

/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "breeze.h"
#include "breezesettings.h"
#include "ui_buttonbehaviour.h"
#include <QCheckBox>
#include <QDialog>

namespace Breeze
{

class ButtonBehaviour : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit ButtonBehaviour(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent = nullptr);
    ~ButtonBehaviour();

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
    void saveAndReloadKWinConfig()
    {
        save(true);
    }
    void setApplyButtonState(const bool on);
    void updateLockIcons();
    void copyCheckedStatusFromNormalToCloseActive();
    void copyCheckedStatusFromCloseToNormalActive();
    void copyCheckedStatusFromNormalToCloseInactive();
    void copyCheckedStatusFromCloseToNormalInactive();
    void copyCheckedStatusFromActiveToInactive();
    void copyCheckedStatusFromInactiveToActive();

Q_SIGNALS:
    void saved();
    void changed(bool);

private:
    void setChanged(bool value);
    bool isDefaults();

    Ui_ButtonBehaviour *m_ui;

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

    enum class TableCheckBox {
        showIconOnPress,
        showIconOnHover,
        showIconNormally,
        varyColorIcon,
        showBackgroundOnPress,
        showBackgroundOnHover,
        showBackgroundNormally,
        varyColorBackground,
        showOutlineOnPress,
        showOutlineOnHover,
        showOutlineNormally,
        varyColorOutline,
        COUNT
    };

    enum class TableCloseCheckBox {
        showCloseIconOnPress,
        showCloseIconOnHover,
        showCloseIconNormally,
        varyColorCloseIcon,
        showCloseBackgroundOnPress,
        showCloseBackgroundOnHover,
        showCloseBackgroundNormally,
        varyColorCloseBackground,
        showCloseOutlineOnPress,
        showCloseOutlineOnHover,
        showCloseOutlineNormally,
        varyColorCloseOutline,
        COUNT
    };
    QMap<TableCheckBox, QCheckBox *> m_tableCheckBoxesActive;
    QMap<TableCloseCheckBox, QCheckBox *> m_tableCloseCheckBoxesActive;
    QMap<TableCheckBox, QCheckBox *> m_tableCheckBoxesInactive;
    QMap<TableCloseCheckBox, QCheckBox *> m_tableCloseCheckBoxesInactive;
};

}

#endif // BUTTONBEHAVIOUR_H
