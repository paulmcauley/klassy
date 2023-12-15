/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "addpreset.h"

namespace Breeze
{

AddPreset::AddPreset(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_AddPreset)
{
    m_ui->setupUi(this);
    connect(m_ui->importButton, &QAbstractButton::clicked, this, &AddPreset::importButtonClicked);
}

AddPreset::~AddPreset()
{
    delete m_ui;
}

void AddPreset::importButtonClicked()
{
    reject();
    Q_EMIT importClicked();
}
}
