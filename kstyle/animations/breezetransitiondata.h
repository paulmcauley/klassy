#ifndef breezetransitiondata_h
#define breezetransitiondata_h

//////////////////////////////////////////////////////////////////////////////
// breezetransitiondata.h
// data container for generic transitions
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezetransitionwidget.h"

#include <QElapsedTimer>
#include <QObject>
#include <QWidget>

namespace Breeze
{

//* generic data
class TransitionData : public QObject
{
    Q_OBJECT

public:
    //* constructor
    TransitionData(QObject *parent, QWidget *target, int);

    //* destructor
    virtual ~TransitionData();

    //* enability
    virtual void setEnabled(bool value)
    {
        _enabled = value;
    }

    //* enability
    virtual bool enabled() const
    {
        return _enabled;
    }

    //* duration
    virtual void setDuration(int duration)
    {
        if (_transition) {
            _transition.data()->setDuration(duration);
        }
    }

    //* max render time
    void setMaxRenderTime(int value)
    {
        _maxRenderTime = value;
    }

    //* max renderTime
    const int &maxRenderTime() const
    {
        return _maxRenderTime;
    }

    //* start clock
    void startClock()
    {
        if (!_clock.isValid())
            _clock.start();
        else
            _clock.restart();
    }

    //* check if rendering is two slow
    bool slow() const
    {
        return !(!_clock.isValid() || _clock.elapsed() <= maxRenderTime());
    }

protected Q_SLOTS:

    //* initialize animation
    virtual bool initializeAnimation() = 0;

    //* animate
    virtual bool animate() = 0;

protected:
    //* returns true if one parent matches given class name
    inline bool hasParent(const QWidget *, const char *) const;

    //* transition widget
    virtual const TransitionWidget::Pointer &transition() const
    {
        return _transition;
    }

    //* used to avoid recursion when grabbing widgets
    void setRecursiveCheck(bool value)
    {
        _recursiveCheck = value;
    }

    //* used to avoid recursion when grabbing widgets
    bool recursiveCheck() const
    {
        return _recursiveCheck;
    }

private:
    //* enability
    bool _enabled = true;

    //* used to avoid recursion when grabbing widgets
    bool _recursiveCheck = false;

    //* timer used to detect slow rendering
    QElapsedTimer _clock;

    //* max render time
    /*! used to detect slow rendering */
    int _maxRenderTime = 200;

    //* animation handling
    TransitionWidget::Pointer _transition;
};

//_____________________________________________________________________________________
bool TransitionData::hasParent(const QWidget *widget, const char *className) const
{
    if (!widget)
        return false;
    for (QWidget *parent = widget->parentWidget(); parent; parent = parent->parentWidget()) {
        if (parent->inherits(className))
            return true;
    }

    return false;
}
}

#endif
