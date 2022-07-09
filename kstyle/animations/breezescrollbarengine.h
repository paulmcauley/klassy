/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezescrollbarengine_h
#define breezescrollbarengine_h

#include "breezescrollbardata.h"
#include "breezewidgetstateengine.h"

namespace Breeze
{

//* stores scrollbar hovered action and timeLine
class ScrollBarEngine : public WidgetStateEngine
{
    Q_OBJECT

public:
    //* constructor
    explicit ScrollBarEngine(QObject *parent)
        : WidgetStateEngine(parent)
    {
    }

    //* destructor
    virtual ~ScrollBarEngine()
    {
    }

    //* register scrollbar
    virtual bool registerWidget(QWidget *, AnimationModes);

    //*@name accessors
    //@{

    using WidgetStateEngine::isAnimated;
    using WidgetStateEngine::opacity;

    //* true if widget is animated
    virtual bool isAnimated(const QObject *, AnimationMode, QStyle::SubControl control);

    //* true if widget is animated
    virtual AnimationMode animationMode(const QObject *object, QStyle::SubControl control);

    //* animation opacity
    virtual qreal opacity(const QObject *object, QStyle::SubControl control);

    //* return true if given subcontrol is hovered
    virtual bool isHovered(const QObject *object, QStyle::SubControl control)
    {
        if (DataMap<WidgetStateData>::Value data = this->data(object, AnimationHover)) {
            return static_cast<const ScrollBarData *>(data.data())->isHovered(control);

        } else
            return false;
    }

    //* control rect associated to object
    virtual QRect subControlRect(const QObject *object, QStyle::SubControl control)
    {
        if (DataMap<WidgetStateData>::Value data = this->data(object, AnimationHover)) {
            return static_cast<const ScrollBarData *>(data.data())->subControlRect(control);

        } else
            return QRect();
    }

    //* mouse position
    virtual QPoint position(const QObject *object)
    {
        if (DataMap<WidgetStateData>::Value data = this->data(object, AnimationHover)) {
            return static_cast<const ScrollBarData *>(data.data())->position();

        } else
            return QPoint(-1, -1);
    }

    //@}

    //*@name modifiers
    //@{

    //* control rect
    virtual void setSubControlRect(const QObject *object, QStyle::SubControl control, const QRect &rect)
    {
        if (DataMap<WidgetStateData>::Value data = this->data(object, AnimationHover)) {
            static_cast<ScrollBarData *>(data.data())->setSubControlRect(control, rect);
        }
    }

    //@}
};
}

#endif
