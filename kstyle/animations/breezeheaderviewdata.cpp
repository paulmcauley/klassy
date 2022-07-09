/*
 * SPDX-FileCopyrightText: 2009, 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later OR MIT
 */

#include "breezeheaderviewdata.h"

#include <QHoverEvent>
#include <QTextStream>

namespace Breeze
{

//______________________________________________
HeaderViewData::HeaderViewData(QObject *parent, QWidget *target, int duration)
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
bool HeaderViewData::updateState(const QPoint &position, bool hovered)
{
    if (!enabled())
        return false;

    const QHeaderView *local(qobject_cast<const QHeaderView *>(target().data()));
    if (!local)
        return false;

    const int index(local->logicalIndexAt(position));
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
Animation::Pointer HeaderViewData::animation(const QPoint &position) const
{
    if (!enabled())
        return Animation::Pointer();

    const QHeaderView *local(qobject_cast<const QHeaderView *>(target().data()));
    if (!local)
        return Animation::Pointer();

    int index(local->logicalIndexAt(position));
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
qreal HeaderViewData::opacity(const QPoint &position) const
{
    if (!enabled())
        return OpacityInvalid;

    const QHeaderView *local(qobject_cast<const QHeaderView *>(target().data()));
    if (!local)
        return OpacityInvalid;

    int index(local->logicalIndexAt(position));
    if (index < 0)
        return OpacityInvalid;
    else if (index == currentIndex())
        return currentOpacity();
    else if (index == previousIndex())
        return previousOpacity();
    else
        return OpacityInvalid;
}

//__________________________________________________________
void HeaderViewData::setDirty() const
{
    QHeaderView *header = qobject_cast<QHeaderView *>(target().data());
    if (!header)
        return;

    // get first and last index, sorted
    const int lastIndex(qMax(previousIndex(), currentIndex()));
    if (lastIndex < 0)
        return;

    int firstIndex(qMin(previousIndex(), currentIndex()));
    if (firstIndex < 0)
        firstIndex = lastIndex;

    // find relevant rectangle to be updated, in viewport coordinate
    QWidget *viewport(header->viewport());
    const int left = header->sectionViewportPosition(firstIndex);
    const int right = header->sectionViewportPosition(lastIndex) + header->sectionSize(lastIndex);

    // trigger update
    if (header->orientation() == Qt::Horizontal)
        viewport->update(left, 0, right - left, header->height());
    else
        viewport->update(0, left, header->width(), right - left);
}
}
