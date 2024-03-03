/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "addpreset.h"
#include "breeze.h"
#include "ui_loadpreset.h"
#include <QDialog>

namespace Breeze
{

class LoadPreset : public QDialog
{
    Q_OBJECT

public:
    explicit LoadPreset(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent = nullptr);
    ~LoadPreset();
    void initPresetsList();

protected Q_SLOTS:
    void addButtonClicked();
    void presetsListActivated();
    void loadButtonClicked();
    void removeButtonClicked();
    void importButtonClicked();
    void exportButtonClicked();

private:
    Ui_LoadPreset *m_ui;
    AddPreset *m_addDialog;

    //* kconfiguration object
    KSharedConfig::Ptr m_configuration;
    //* presets kconfiguration object
    KSharedConfig::Ptr m_presetsConfiguration;
    QObject *m_parent;
};

}
