/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezedialdata.h"

#include <QDial>
#include <QHoverEvent>

namespace Breeze
{

//______________________________________________
DialData::DialData(QObject *parent, QWidget *target, int duration)
    : WidgetStateData(parent, target, duration)
    , _position(-1, -1)
{
    target->installEventFilter(this);
}

//______________________________________________
bool DialData::eventFilter(QObject *object, QEvent *event)
{
    if (object != target().data()) {
        return WidgetStateData::eventFilter(object, event);
    }

    // check event type
    switch (event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove:
        hoverMoveEvent(object, event);
        break;

    case QEvent::HoverLeave:
        hoverLeaveEvent(object, event);
        break;

    default:
        break;
    }

    return WidgetStateData::eventFilter(object, event);
}

//______________________________________________
void DialData::hoverMoveEvent(QObject *object, QEvent *event)
{
    // try cast object to dial
    QDial *scrollBar(qobject_cast<QDial *>(object));
    if (!scrollBar || scrollBar->isSliderDown())
        return;

    // cast event
    QHoverEvent *hoverEvent = static_cast<QHoverEvent *>(event);

    // store position
    _position = hoverEvent->pos();

    // trigger animation if position match handle rect
    updateState(_handleRect.contains(_position));
}

//______________________________________________
void DialData::hoverLeaveEvent(QObject *, QEvent *)
{
    // reset hover state
    updateState(false);

    // reset mouse position
    _position = QPoint(-1, -1);
}
}
