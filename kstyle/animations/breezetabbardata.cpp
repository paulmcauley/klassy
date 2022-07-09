/*
 * breezetabbardata.cpp
 * data container for QTabBar animations
 *
 * SPDX-FileCopyrightText: 2009, 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later OR MIT
 */

#include "breezetabbardata.h"

#include <QHoverEvent>

namespace Breeze
{

//______________________________________________
TabBarData::TabBarData(QObject *parent, QWidget *target, int duration)
    : AnimationData(parent, target)
{
    _current._animation = new Animation(duration, this);
    setupAnimation(currentIndexAnimation(), "currentOpacity");
    currentIndexAnimation().data()->setDirection(Animation::Forward);

    _previous._animation = new Animation(duration, this);
    setupAnimation(previousIndexAnimation(), "previousOpacity");
    previousIndexAnimation().data()->setDirection(Animation::Backward);
}

//______________________________________________
Animation::Pointer TabBarData::animation(const QPoint &position) const
{
    if (!enabled())
        return Animation::Pointer();

    const QTabBar *local(qobject_cast<const QTabBar *>(target().data()));
    if (!local)
        return Animation::Pointer();

    int index(local->tabAt(position));
    if (index < 0)
        return Animation::Pointer();
    else if (index == currentIndex())
        return currentIndexAnimation();
    else if (index == previousIndex())
        return previousIndexAnimation();
    else
        return Animation::Pointer();
}

//______________________________________________
bool TabBarData::updateState(const QPoint &position, bool hovered)
{
    if (!enabled())
        return false;

    const QTabBar *local(qobject_cast<const QTabBar *>(target().data()));
    if (!local)
        return false;

    int index(local->tabAt(position));
    if (index < 0)
        return false;

    if (hovered) {
        if (index != currentIndex()) {
            if (currentIndex() >= 0) {
                setPreviousIndex(currentIndex());
                setCurrentIndex(-1);
                previousIndexAnimation().data()->restart();
            }

            setCurrentIndex(index);
            currentIndexAnimation().data()->restart();
            return true;

        } else
            return false;

    } else if (index == currentIndex()) {
        setPreviousIndex(currentIndex());
        setCurrentIndex(-1);
        previousIndexAnimation().data()->restart();
        return true;

    } else
        return false;
}

//______________________________________________
qreal TabBarData::opacity(const QPoint &position) const
{
    if (!enabled())
        return OpacityInvalid;

    const QTabBar *local(qobject_cast<const QTabBar *>(target().data()));
    if (!local)
        return OpacityInvalid;

    int index(local->tabAt(position));
    if (index < 0)
        return OpacityInvalid;
    else if (index == currentIndex())
        return currentOpacity();
    else if (index == previousIndex())
        return previousOpacity();
    else
        return OpacityInvalid;
}
}
