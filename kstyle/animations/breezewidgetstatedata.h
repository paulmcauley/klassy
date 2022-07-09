/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezewidgetstatedata_h
#define breezewidgetstatedata_h

#include "breezegenericdata.h"

namespace Breeze
{

//* handle widget state (hover/focus/enable) changes
class WidgetStateData : public GenericData
{
    Q_OBJECT

public:
    //* constructor
    WidgetStateData(QObject *parent, QWidget *target, int duration, bool state = false)
        : GenericData(parent, target, duration)
        , _initialized(false)
        , _state(state)
    {
    }

    //* destructor
    virtual ~WidgetStateData()
    {
    }

    /**
    returns true if hover has Changed
    and starts timer accordingly
    */
    virtual bool updateState(bool value);

private:
    bool _initialized;
    bool _state;
};
}

#endif
