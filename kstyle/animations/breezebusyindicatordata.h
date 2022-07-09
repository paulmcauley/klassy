/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezebusyindicatordata_h
#define breezebusyindicatordata_h

#include <QObject>

namespace Breeze
{

class BusyIndicatorData : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit BusyIndicatorData(QObject *parent)
        : QObject(parent)
        , _animated(false)
    {
    }

    //* destructor
    virtual ~BusyIndicatorData()
    {
    }

    //*@name accessors
    //@{

    //* animated
    bool isAnimated() const
    {
        return _animated;
    }

    //@}

    //*@name modifiers
    //@{

    //* enabled
    void setEnabled(bool)
    {
    }

    //* enabled
    void setDuration(int)
    {
    }

    //* animated
    void setAnimated(bool value)
    {
        _animated = value;
    }

    //@}

private:
    //* animated
    bool _animated;
};
}

#endif
