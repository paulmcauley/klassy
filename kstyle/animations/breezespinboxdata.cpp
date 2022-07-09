/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezespinboxdata.h"

namespace Breeze
{

//________________________________________________
SpinBoxData::SpinBoxData(QObject *parent, QWidget *target, int duration)
    : AnimationData(parent, target)
{
    _upArrowData._animation = new Animation(duration, this);
    _downArrowData._animation = new Animation(duration, this);
    setupAnimation(upArrowAnimation(), "upArrowOpacity");
    setupAnimation(downArrowAnimation(), "downArrowOpacity");
}

//______________________________________________
bool SpinBoxData::Data::updateState(bool value)
{
    if (_state == value)
        return false;
    else {
        _state = value;
        _animation.data()->setDirection(_state ? Animation::Forward : Animation::Backward);
        if (!_animation.data()->isRunning())
            _animation.data()->start();
        return true;
    }
}
}
