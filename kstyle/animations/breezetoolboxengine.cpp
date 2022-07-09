/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezetoolboxengine.h"

namespace Breeze
{

//____________________________________________________________
bool ToolBoxEngine::registerWidget(QWidget *widget)
{
    if (!widget)
        return false;
    if (!_data.contains(widget)) {
        _data.insert(widget, new WidgetStateData(this, widget, duration()), enabled());
    }

    // connect destruction signal
    connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(unregisterWidget(QObject *)), Qt::UniqueConnection);
    return true;
}

//____________________________________________________________
bool ToolBoxEngine::updateState(const QPaintDevice *object, bool value)
{
    PaintDeviceDataMap<WidgetStateData>::Value data(ToolBoxEngine::data(object));
    return (data && data.data()->updateState(value));
}

//____________________________________________________________
bool ToolBoxEngine::isAnimated(const QPaintDevice *object)
{
    PaintDeviceDataMap<WidgetStateData>::Value data(ToolBoxEngine::data(object));
    return (data && data.data()->animation() && data.data()->animation().data()->isRunning());
}
}
