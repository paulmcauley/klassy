/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezesplitterproxy_h
#define breezesplitterproxy_h

#include "breeze.h"
#include "breezeaddeventfilter.h"

#include <QEvent>
#include <QHoverEvent>
#include <QMainWindow>
#include <QMap>
#include <QMouseEvent>
#include <QSplitterHandle>
#include <QWidget>

namespace Breeze
{

class SplitterProxy;

//* factory
class SplitterFactory : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit SplitterFactory(QObject *parent)
        : QObject(parent)
        , _enabled(false)
    {
    }

    //* enabled state
    void setEnabled(bool);

    //* register widget
    bool registerWidget(QWidget *);

    //* unregister widget
    void unregisterWidget(QWidget *);

private:
    //* enabled state
    bool _enabled;

    //* needed to block ChildAdded events when creating proxy
    AddEventFilter _addEventFilter;

    //* pointer to SplitterProxy
    using SplitterProxyPointer = WeakPointer<SplitterProxy>;

    //* registered widgets
    using WidgetMap = QMap<QWidget *, SplitterProxyPointer>;
    WidgetMap _widgets;
};

//* splitter 'proxy' widget, with extended hit area
class SplitterProxy : public QWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit SplitterProxy(QWidget *, bool = false);

    //* event filter
    bool eventFilter(QObject *, QEvent *) override;

    //* enable state
    void setEnabled(bool);

    //* enable state
    bool enabled() const
    {
        return _enabled;
    }

protected:
    //* event handler
    bool event(QEvent *) override;

protected:
    //* reset 'true' splitter widget
    void clearSplitter();

    //* keep track of 'true' splitter widget
    void setSplitter(QWidget *);

private:
    //* enabled state
    bool _enabled;

    //* splitter object
    WeakPointer<QWidget> _splitter;

    //* hook
    QPoint _hook;

    //* timer id
    int _timerId;
};
}

#endif
