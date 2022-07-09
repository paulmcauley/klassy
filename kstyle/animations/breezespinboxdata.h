/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezespinbox_data_h
#define breezespinbox_data_h

#include "breezeanimationdata.h"

#include <QStyle>

namespace Breeze
{

//* handles spinbox arrows hover
class SpinBoxData : public AnimationData
{
    Q_OBJECT

    //* declare opacity property
    Q_PROPERTY(qreal upArrowOpacity READ upArrowOpacity WRITE setUpArrowOpacity)
    Q_PROPERTY(qreal downArrowOpacity READ downArrowOpacity WRITE setDownArrowOpacity)

public:
    //* constructor
    SpinBoxData(QObject *, QWidget *, int);

    //* animation state
    bool updateState(QStyle::SubControl subControl, bool value)
    {
        if (subControl == QStyle::SC_SpinBoxUp)
            return _upArrowData.updateState(value);
        else if (subControl == QStyle::SC_SpinBoxDown)
            return _downArrowData.updateState(value);
        else
            return false;
    }

    //* animation state
    bool isAnimated(QStyle::SubControl subControl) const
    {
        return ((subControl == QStyle::SC_SpinBoxUp && upArrowAnimation().data()->isRunning())
                || (subControl == QStyle::SC_SpinBoxDown && downArrowAnimation().data()->isRunning()));
    }

    //* opacity
    qreal opacity(QStyle::SubControl subControl) const
    {
        if (subControl == QStyle::SC_SpinBoxUp)
            return upArrowOpacity();
        else if (subControl == QStyle::SC_SpinBoxDown)
            return downArrowOpacity();
        else
            return OpacityInvalid;
    }

    //* duration
    void setDuration(int duration) override
    {
        upArrowAnimation().data()->setDuration(duration);
        downArrowAnimation().data()->setDuration(duration);
    }

    //*@name up arrow animation
    //@{

    //* opacity
    qreal upArrowOpacity() const
    {
        return _upArrowData._opacity;
    }

    //* opacity
    void setUpArrowOpacity(qreal value)
    {
        value = digitize(value);
        if (_upArrowData._opacity == value)
            return;
        _upArrowData._opacity = value;
        setDirty();
    }

    //* animation
    Animation::Pointer upArrowAnimation() const
    {
        return _upArrowData._animation;
    }

    //@}

    //*@name down arrow animation
    //@{

    //* opacity
    qreal downArrowOpacity() const
    {
        return _downArrowData._opacity;
    }

    //* opacity
    void setDownArrowOpacity(qreal value)
    {
        value = digitize(value);
        if (_downArrowData._opacity == value)
            return;
        _downArrowData._opacity = value;
        setDirty();
    }

    //* animation
    Animation::Pointer downArrowAnimation() const
    {
        return _downArrowData._animation;
    }

    //@}

private:
    //* container for needed animation data
    class Data
    {
    public:
        //* default constructor
        Data()
            : _state(false)
            , _opacity(0)
        {
        }

        //* state
        bool updateState(bool);

        //* arrow state
        bool _state;

        //* animation
        Animation::Pointer _animation;

        //* opacity
        qreal _opacity;
    };

    //* up arrow data
    Data _upArrowData;

    //* down arrow data
    Data _downArrowData;
};
}

#endif
