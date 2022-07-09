/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezeheaderview_data_h
#define breezeheaderview_data_h

#include "breezeanimationdata.h"
#include <QHeaderView>

namespace Breeze
{

//* headerviews
class HeaderViewData : public AnimationData
{
    Q_OBJECT

    //* declare opacity property
    Q_PROPERTY(qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity)
    Q_PROPERTY(qreal previousOpacity READ previousOpacity WRITE setPreviousOpacity)

public:
    //* constructor
    HeaderViewData(QObject *parent, QWidget *target, int duration);

    //* duration
    void setDuration(int duration) override
    {
        currentIndexAnimation().data()->setDuration(duration);
        previousIndexAnimation().data()->setDuration(duration);
    }

    //* update state
    bool updateState(const QPoint &, bool);

    //*@name current index handling
    //@{

    //* current opacity
    qreal currentOpacity() const
    {
        return _current._opacity;
    }

    //* current opacity
    void setCurrentOpacity(qreal value)
    {
        value = digitize(value);
        if (_current._opacity == value)
            return;
        _current._opacity = value;
        setDirty();
    }

    //* current index
    int currentIndex() const
    {
        return _current._index;
    }

    //* current index
    void setCurrentIndex(int index)
    {
        _current._index = index;
    }

    //* current index animation
    const Animation::Pointer &currentIndexAnimation() const
    {
        return _current._animation;
    }

    //@}

    //*@name previous index handling
    //@{

    //* previous opacity
    qreal previousOpacity() const
    {
        return _previous._opacity;
    }

    //* previous opacity
    void setPreviousOpacity(qreal value)
    {
        value = digitize(value);
        if (_previous._opacity == value)
            return;
        _previous._opacity = value;
        setDirty();
    }

    //* previous index
    int previousIndex() const
    {
        return _previous._index;
    }

    //* previous index
    void setPreviousIndex(int index)
    {
        _previous._index = index;
    }

    //* previous index Animation
    const Animation::Pointer &previousIndexAnimation() const
    {
        return _previous._animation;
    }

    //@}

    //* return Animation associated to action at given position, if any
    Animation::Pointer animation(const QPoint &position) const;

    //* return opacity associated to action at given position, if any
    qreal opacity(const QPoint &position) const;

protected:
    //* dirty
    void setDirty() const override;

private:
    //* container for needed animation data
    class Data
    {
    public:
        //* default constructor
        Data()
            : _opacity(0)
            , _index(-1)
        {
        }

        Animation::Pointer _animation;
        qreal _opacity;
        int _index;
    };

    //* current tab animation data (for hover enter animations)
    Data _current;

    //* previous tab animations data (for hover leave animations)
    Data _previous;
};
}

#endif
