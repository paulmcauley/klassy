/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "breezewidgetstatedata.h"

namespace Breeze
{

//* Enable data
class EnableData : public WidgetStateData
{
    Q_OBJECT

public:
    //* constructor
    EnableData(QObject *parent, QObject *target, int duration, bool state = true)
        : WidgetStateData(parent, target, duration, state)
    {
        target->installEventFilter(this);
    }

    //* event filter
    bool eventFilter(QObject *, QEvent *) override;
};
}
