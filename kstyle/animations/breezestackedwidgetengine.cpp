//////////////////////////////////////////////////////////////////////////////
// breezestackedwidgetengine.cpp
// stores event filters and maps widgets to animations
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezestackedwidgetengine.h"

namespace Breeze
{

//____________________________________________________________
bool StackedWidgetEngine::registerWidget(QStackedWidget *widget)
{
    if (!widget)
        return false;
    if (!_data.contains(widget)) {
        _data.insert(widget, new StackedWidgetData(this, widget, duration()), enabled());
    }

    // connect destruction signal
    disconnect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(unregisterWidget(QObject *)));
    connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(unregisterWidget(QObject *)));

    return true;
}
}
