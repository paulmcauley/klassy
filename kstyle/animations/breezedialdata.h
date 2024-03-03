/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "breezewidgetstatedata.h"

namespace Breeze
{

//* dial data
class DialData : public WidgetStateData
{
    Q_OBJECT

public:
    //* constructor
    DialData(QObject *parent, QObject *target, int duration);

    //* event filter
    bool eventFilter(QObject *, QEvent *) override;

    //* subcontrol rect
    void setHandleRect(const QRect &rect)
    {
        _handleRect = rect;
    }

    //* mouse position
    QPoint position() const
    {
        return _position;
    }

private:
    //* hoverMoveEvent
    void hoverMoveEvent(QObject *, QEvent *);

    //* hoverMoveEvent
    void hoverLeaveEvent(QObject *, QEvent *);

    //* rect
    QRect _handleRect;

    //* mouse position
    QPoint _position;
};
}
