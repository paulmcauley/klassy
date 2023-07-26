/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezescrollbarengine.h"

#include <QEvent>

namespace Breeze
{

//____________________________________________________________
bool ScrollBarEngine::registerWidget(QObject *target, AnimationModes modes)
{
    // check widget
    if (!target) {
        return false;
    }

    // only handle hover and focus
    if (modes & AnimationHover && !dataMap(AnimationHover).contains(target)) {
        dataMap(AnimationHover).insert(target, new ScrollBarData(this, target, duration()), enabled());
    }
    if (modes & AnimationFocus && !dataMap(AnimationFocus).contains(target)) {
        dataMap(AnimationFocus).insert(target, new WidgetStateData(this, target, duration()), enabled());
    }

    // connect destruction signal
    connect(target, &QObject::destroyed, this, &ScrollBarEngine::unregisterWidget, Qt::UniqueConnection);

    return true;
}

//____________________________________________________________
bool ScrollBarEngine::isAnimated(const QObject *object, AnimationMode mode, QStyle::SubControl control)
{
    if (mode == AnimationHover) {
        if (DataMap<WidgetStateData>::Value data = this->data(object, AnimationHover)) {
            const ScrollBarData *scrollBarData(static_cast<const ScrollBarData *>(data.data()));
            const Animation::Pointer &animation = scrollBarData->animation(control);
            return animation.data()->isRunning();

        } else {
            return false;
        }

    } else if (control == QStyle::SC_ScrollBarSlider) {
        return WidgetStateEngine::isAnimated(object, mode);

    } else {
        return false;
    }
}

//____________________________________________________________
AnimationMode ScrollBarEngine::animationMode(const QObject *object, QStyle::SubControl control)
{
    // enable state
    if (isAnimated(object, AnimationHover, control)) {
        return AnimationHover;
    } else if (isAnimated(object, AnimationFocus, control)) {
        return AnimationFocus;
    } else if (isAnimated(object, AnimationPressed, control)) {
        return AnimationPressed;
    } else {
        return AnimationNone;
    }
}

//____________________________________________________________
qreal ScrollBarEngine::opacity(const QObject *object, QStyle::SubControl control)
{
    if (isAnimated(object, AnimationHover, control)) {
        return static_cast<const ScrollBarData *>(data(object, AnimationHover).data())->opacity(control);
    } else if (control == QStyle::SC_ScrollBarSlider) {
        return WidgetStateEngine::buttonOpacity(object);
    }
    return AnimationData::OpacityInvalid;
}
}
