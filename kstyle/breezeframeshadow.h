/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "breeze.h"
#include "breezeaddeventfilter.h"
#include "breezehelper.h"

#include <QEvent>
#include <QObject>
#include <QSet>

#include <KColorScheme>
#include <QPaintEvent>
#include <QWidget>

namespace Breeze
{

//* shadow manager
class FrameShadowFactory : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit FrameShadowFactory(QObject *parent = nullptr);

    //* register widget
    bool registerWidget(QWidget *, const std::shared_ptr<Helper> &helper);

    //* unregister
    void unregisterWidget(QWidget *);

    //* true if widget is registered
    bool isRegistered(const QWidget *widget) const
    {
        return _registeredWidgets.contains(widget);
    }

    //* event filter
    bool eventFilter(QObject *, QEvent *) override;

    //* update state
    void updateState(const QWidget *, bool focus, bool hover, qreal opacity, AnimationMode) const;

    //* update shadows geometry
    void updateShadowsGeometry(const QObject *, QRect) const;

protected:
    //* install shadows on given widget
    void installShadows(QWidget *, const std::shared_ptr<Helper> &helper);

    //* remove shadows from widget
    void removeShadows(QWidget *);

    //* raise shadows
    void raiseShadows(QObject *) const;

    //* update shadows
    void update(QObject *) const;

    //* install shadow on given side
    void installShadow(QWidget *, const std::shared_ptr<Helper> &helper, Side area) const;

protected Q_SLOTS:

    //* triggered by object destruction
    void widgetDestroyed(QObject *);

private:
    //* needed to block ChildAdded events when creating shadows
    AddEventFilter _addEventFilter;

    //* set of registered widgets
    QSet<const QObject *> _registeredWidgets;
};

//* frame shadow
/** this allows the shadow to be painted over the widgets viewport */
class FrameShadow : public QWidget
{
    Q_OBJECT

public:
    //* constructor
    FrameShadow(Side, const std::shared_ptr<Helper> &helper);

    //* update geometry
    virtual void updateGeometry(QRect);

    //* update state
    void updateState(bool focus, bool hover, qreal opacity, AnimationMode);

protected:
    //* painting
    void paintEvent(QPaintEvent *) override;

    //* return viewport associated to parent widget
    QWidget *viewport() const;

private:
    //* helper
    std::shared_ptr<Helper> _helper;

    //* shadow area
    Side _area;

    //* margins
    /** offsets between update rect and parent widget rect. It is set via updateGeometry */
    QMargins _margins;

    //*@name widget state
    //@{
    bool _hasFocus = false;
    bool _mouseOver = false;
    qreal _opacity = -1;
    AnimationMode _mode = AnimationNone;
    //@}
};
}
