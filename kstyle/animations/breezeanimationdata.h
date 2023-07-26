/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef breeze_animationdata_h
#define breeze_animationdata_h

#include "config-breeze.h"

#include "breezeanimation.h"

#if BREEZE_HAVE_QTQUICK
#include <QQuickItem>
#endif

#include <QEvent>
#include <QObject>
#include <QWidget>

#include <cmath>

namespace Breeze
{

//* base class
class AnimationData : public QObject
{
    Q_OBJECT

public:
    //* constructor
    AnimationData(QObject *parent, QObject *target)
        : QObject(parent)
        , _target(target)
    {
    }

    //* duration
    virtual void setDuration(int) = 0;

    //* steps
    static void setSteps(int value)
    {
        _steps = value;
    }

    //* enability
    virtual bool enabled() const
    {
        return _enabled;
    }

    //* enability
    virtual void setEnabled(bool value)
    {
        _enabled = value;
    }

    //* target
    const WeakPointer<QObject> &target() const
    {
        return _target;
    }

    //* invalid opacity
    static const qreal OpacityInvalid;

protected:
    //* setup animation
    virtual void setupAnimation(const Animation::Pointer &animation, const QByteArray &property);

    //* apply step
    virtual qreal digitize(const qreal &value) const
    {
        if (_steps > 0) {
            return std::floor(value * _steps) / _steps;
        } else {
            return value;
        }
    }

    //* trigger target update
    virtual void setDirty() const
    {
        if (auto widget = qobject_cast<QWidget *>(_target)) {
            widget->update();
        }
#if BREEZE_HAVE_QTQUICK
        else if (auto item = qobject_cast<QQuickItem *>(_target)) {
            // Note: Calling polish() instead of update() because that's where
            // Breeze would repaint its image for texture.
            item->polish();
        }
#endif
    }

private:
    //* guarded target
    WeakPointer<QObject> _target;

    //* enability
    bool _enabled = true;

    //* steps
    static int _steps;
};
}

#endif
