/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezewidgetstatedata.h"

namespace Breeze
{

//______________________________________________
bool WidgetStateData::updateState(bool value)
{
    if (!_initialized) {
        _state = value;
        _initialized = true;
        return false;

    } else if (_state == value) {
        return false;

    } else {
        _state = value;
        animation().data()->setDirection(_state ? Animation::Forward : Animation::Backward);
        if (!animation().data()->isRunning())
            animation().data()->start();
        return true;
    }
}
}
