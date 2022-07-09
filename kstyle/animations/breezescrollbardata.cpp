/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezescrollbardata.h"

#include <QHoverEvent>
#include <QScrollBar>
#include <QStyleOptionSlider>

Q_GUI_EXPORT QStyleOptionSlider qt_qscrollbarStyleOption(QScrollBar *);

namespace Breeze
{

//______________________________________________
ScrollBarData::ScrollBarData(QObject *parent, QWidget *target, int duration)
    : WidgetStateData(parent, target, duration)
    , _position(-1, -1)
{
    target->installEventFilter(this);

    _addLineData._animation = new Animation(duration, this);
    _subLineData._animation = new Animation(duration, this);
    _grooveData._animation = new Animation(duration, this);

    connect(addLineAnimation().data(), &QAbstractAnimation::finished, this, &ScrollBarData::clearAddLineRect);
    connect(subLineAnimation().data(), &QAbstractAnimation::finished, this, &ScrollBarData::clearSubLineRect);

    // setup animation
    setupAnimation(addLineAnimation(), "addLineOpacity");
    setupAnimation(subLineAnimation(), "subLineOpacity");
    setupAnimation(grooveAnimation(), "grooveOpacity");
}

//______________________________________________
bool ScrollBarData::eventFilter(QObject *object, QEvent *event)
{
    if (object != target().data()) {
        return WidgetStateData::eventFilter(object, event);
    }

    // check event type
    switch (event->type()) {
    case QEvent::HoverEnter:
        setGrooveHovered(true);
        grooveAnimation().data()->setDirection(Animation::Forward);
        if (!grooveAnimation().data()->isRunning())
            grooveAnimation().data()->start();
        break;

    case QEvent::HoverMove:
        hoverMoveEvent(object, event);
        break;

    case QEvent::HoverLeave:
        setGrooveHovered(false);
        grooveAnimation().data()->setDirection(Animation::Backward);
        if (!grooveAnimation().data()->isRunning())
            grooveAnimation().data()->start();
        hoverLeaveEvent(object, event);
        break;

    default:
        break;
    }

    return WidgetStateData::eventFilter(object, event);
}

//______________________________________________
const Animation::Pointer &ScrollBarData::animation(QStyle::SubControl subcontrol) const
{
    switch (subcontrol) {
    default:
    case QStyle::SC_ScrollBarSlider:
        return animation();

    case QStyle::SC_ScrollBarAddLine:
        return addLineAnimation();

    case QStyle::SC_ScrollBarSubLine:
        return subLineAnimation();

    case QStyle::SC_ScrollBarGroove:
        return grooveAnimation();
    }
}

//______________________________________________
qreal ScrollBarData::opacity(QStyle::SubControl subcontrol) const
{
    switch (subcontrol) {
    default:
    case QStyle::SC_ScrollBarSlider:
        return opacity();

    case QStyle::SC_ScrollBarAddLine:
        return addLineOpacity();

    case QStyle::SC_ScrollBarSubLine:
        return subLineOpacity();

    case QStyle::SC_ScrollBarGroove:
        return grooveOpacity();
    }
}

//______________________________________________
void ScrollBarData::hoverMoveEvent(QObject *object, QEvent *event)
{
    // try cast object to scrollbar
    QScrollBar *scrollBar(qobject_cast<QScrollBar *>(object));
    if (!scrollBar || scrollBar->isSliderDown())
        return;

    // retrieve scrollbar option
    QStyleOptionSlider opt(qt_qscrollbarStyleOption(scrollBar));

    // cast event
    QHoverEvent *hoverEvent = static_cast<QHoverEvent *>(event);
    QStyle::SubControl hoverControl = scrollBar->style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, hoverEvent->pos(), scrollBar);

    // update hover state
    updateAddLineArrow(hoverControl);
    updateSubLineArrow(hoverControl);

    // store position
    _position = hoverEvent->pos();
}

//______________________________________________
void ScrollBarData::hoverLeaveEvent(QObject *, QEvent *)
{
    // reset hover state
    updateSubLineArrow(QStyle::SC_None);
    updateAddLineArrow(QStyle::SC_None);

    // reset mouse position
    _position = QPoint(-1, -1);
}

//_____________________________________________________________________
void ScrollBarData::updateSubLineArrow(QStyle::SubControl hoverControl)
{
    if (hoverControl == QStyle::SC_ScrollBarSubLine) {
        if (!subLineArrowHovered()) {
            setSubLineArrowHovered(true);
            if (enabled()) {
                subLineAnimation().data()->setDirection(Animation::Forward);
                if (!subLineAnimation().data()->isRunning())
                    subLineAnimation().data()->start();
            } else
                setDirty();
        }

    } else {
        if (subLineArrowHovered()) {
            setSubLineArrowHovered(false);
            if (enabled()) {
                subLineAnimation().data()->setDirection(Animation::Backward);
                if (!subLineAnimation().data()->isRunning())
                    subLineAnimation().data()->start();
            } else
                setDirty();
        }
    }
}

//_____________________________________________________________________
void ScrollBarData::updateAddLineArrow(QStyle::SubControl hoverControl)
{
    if (hoverControl == QStyle::SC_ScrollBarAddLine) {
        if (!addLineArrowHovered()) {
            setAddLineArrowHovered(true);
            if (enabled()) {
                addLineAnimation().data()->setDirection(Animation::Forward);
                if (!addLineAnimation().data()->isRunning())
                    addLineAnimation().data()->start();
            } else
                setDirty();
        }

    } else {
        if (addLineArrowHovered()) {
            setAddLineArrowHovered(false);
            if (enabled()) {
                addLineAnimation().data()->setDirection(Animation::Backward);
                if (!addLineAnimation().data()->isRunning())
                    addLineAnimation().data()->start();
            } else
                setDirty();
        }
    }
}
}
