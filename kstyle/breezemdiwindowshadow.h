/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezemdiwindowshadow_h
#define breezemdiwindowshadow_h

#include <QEvent>
#include <QObject>
#include <QSet>

#include <QPaintEvent>
#include <QPointer>
#include <QWidget>

#include "breezetileset.h"

namespace Breeze
{

class ShadowHelper;

//* frame shadow
/** this allows the shadow to be painted over the widgets viewport */
class MdiWindowShadow : public QWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit MdiWindowShadow(QWidget *, const TileSet &);

    //* update geometry
    void updateGeometry();

    //* update ZOrder
    void updateZOrder();

    //* set associated window
    void setWidget(QWidget *value)
    {
        _widget = value;
    }

    //* associated window
    QWidget *widget() const
    {
        return _widget;
    }

protected:
    //* painting
    void paintEvent(QPaintEvent *) override;

private:
    //* associated widget
    QWidget *_widget = nullptr;

    //* tileset rect, used for painting
    QRect _shadowTilesRect;

    //* tileset used to draw shadow
    TileSet _shadowTiles;
};

//* shadow manager
class MdiWindowShadowFactory : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit MdiWindowShadowFactory(QObject *);

    //* set shadow helper
    void setShadowHelper(ShadowHelper *shadowHelper)
    {
        _shadowHelper = shadowHelper;
    }

    //* register widget
    bool registerWidget(QWidget *);

    //* unregister
    void unregisterWidget(QWidget *);

    //* true if widget is registered
    bool isRegistered(const QObject *widget) const
    {
        return _registeredWidgets.contains(widget);
    }

    //* event filter
    bool eventFilter(QObject *, QEvent *) override;

protected:
    //* find shadow matching a given object
    MdiWindowShadow *findShadow(QObject *) const;

    //* install shadows on given widget
    void installShadow(QObject *);

    //* remove shadows from widget
    void removeShadow(QObject *);

    //* hide shadows
    void hideShadows(QObject *object) const
    {
        if (MdiWindowShadow *windowShadow = findShadow(object)) {
            windowShadow->hide();
        }
    }

    //* update ZOrder
    void updateShadowZOrder(QObject *object) const
    {
        if (MdiWindowShadow *windowShadow = findShadow(object)) {
            if (!windowShadow->isVisible())
                windowShadow->show();
            windowShadow->updateZOrder();
        }
    }

    //* update shadows geometry
    void updateShadowGeometry(QObject *object) const
    {
        if (MdiWindowShadow *windowShadow = findShadow(object)) {
            windowShadow->updateGeometry();
        }
    }

    //* update shadows
    void update(QObject *object) const
    {
        if (MdiWindowShadow *windowShadow = findShadow(object)) {
            windowShadow->update();
        }
    }

protected Q_SLOTS:

    //* triggered by object destruction
    void widgetDestroyed(QObject *);

private:
    //* set of registered widgets
    QSet<const QObject *> _registeredWidgets;

    //* shadow helper used to generate the shadows
    QPointer<ShadowHelper> _shadowHelper;
};
}

#endif
