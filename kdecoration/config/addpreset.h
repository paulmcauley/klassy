#ifndef ADDPRESET_H
#define ADDPRESET_H

/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ui_addpreset.h"
#include <QDialog>

namespace Breeze
{

class AddPreset : public QDialog
{
    Q_OBJECT

    friend class LoadPreset;

public:
    explicit AddPreset(QWidget *parent = nullptr);
    ~AddPreset();

signals:
    void importClicked();

private:
    Ui_AddPreset *m_ui;
    void importButtonClicked();
};

}

#endif // ADDPRESET_H
