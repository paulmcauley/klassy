/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezeanimation_h
#define breezeanimation_h

#include "breeze.h"

#include <QPropertyAnimation>
#include <QVariant>

namespace Breeze
{

class Animation : public QPropertyAnimation
{
    Q_OBJECT

public:
    //* convenience
    using Pointer = WeakPointer<Animation>;

    //* constructor
    Animation(int duration, QObject *parent)
        : QPropertyAnimation(parent)
    {
        setDuration(duration);
    }

    //* true if running
    bool isRunning() const
    {
        return state() == Animation::Running;
    }

    //* restart
    void restart()
    {
        if (isRunning())
            stop();
        start();
    }
};
}

#endif
