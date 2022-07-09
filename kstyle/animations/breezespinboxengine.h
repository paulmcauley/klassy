/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezespinboxengine_h
#define breezespinboxengine_h

#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezespinboxdata.h"

namespace Breeze
{

//* handle spinbox arrows hover effect
class SpinBoxEngine : public BaseEngine
{
    Q_OBJECT

public:
    //* constructor
    explicit SpinBoxEngine(QObject *parent)
        : BaseEngine(parent)
    {
    }

    //* register widget
    bool registerWidget(QWidget *);

    //* state
    bool updateState(const QObject *object, QStyle::SubControl subControl, bool value)
    {
        if (DataMap<SpinBoxData>::Value data = _data.find(object)) {
            return data.data()->updateState(subControl, value);
        } else
            return false;
    }

    //* true if widget is animated
    bool isAnimated(const QObject *object, QStyle::SubControl subControl)
    {
        if (DataMap<SpinBoxData>::Value data = _data.find(object)) {
            return data.data()->isAnimated(subControl);
        } else
            return false;
    }

    //* animation opacity
    qreal opacity(const QObject *object, QStyle::SubControl subControl)
    {
        if (DataMap<SpinBoxData>::Value data = _data.find(object)) {
            return data.data()->opacity(subControl);
        } else
            return AnimationData::OpacityInvalid;
    }

    //* enability
    void setEnabled(bool value) override
    {
        BaseEngine::setEnabled(value);
        _data.setEnabled(value);
    }

    //* duration
    void setDuration(int value) override
    {
        BaseEngine::setDuration(value);
        _data.setDuration(value);
    }

public Q_SLOTS:

    //* remove widget from map
    bool unregisterWidget(QObject *object) override
    {
        return _data.unregisterWidget(object);
    }

private:
    //* data map
    DataMap<SpinBoxData> _data;
};
}

#endif
