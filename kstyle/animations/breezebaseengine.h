/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezebaseengine_h
#define breezebaseengine_h

#include "breeze.h"

#include <QObject>
#include <QSet>

namespace Breeze
{

//* base class for all animation engines
/** it is used to store configuration values used by all animations stored in the engine */
class BaseEngine : public QObject
{
    Q_OBJECT

public:
    using Pointer = WeakPointer<BaseEngine>;

    //* constructor
    explicit BaseEngine(QObject *parent)
        : QObject(parent)
    {
    }

    //* enability
    virtual void setEnabled(bool value)
    {
        _enabled = value;
    }

    //* enability
    virtual bool enabled() const
    {
        return _enabled;
    }

    //* duration
    virtual void setDuration(int value)
    {
        _duration = value;
    }

    //* duration
    virtual int duration() const
    {
        return _duration;
    }

    //* unregister widget
    virtual bool unregisterWidget(QObject *object) = 0;

    //* list of widgets
    using WidgetList = QSet<QWidget *>;

    //* returns registered widgets
    virtual WidgetList registeredWidgets() const
    {
        return WidgetList();
    }

private:
    //* engine enability
    bool _enabled = true;

    //* animation duration
    int _duration = 200;
};
}

#endif
