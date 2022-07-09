/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezespinboxengine.h"

#include <QEvent>

namespace Breeze
{

//____________________________________________________________
bool SpinBoxEngine::registerWidget(QWidget *widget)
{
    if (!widget)
        return false;

    // create new data class
    if (!_data.contains(widget))
        _data.insert(widget, new SpinBoxData(this, widget, duration()), enabled());

    // connect destruction signal
    connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(unregisterWidget(QObject *)), Qt::UniqueConnection);
    return true;
}
}
