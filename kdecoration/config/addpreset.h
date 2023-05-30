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

private:
    Ui_AddPreset *m_ui;
};

}

#endif // ADDPRESET_H
