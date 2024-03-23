/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config-breeze.h"

#include "breezebusyindicatorengine.h"

#include "breezemetrics.h"

#if BREEZE_HAVE_QTQUICK
#include <QQuickItem>
#endif

#include <QVariant>
#include <QWidget>

namespace Breeze
{

//_______________________________________________
BusyIndicatorEngine::BusyIndicatorEngine(QObject *object)
    : BaseEngine(object)
{
}

//_______________________________________________
bool BusyIndicatorEngine::registerWidget(QObject *object)
{
    // check widget validity
    if (!object) {
        return false;
    }

    // create new data class
    if (!_data.contains(object)) {
        _data.insert(object, new BusyIndicatorData(this));

        // connect destruction signal
        connect(object, &QObject::destroyed, this, &BusyIndicatorEngine::unregisterWidget, Qt::UniqueConnection);

#if BREEZE_HAVE_QTQUICK
        if (QQuickItem *item = qobject_cast<QQuickItem *>(object)) {
            connect(item, &QQuickItem::visibleChanged, this, [this, item, object]() {
                if (!item->isVisible()) {
                    this->setAnimated(object, false);
                }
                // no need for the else {} branch, as its animation will be reset anyway
            });
        }
#endif
    }

    return true;
}

//____________________________________________________________
bool BusyIndicatorEngine::isAnimated(const QObject *object)
{
    DataMap<BusyIndicatorData>::Value data(BusyIndicatorEngine::data(object));
    return data && data.data()->isAnimated();
}

//____________________________________________________________
void BusyIndicatorEngine::setDuration(int value)
{
    if (duration() == value) {
        return;
    }
    BaseEngine::setDuration(value);

    // restart timer with specified time
    if (_animation) {
        _animation.data()->setDuration(value);
    }
}

//____________________________________________________________
void BusyIndicatorEngine::setAnimated(const QObject *object, bool value)
{
    DataMap<BusyIndicatorData>::Value data(BusyIndicatorEngine::data(object));
    if (data) {
        // update data
        data.data()->setAnimated(value);

        // start timer if needed
        if (value) {
            if (!_animation) {
                // create animation if not already there
                _animation = new Animation(duration(), this);

                // setup
                _animation.data()->setStartValue(0);
                _animation.data()->setEndValue(2 * Metrics::ProgressBar_BusyIndicatorSize);
                _animation.data()->setTargetObject(this);
                _animation.data()->setPropertyName("value");
                _animation.data()->setLoopCount(-1);
                _animation.data()->setDuration(duration());
            }

            // start if  not already running
            if (!_animation.data()->isRunning()) {
                _animation.data()->start();
            }
        }
    }
}

//____________________________________________________________
DataMap<BusyIndicatorData>::Value BusyIndicatorEngine::data(const QObject *object)
{
    return _data.find(object).data();
}

//_______________________________________________
void BusyIndicatorEngine::setValue(int value)
{
    // update
    _value = value;

    bool animated(false);

    // loop over objects in map
    for (DataMap<BusyIndicatorData>::iterator iter = _data.begin(); iter != _data.end(); ++iter) {
        if (iter.value().data()->isAnimated()) {
            // update animation flag
            animated = true;

            const void *key = iter.key();
            QObject *obj = const_cast<QObject *>(static_cast<const QObject *>(key));
#if BREEZE_HAVE_QTQUICK
            if (QQuickItem *item = qobject_cast<QQuickItem *>(obj)) {
                item->polish();
            } else
#endif
                if (QWidget *widget = qobject_cast<QWidget *>(obj)) {
                widget->update();
            }
        }
    }

    if (_animation && !animated) {
        _animation.data()->stop();
        _animation.data()->deleteLater();
        _animation.clear();
    }
}

//__________________________________________________________
bool BusyIndicatorEngine::unregisterWidget(QObject *object)
{
    const bool removed(_data.unregisterWidget(object));
    if (_animation && _data.isEmpty()) {
        _animation.data()->stop();
        _animation.data()->deleteLater();
        _animation.clear();
    }

    return removed;
}

}
