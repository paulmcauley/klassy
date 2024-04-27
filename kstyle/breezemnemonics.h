/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QApplication>
#include <QEvent>
#include <QObject>

#include "breezestyleconfigdata.h"

namespace Breeze
{
//* keyboard accelerators
class Mnemonics : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit Mnemonics(QObject *parent = nullptr);

    //* set mode
    void setMode(int);

    //* event filter
    bool eventFilter(QObject *, QEvent *) override;

    //* true if mnemonics are enabled
    bool enabled() const
    {
        return _enabled;
    }

    //* alignment flag
    int textFlags() const
    {
        return _enabled ? Qt::TextShowMnemonic : Qt::TextHideMnemonic;
    }

protected:
    //* set enable state
    void setEnabled(bool);

private:
    //* enable state
    bool _enabled = true;
};
}
