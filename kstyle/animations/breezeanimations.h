/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef breezeanimations_h
#define breezeanimations_h

#include "breezebusyindicatorengine.h"
#include "breezedialengine.h"
#include "breezeheaderviewengine.h"
#include "breezescrollbarengine.h"
#include "breezespinboxengine.h"
#include "breezestackedwidgetengine.h"
#include "breezetabbarengine.h"
#include "breezetoolboxengine.h"
#include "breezewidgetstateengine.h"

#include <QList>
#include <QObject>

namespace Breeze
{

//* stores engines
class Animations : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit Animations(QObject *);

    //* register animations corresponding to given widget, depending on its type.
    void registerWidget(QWidget *widget) const;

    /** unregister all animations associated to a widget */
    void unregisterWidget(QWidget *widget) const;

    //* enability engine
    WidgetStateEngine &widgetEnabilityEngine() const
    {
        return *_widgetEnabilityEngine;
    }

    //* abstractButton engine
    WidgetStateEngine &widgetStateEngine() const
    {
        return *_widgetStateEngine;
    }

    //* editable combobox arrow hover engine
    WidgetStateEngine &comboBoxEngine() const
    {
        return *_comboBoxEngine;
    }

    //* Tool buttons arrow hover engine
    WidgetStateEngine &toolButtonEngine() const
    {
        return *_toolButtonEngine;
    }

    //* item view engine
    WidgetStateEngine &inputWidgetEngine() const
    {
        return *_inputWidgetEngine;
    }

    //* busy indicator
    BusyIndicatorEngine &busyIndicatorEngine() const
    {
        return *_busyIndicatorEngine;
    }

    //* header view engine
    HeaderViewEngine &headerViewEngine() const
    {
        return *_headerViewEngine;
    }

    //* scrollbar engine
    ScrollBarEngine &scrollBarEngine() const
    {
        return *_scrollBarEngine;
    }

    //* dial engine
    DialEngine &dialEngine() const
    {
        return *_dialEngine;
    }

    //* spinbox engine
    SpinBoxEngine &spinBoxEngine() const
    {
        return *_spinBoxEngine;
    }

    //* tabbar
    TabBarEngine &tabBarEngine() const
    {
        return *_tabBarEngine;
    }

    //* toolbox
    ToolBoxEngine &toolBoxEngine() const
    {
        return *_toolBoxEngine;
    }

    //* setup engines
    void setupEngines();

protected Q_SLOTS:

    //* enregister engine
    void unregisterEngine(QObject *);

private:
    //* register new engine
    void registerEngine(BaseEngine *);

    //* busy indicator
    BusyIndicatorEngine *_busyIndicatorEngine = nullptr;

    //* headerview hover effect
    HeaderViewEngine *_headerViewEngine = nullptr;

    //* widget enability engine
    WidgetStateEngine *_widgetEnabilityEngine = nullptr;

    //* abstract button engine
    WidgetStateEngine *_widgetStateEngine = nullptr;

    //* editable combobox arrow hover effect
    WidgetStateEngine *_comboBoxEngine = nullptr;

    //* mennu toolbutton arrow hover effect
    WidgetStateEngine *_toolButtonEngine = nullptr;

    //* item view engine
    WidgetStateEngine *_inputWidgetEngine = nullptr;

    //* scrollbar engine
    ScrollBarEngine *_scrollBarEngine = nullptr;

    //* dial engine
    DialEngine *_dialEngine = nullptr;

    //* spinbox engine
    SpinBoxEngine *_spinBoxEngine = nullptr;

    //* stacked widget engine
    StackedWidgetEngine *_stackedWidgetEngine = nullptr;

    //* tabbar engine
    TabBarEngine *_tabBarEngine = nullptr;

    //* toolbar engine
    ToolBoxEngine *_toolBoxEngine = nullptr;

    //* keep list of existing engines
    QList<BaseEngine::Pointer> _engines;
};
}

#endif
