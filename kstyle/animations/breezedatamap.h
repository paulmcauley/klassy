/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "breeze.h"

#include <QMap>
#include <QObject>
#include <QPaintDevice>

namespace Breeze
{

//* data map
/** it maps an opaque pointer an associated QPointer<object> */
template<typename T>
class DataMap : public QMap<const void *, WeakPointer<T>>
{
public:
    using Key = const void *;
    using Value = WeakPointer<T>;

    //* insertion
    typename QMap<Key, Value>::iterator insert(const Key &key, const Value &value, bool enabled = true)
    {
        if (value) {
            value.data()->setEnabled(enabled);
        }
        return QMap<Key, Value>::insert(key, value);
    }

    //* find value
    Value find(Key key)
    {
        if (!(enabled() && key)) {
            return Value();
        }
        if (key == _lastKey) {
            return _lastValue;
        } else {
            Value out;
            typename QMap<Key, Value>::iterator iter(QMap<Key, Value>::find(key));
            if (iter != QMap<Key, Value>::end()) {
                out = iter.value();
            }
            _lastKey = key;
            _lastValue = out;
            return out;
        }
    }

    //* unregister widget
    bool unregisterWidget(Key key)
    {
        // check key
        if (!key) {
            return false;
        }

        // clear last value if needed
        if (key == _lastKey) {
            if (_lastValue) {
                _lastValue.clear();
            }
            _lastKey = nullptr;
        }

        // find key in map
        typename QMap<Key, Value>::iterator iter(QMap<Key, Value>::find(key));
        if (iter == QMap<Key, Value>::end()) {
            return false;
        }

        // delete value from map if found
        if (iter.value()) {
            iter.value().data()->deleteLater();
        }
        QMap<Key, Value>::erase(iter);

        return true;
    }

    //* maxFrame
    void setEnabled(bool enabled)
    {
        _enabled = enabled;
        for (const Value &value : std::as_const(*this)) {
            if (value) {
                value.data()->setEnabled(enabled);
            }
        }
    }

    //* enability
    bool enabled() const
    {
        return _enabled;
    }

    //* duration
    void setDuration(int duration) const
    {
        for (const Value &value : std::as_const(*this)) {
            if (value) {
                value.data()->setDuration(duration);
            }
        }
    }

private:
    //* enability
    bool _enabled = false;

    //* last key
    Key _lastKey = nullptr;

    //* last value
    Value _lastValue;
};
}
