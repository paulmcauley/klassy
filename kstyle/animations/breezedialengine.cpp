/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezedialengine.h"

#include <QEvent>

namespace Breeze
{

//____________________________________________________________
bool DialEngine::registerWidget(QWidget *widget, AnimationModes mode)
{
    // check widget
    if (!widget)
        return false;

    // only handle hover and focus
    if (mode & AnimationHover && !dataMap(AnimationHover).contains(widget)) {
        dataMap(AnimationHover).insert(widget, new DialData(this, widget, duration()), enabled());
    }
    if (mode & AnimationFocus && !dataMap(AnimationFocus).contains(widget)) {
        dataMap(AnimationFocus).insert(widget, new WidgetStateData(this, widget, duration()), enabled());
    }

    // connect destruction signal
    connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(unregisterWidget(QObject *)), Qt::UniqueConnection);

    return true;
}
}
