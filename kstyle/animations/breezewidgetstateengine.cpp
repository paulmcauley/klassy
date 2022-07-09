/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezewidgetstateengine.h"

#include "breezeenabledata.h"

namespace Breeze
{

//____________________________________________________________
bool WidgetStateEngine::registerWidget(QWidget *widget, AnimationModes mode)
{
    if (!widget)
        return false;
    if (mode & AnimationHover && !_hoverData.contains(widget)) {
        _hoverData.insert(widget, new WidgetStateData(this, widget, duration()), enabled());
    }
    if (mode & AnimationFocus && !_focusData.contains(widget)) {
        _focusData.insert(widget, new WidgetStateData(this, widget, duration()), enabled());
    }
    if (mode & AnimationEnable && !_enableData.contains(widget)) {
        _enableData.insert(widget, new EnableData(this, widget, duration()), enabled());
    }
    if (mode & AnimationPressed && !_pressedData.contains(widget)) {
        _pressedData.insert(widget, new WidgetStateData(this, widget, duration()), enabled());
    }

    // connect destruction signal
    connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(unregisterWidget(QObject *)), Qt::UniqueConnection);

    return true;
}

//____________________________________________________________
BaseEngine::WidgetList WidgetStateEngine::registeredWidgets(AnimationModes mode) const
{
    WidgetList out;

    using Value = DataMap<WidgetStateData>::Value;

    if (mode & AnimationHover) {
        foreach (const Value &value, _hoverData) {
            if (value)
                out.insert(value.data()->target().data());
        }
    }

    if (mode & AnimationFocus) {
        foreach (const Value &value, _focusData) {
            if (value)
                out.insert(value.data()->target().data());
        }
    }

    if (mode & AnimationEnable) {
        foreach (const Value &value, _enableData) {
            if (value)
                out.insert(value.data()->target().data());
        }
    }

    if (mode & AnimationPressed) {
        foreach (const Value &value, _pressedData) {
            if (value)
                out.insert(value.data()->target().data());
        }
    }

    return out;
}

//____________________________________________________________
bool WidgetStateEngine::updateState(const QObject *object, AnimationMode mode, bool value)
{
    DataMap<WidgetStateData>::Value data(WidgetStateEngine::data(object, mode));
    return (data && data.data()->updateState(value));
}

//____________________________________________________________
bool WidgetStateEngine::isAnimated(const QObject *object, AnimationMode mode)
{
    DataMap<WidgetStateData>::Value data(WidgetStateEngine::data(object, mode));
    return (data && data.data()->animation() && data.data()->animation().data()->isRunning());
}

//____________________________________________________________
DataMap<WidgetStateData>::Value WidgetStateEngine::data(const QObject *object, AnimationMode mode)
{
    switch (mode) {
    case AnimationHover:
        return _hoverData.find(object).data();
    case AnimationFocus:
        return _focusData.find(object).data();
    case AnimationEnable:
        return _enableData.find(object).data();
    case AnimationPressed:
        return _pressedData.find(object).data();
    default:
        return DataMap<WidgetStateData>::Value();
    }
}

//____________________________________________________________
DataMap<WidgetStateData> &WidgetStateEngine::dataMap(AnimationMode mode)
{
    switch (mode) {
    default:
    case AnimationHover:
        return _hoverData;
    case AnimationFocus:
        return _focusData;
    case AnimationEnable:
        return _enableData;
    case AnimationPressed:
        return _pressedData;
    }
}
}
