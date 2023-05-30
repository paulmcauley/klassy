#ifndef LOADPRESET_H
#define LOADPRESET_H

/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "breeze.h"
#include "ui_loadpreset.h"
#include <QDialog>

namespace Breeze
{

class LoadPreset : public QDialog
{
    Q_OBJECT

public:
    explicit LoadPreset(KSharedConfig::Ptr config, QWidget *parent = nullptr);
    ~LoadPreset();
    void initPresetsList();

protected Q_SLOTS:
    void addButtonClicked();
    void presetsListActivated();
    void loadButtonClicked();
    void removeButtonClicked();

private:
    Ui_LoadPreset *m_ui;

    //* kconfiguration object
    KSharedConfig::Ptr m_configuration;
    QWidget *m_parent;
};

}
#endif // LOADPRESET_H
