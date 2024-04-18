//////////////////////////////////////////////////////////////////////////////
// breezeblurhelper.cpp
// handle regions passed to kwin for blurring
// -------------------
//
// SPDX-FileCopyrightText: 2018 Alex Nemeth <alex.nemeth329@gmail.com>
//
// Largely rewritten from Oxygen widget style
// SPDX-FileCopyrightText: 2007 Thomas Luebking <thomas.luebking@web.de>
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeblurhelper.h"
#include "breezehelper.h"
#include "breezestyleconfigdata.h"

#include <KWindowEffects>

#include <QEvent>
#include <QMenu>
#include <QVector>

namespace Breeze
{
//___________________________________________________________
BlurHelper::BlurHelper(const std::shared_ptr<Helper> &helper)
    : QObject()
    , _helper(helper)
{
}

//___________________________________________________________
void BlurHelper::registerWidget(QWidget *widget)
{
    // install event filter
    addEventFilter(widget);

    // schedule shadow area repaint
    update(widget);
}

//___________________________________________________________
void BlurHelper::unregisterWidget(QWidget *widget)
{
    // remove event filter
    widget->removeEventFilter(this);
}

//___________________________________________________________
bool BlurHelper::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Hide:
    case QEvent::Show:
    case QEvent::Resize: {
        // cast to widget and check
        QWidget *widget(qobject_cast<QWidget *>(object));

        if (!widget) {
            break;
        }

        update(widget);
        break;
    }

    default:
        break;
    }

    // never eat events
    return false;
}

//___________________________________________________________
void BlurHelper::update(QWidget *widget) const
{
    /*
    directly from bespin code. Supposedly prevent playing with some 'pseudo-widgets'
    that have winId matching some other -random- window
    */
    if (!(widget->testAttribute(Qt::WA_WState_Created) || widget->internalWinId())) {
        return;
    }

    widget->winId(); // force creation of the window handle

    QRegion region;
    if (const auto menu = qobject_cast<QMenu *>(widget)) {
        region = _helper->menuFrameRegion(menu);
    }
    KWindowEffects::enableBlurBehind(widget->windowHandle(), true, region);

    // force update
    if (widget->isVisible()) {
        widget->update();
    }
}
}
