//////////////////////////////////////////////////////////////////////////////
// breezestackedwidgetdata.cpp
// data container for QStackedWidget transition
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezestackedwidgetdata.h"

namespace Breeze
{

//______________________________________________________
StackedWidgetData::StackedWidgetData(QObject *parent, QStackedWidget *target, int duration)
    : TransitionData(parent, target, duration)
    , _target(target)
    , _index(target->currentIndex())
{
    // configure transition
    connect(_target.data(), &QObject::destroyed, this, &StackedWidgetData::targetDestroyed);
    connect(_target.data(), SIGNAL(currentChanged(int)), SLOT(animate()));

    // disable focus
    transition().data()->setAttribute(Qt::WA_NoMousePropagation, true);
    transition().data()->setFlag(TransitionWidget::PaintOnWidget, true);

    setMaxRenderTime(50);
}

//___________________________________________________________________
bool StackedWidgetData::initializeAnimation()
{
    // check enability
    if (!(_target && _target.data()->isVisible())) {
        return false;
    }

    // check index
    if (_target.data()->currentIndex() == _index) {
        return false;
    }

    // do not animate if either index or currentIndex is not valid
    // but update _index none the less
    if (_target.data()->currentIndex() < 0 || _index < 0) {
        _index = _target.data()->currentIndex();
        return false;
    }

    // get old widget (matching _index) and initialize transition
    if (QWidget *widget = _target.data()->widget(_index)) {
        transition().data()->setOpacity(0);
        startClock();
        transition().data()->setGeometry(widget->geometry());
        transition().data()->setStartPixmap(transition().data()->grab(widget));

        _index = _target.data()->currentIndex();
        return !slow();

    } else {
        _index = _target.data()->currentIndex();
        return false;
    }
}

//___________________________________________________________________
bool StackedWidgetData::animate()
{
    // check enability
    if (!enabled())
        return false;

    // initialize animation
    if (!initializeAnimation())
        return false;

    // show transition widget
    transition().data()->show();
    transition().data()->raise();
    transition().data()->animate();
    return true;
}

//___________________________________________________________________
void StackedWidgetData::finishAnimation()
{
    // disable updates on currentWidget
    if (_target && _target.data()->currentWidget()) {
        _target.data()->currentWidget()->setUpdatesEnabled(false);
    }

    // hide transition
    transition().data()->hide();

    // reenable updates and repaint
    if (_target && _target.data()->currentWidget()) {
        _target.data()->currentWidget()->setUpdatesEnabled(true);
        _target.data()->currentWidget()->repaint();
    }

    // invalidate start widget
    transition().data()->resetStartPixmap();
}

//___________________________________________________________________
void StackedWidgetData::targetDestroyed()
{
    setEnabled(false);
    _target.clear();
}
}
