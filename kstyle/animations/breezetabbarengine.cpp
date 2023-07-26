/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezetabbarengine.h"

#include <QEvent>

namespace Breeze
{

//____________________________________________________________
bool TabBarEngine::registerWidget(QObject *target)
{
    if (!target) {
        return false;
    }

    // create new data class
    if (!_hoverData.contains(target)) {
        _hoverData.insert(target, new TabBarData(this, target, duration()), enabled());
    }
    if (!_focusData.contains(target)) {
        _focusData.insert(target, new TabBarData(this, target, duration()), enabled());
    }

    // connect destruction signal
    connect(target, &QObject::destroyed, this, &TabBarEngine::unregisterWidget, Qt::UniqueConnection);
    return true;
}

//____________________________________________________________
bool TabBarEngine::updateState(const QObject *object, const QPoint &position, AnimationMode mode, bool value)
{
    DataMap<TabBarData>::Value data(TabBarEngine::data(object, mode));
    return (data && data.data()->updateState(position, value));
}

//____________________________________________________________
bool TabBarEngine::isAnimated(const QObject *object, const QPoint &position, AnimationMode mode)
{
    DataMap<TabBarData>::Value data(TabBarEngine::data(object, mode));
    return (data && data.data()->animation(position) && data.data()->animation(position).data()->isRunning());
}

//____________________________________________________________
DataMap<TabBarData>::Value TabBarEngine::data(const QObject *object, AnimationMode mode)
{
    switch (mode) {
    case AnimationHover:
        return _hoverData.find(object).data();
    case AnimationFocus:
        return _focusData.find(object).data();
    default:
        return DataMap<TabBarData>::Value();
    }
}
}
