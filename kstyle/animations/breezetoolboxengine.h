/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezetoolboxengine_h
#define breezetoolboxengine_h

#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezewidgetstatedata.h"

namespace Breeze
{

//* QToolBox animation engine
class ToolBoxEngine : public BaseEngine
{
    Q_OBJECT

public:
    //* constructor
    explicit ToolBoxEngine(QObject *parent)
        : BaseEngine(parent)
    {
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

    //* register widget
    bool registerWidget(QWidget *);

    //* true if widget hover state is changed
    bool updateState(const QPaintDevice *, bool);

    //* true if widget is animated
    bool isAnimated(const QPaintDevice *);

    //* animation opacity
    qreal opacity(const QPaintDevice *object)
    {
        return isAnimated(object) ? data(object).data()->opacity() : AnimationData::OpacityInvalid;
    }

public Q_SLOTS:

    //* remove widget from map
    bool unregisterWidget(QObject *data) override
    {
        if (!data)
            return false;

        // reinterpret_cast is safe here since only the address is used to find
        // data in the map
        return _data.unregisterWidget(reinterpret_cast<QPaintDevice *>(data));
    }

protected:
    //* returns data associated to widget
    PaintDeviceDataMap<WidgetStateData>::Value data(const QPaintDevice *object)
    {
        return _data.find(object).data();
    }

private:
    //* map
    PaintDeviceDataMap<WidgetStateData> _data;
};
}

#endif
