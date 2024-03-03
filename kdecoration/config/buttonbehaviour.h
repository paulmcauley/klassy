/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "breeze.h"
#include "breezesettings.h"
#include "ui_buttonbehaviour.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>

namespace Breeze
{

class ButtonBehaviour : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit ButtonBehaviour(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent = nullptr);
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
    void copyComboStatusFromNormalToCloseActive();
    void copyComboStatusFromNormalToCloseInactive();
    void copyComboStatusFromCloseToNormalActive();
    void copyComboStatusFromCloseToNormalInactive();
    void copyComboStatusFromActiveToInactive();
    void copyComboStatusFromInactiveToActive();

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
        showBackgroundOnPress,
        showBackgroundOnHover,
        showBackgroundNormally,
        showOutlineOnPress,
        showOutlineOnHover,
        showOutlineNormally,
        COUNT
    };

    enum class TableCloseCheckBox {
        showCloseIconOnPress,
        showCloseIconOnHover,
        showCloseIconNormally,
        showCloseBackgroundOnPress,
        showCloseBackgroundOnHover,
        showCloseBackgroundNormally,
        showCloseOutlineOnPress,
        showCloseOutlineOnHover,
        showCloseOutlineNormally,
        COUNT
    };

    enum class TableVaryColorComboBox { varyColorIcon, varyColorBackground, varyColorOutline, COUNT };

    enum class TableCloseVaryColorComboBox { varyColorCloseIcon, varyColorCloseBackground, varyColorCloseOutline, COUNT };

    QMap<TableCheckBox, QCheckBox *> m_tableCheckBoxesActive;
    QMap<TableCloseCheckBox, QCheckBox *> m_tableCloseCheckBoxesActive;
    QMap<TableCheckBox, QCheckBox *> m_tableCheckBoxesInactive;
    QMap<TableCloseCheckBox, QCheckBox *> m_tableCloseCheckBoxesInactive;
    QMap<TableVaryColorComboBox, QComboBox *> m_tableComboBoxesActive;
    QMap<TableCloseVaryColorComboBox, QComboBox *> m_tableCloseComboBoxesActive;
    QMap<TableVaryColorComboBox, QComboBox *> m_tableComboBoxesInactive;
    QMap<TableCloseVaryColorComboBox, QComboBox *> m_tableCloseComboBoxesInactive;
};

}
