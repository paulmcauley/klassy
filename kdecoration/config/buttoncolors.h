#ifndef BUTTONCOLORS_H
#define BUTTONCOLORS_H

/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "breeze.h"
#include "breezesettings.h"
#include "ui_buttoncolors.h"
#include <KColorButton>
#include <QDialog>

namespace Breeze
{

class ButtonColors : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit ButtonColors(KSharedConfig::Ptr config, QWidget *parent = nullptr);
    ~ButtonColors();

    void loadMain(const QString loadPreset = QString(), const bool assignUiValuesOnly = false);
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
    void updateChanged();
    void saveAndReloadKWinConfig()
    {
        save(true);
    }
    void setApplyButtonState(const bool on);
    void setNegativeCloseBackgroundHoverPressState();
    void setCloseIconNegativeBackgroundState();
    void resizeOverrideColorTable();
    void showActiveOverrideGroupBox(const bool on);
    void copyCellDataToInactiveTable(const bool on);
    void setButtonBackgroundColorsIcons();

Q_SIGNALS:
    void changed(bool);

private:
    //* outputs pointers to the CheckBox and ColorButton at a given table cell. Returns true if they are valid
    bool checkBoxAndColorButtonAtTableCell(QTableWidget *table, const int row, const int column, QCheckBox *&outputCheckBox, KColorButton *&outputColorButton);

    //* encodes the custom override colour settings for a row (button type) for storage in the config file
    // The output colorsList only stores set colours, and the output colourFlags has a complete index for all colours in the reloadKwinConfig
    // colorsFlgs storage bits: Active Window colours: bits 0-15, Inactive Window colours bits 16-31
    // active normal bits 0-3, active hover bits 4-7, active outline bits 8-11
    // inactive normal bits 16-19, inactive hover bits 20-23, inactive outline bits 24-27
    void encodeColorOverridableButtonTypeRow(QTableWidget *table, int row, uint32_t &colorsFlags, QList<int> &colorsList);

    //*returns true if the row was loaded with a value
    bool decodeColorsFlagsAndLoadRow(QTableWidget *table, int row, uint32_t colorsFlags, const QList<int> &colorsList);

    void setChanged(bool value);

    Ui_ButtonColors *m_ui;

    InternalSettingsPtr m_internalSettings;
    KSharedConfig::Ptr m_configuration;

    //* changed state
    bool m_changed;

    //* defaults clicked
    bool m_defaultsPressed = false;

    bool m_loading = false;
    bool m_loaded = false;
    bool m_processingDefaults = false;

    bool m_overrideColorsLoaded = false;

    // strings for UI corresponding to enum ColorOverridableButtonTypes in breeze.h
    QStringList m_colorOverridableButtonTypesStrings{
        i18n("Close"),
        i18n("Maximize"),
        i18n("Restore"),
        i18n("Minimize"),
        i18n("Context Help"),
        i18n("Shade"),
        i18n("Unshade"),
        i18n("Pin on all desktops"),
        i18n("Unpin"),
        i18n("Keep Below"),
        i18n("Keep Below (checked)"),
        i18n("Keep Above"),
        i18n("Keep Above (checked)"),
        i18n("Application Menu"),
        i18n("Menu (app icon)"),
    };
};

}

#endif // BUTTONCOLORS_H
