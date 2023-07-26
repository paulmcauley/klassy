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
bool WidgetStateEngine::registerWidget(QObject *target, AnimationModes modes)
{
    if (!target) {
        return false;
    }
    if (modes & AnimationHover && !_hoverData.contains(target)) {
        _hoverData.insert(target, new WidgetStateData(this, target, duration()), enabled());
    }
    if (modes & AnimationFocus && !_focusData.contains(target)) {
        _focusData.insert(target, new WidgetStateData(this, target, duration()), enabled());
    }
    if (modes & AnimationEnable && !_enableData.contains(target)) {
        _enableData.insert(target, new EnableData(this, target, duration()), enabled());
    }
    if (modes & AnimationPressed && !_pressedData.contains(target)) {
        _pressedData.insert(target, new WidgetStateData(this, target, duration()), enabled());
    }

    // connect destruction signal
    connect(target, &QObject::destroyed, this, &WidgetStateEngine::unregisterWidget, Qt::UniqueConnection);

    return true;
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
