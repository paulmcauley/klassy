#ifndef breezeblurhelper_h
#define breezeblurhelper_h

//////////////////////////////////////////////////////////////////////////////
// breezeblurhelper.h
// handle regions passed to kwin for blurring
// -------------------
//
// SPDX-FileCopyrightText: 2018 Alex Nemeth <alex.nemeth329@gmail.com>
//
// Largely rewritten from Oxygen widget style
// SPDX-FileCopyrightText: 2007 Thomas Luebking <thomas.luebking@web.de>
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breeze.h"
#include "breezehelper.h"

#include <QHash>
#include <QObject>

namespace Breeze
{
class BlurHelper : public QObject
{
    Q_OBJECT

public:
    //! constructor
    BlurHelper(QObject *);

    //! register widget
    void registerWidget(QWidget *);

    //! register widget
    void unregisterWidget(QWidget *);

    //! event filter
    bool eventFilter(QObject *, QEvent *) override;

protected:
    //! install event filter to object, in a unique way
    void addEventFilter(QObject *object)
    {
        object->removeEventFilter(this);
        object->installEventFilter(this);
    }

    //! update blur regions for given widget
    void update(QWidget *) const;
};
}

#endif
