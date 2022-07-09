#ifndef breezestackedwidget_datah
#define breezestackedwidget_datah

//////////////////////////////////////////////////////////////////////////////
// breezestackedwidgetdata.h
// data container for QStackedWidget transition
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezetransitiondata.h"

#include <QStackedWidget>

namespace Breeze
{

//* generic data
class StackedWidgetData : public TransitionData
{
    Q_OBJECT

public:
    //* constructor
    StackedWidgetData(QObject *, QStackedWidget *, int);

protected Q_SLOTS:

    //* initialize animation
    bool initializeAnimation() override;

    //* animate
    bool animate() override;

    //* finish animation
    void finishAnimation();

    //* called when target is destroyed
    void targetDestroyed();

private:
    //* target
    WeakPointer<QStackedWidget> _target;

    //* current index
    int _index;
};
}

#endif
