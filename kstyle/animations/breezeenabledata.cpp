/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezeenabledata.h"

namespace Breeze
{

//______________________________________________
bool EnableData::eventFilter(QObject *object, QEvent *event)
{
    if (!enabled())
        return WidgetStateData::eventFilter(object, event);

    // check event type
    switch (event->type()) {
    // enter event
    case QEvent::EnabledChange: {
        if (QWidget *widget = qobject_cast<QWidget *>(object)) {
            updateState(widget->isEnabled());
        }
        break;
    }

    default:
        break;
    }

    return WidgetStateData::eventFilter(object, event);
}
}
