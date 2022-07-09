/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezewidgetexplorer_h
#define breezewidgetexplorer_h

#include <QEvent>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QWidget>

namespace Breeze
{

//* print widget's and parent's information on mouse click
class WidgetExplorer : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit WidgetExplorer(QObject *);

    //* enable
    bool enabled() const;

    //* enable
    void setEnabled(bool);

    //* widget rects
    void setDrawWidgetRects(bool value)
    {
        _drawWidgetRects = value;
    }

    //* event filter
    bool eventFilter(QObject *, QEvent *) override;

protected:
    //* event type
    QString eventType(const QEvent::Type &) const;

    //* print widget information
    QString widgetInformation(const QWidget *) const;

private:
    //* enable state
    bool _enabled = false;

    //* widget rects
    bool _drawWidgetRects = false;

    //* map event types to string
    QMap<QEvent::Type, QString> _eventTypes;
};
}

#endif
