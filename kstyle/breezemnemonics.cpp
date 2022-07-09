/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezemnemonics.h"

#include <QKeyEvent>
#include <QWidget>

namespace Breeze
{

//____________________________________________________
void Mnemonics::setMode(int mode)
{
    switch (mode) {
    case StyleConfigData::MN_NEVER:
        qApp->removeEventFilter(this);
        setEnabled(false);
        break;

    default:
    case StyleConfigData::MN_ALWAYS:
        qApp->removeEventFilter(this);
        setEnabled(true);
        break;

    case StyleConfigData::MN_AUTO:
        qApp->removeEventFilter(this);
        qApp->installEventFilter(this);
        setEnabled(false);
        break;
    }
}

//____________________________________________________
bool Mnemonics::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Alt) {
            setEnabled(true);
        }
        break;

    case QEvent::KeyRelease:
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Alt) {
            setEnabled(false);
        }
        break;

    case QEvent::ApplicationStateChange: {
        setEnabled(false);
    } break;

    default:
        break;
    }

    return false;
}

//____________________________________________________
void Mnemonics::setEnabled(bool value)
{
    if (_enabled == value)
        return;

    _enabled = value;

    // update all top level widgets
    foreach (QWidget *widget, qApp->topLevelWidgets()) {
        widget->update();
    }
}
}
