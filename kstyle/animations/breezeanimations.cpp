/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezeanimations.h"
#include "breezepropertynames.h"
#include "breezestyleconfigdata.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDial>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QProgressBar>
#include <QRadioButton>
#include <QScrollBar>
#include <QSpinBox>
#include <QTextEdit>
#include <QToolBox>
#include <QToolButton>

namespace Breeze
{

//____________________________________________________________
Animations::Animations()
    : QObject()
{
    _widgetEnabilityEngine = new WidgetStateEngine(this);
    _busyIndicatorEngine = new BusyIndicatorEngine(this);
    _comboBoxEngine = new WidgetStateEngine(this);
    _toolButtonEngine = new WidgetStateEngine(this);
    _spinBoxEngine = new SpinBoxEngine(this);
    _toolBoxEngine = new ToolBoxEngine(this);

    registerEngine(_headerViewEngine = new HeaderViewEngine(this));
    registerEngine(_widgetStateEngine = new WidgetStateEngine(this));
    registerEngine(_inputWidgetEngine = new WidgetStateEngine(this));
    registerEngine(_scrollBarEngine = new ScrollBarEngine(this));
    registerEngine(_stackedWidgetEngine = new StackedWidgetEngine(this));
    registerEngine(_tabBarEngine = new TabBarEngine(this));
    registerEngine(_dialEngine = new DialEngine(this));
}

//____________________________________________________________
void Animations::setupEngines()
{
    // animation steps
    AnimationData::setSteps(StyleConfigData::animationSteps());

    const bool animationsEnabled(StyleConfigData::animationsEnabled());
    const int animationsDuration(StyleConfigData::animationsDuration());

    _widgetEnabilityEngine->setEnabled(animationsEnabled);
    _comboBoxEngine->setEnabled(animationsEnabled);
    _toolButtonEngine->setEnabled(animationsEnabled);
    _spinBoxEngine->setEnabled(animationsEnabled);
    _toolBoxEngine->setEnabled(animationsEnabled);

    _widgetEnabilityEngine->setDuration(animationsDuration);
    _comboBoxEngine->setDuration(animationsDuration);
    _toolButtonEngine->setDuration(animationsDuration);
    _spinBoxEngine->setDuration(animationsDuration);
    _stackedWidgetEngine->setDuration(animationsDuration);
    _toolBoxEngine->setDuration(animationsDuration);

    // registered engines
    for (const BaseEngine::Pointer &engine : std::as_const(_engines)) {
        engine.data()->setEnabled(animationsEnabled);
        engine.data()->setDuration(animationsDuration);
    }

    // stacked widget transition has an extra flag for animations
    _stackedWidgetEngine->setEnabled(animationsEnabled && StyleConfigData::stackedWidgetTransitionsEnabled());

    // busy indicator
    _busyIndicatorEngine->setEnabled(StyleConfigData::progressBarAnimated());
    _busyIndicatorEngine->setDuration(StyleConfigData::progressBarBusyStepDuration());
}

//____________________________________________________________
void Animations::registerWidget(QObject *widget) const
{
    if (!widget) {
        return;
    }

    // check against noAnimations property
    QVariant propertyValue(widget->property(PropertyNames::noAnimations));
    if (propertyValue.isValid() && propertyValue.toBool()) {
        return;
    }

    // all widgets are registered to the enability engine.
    _widgetEnabilityEngine->registerWidget(widget, AnimationEnable);

    // install animation timers
    // for optimization, one should put with most used widgets here first

    // KQuickStyleItem.
    const QString elementType = widget->property("elementType").toString();
    if (!elementType.isEmpty()) {
        if (elementType == QLatin1String("checkbox") || elementType == QLatin1String("radiobutton")) {
            _widgetStateEngine->registerWidget(widget, AnimationHover | AnimationFocus | AnimationPressed);
        }
        return;
    }

    // buttons
    if (auto toolButton = qobject_cast<QToolButton *>(widget)) {
        _toolButtonEngine->registerWidget(toolButton, AnimationHover | AnimationFocus);
        _widgetStateEngine->registerWidget(toolButton, AnimationHover | AnimationFocus);

    } else if (qobject_cast<QCheckBox *>(widget) || qobject_cast<QRadioButton *>(widget)) {
        _widgetStateEngine->registerWidget(widget, AnimationHover | AnimationFocus | AnimationPressed);

    } else if (auto button = qobject_cast<QAbstractButton *>(widget)) {
        // register to toolbox engine if needed
        if (auto toolBox = qobject_cast<QToolBox *>(widget->parent())) {
            _toolBoxEngine->registerWidget(toolBox);
        }

        _widgetStateEngine->registerWidget(button, AnimationHover | AnimationFocus);

    }

    // groupboxes
    else if (QGroupBox *groupBox = qobject_cast<QGroupBox *>(widget)) {
        if (groupBox->isCheckable()) {
            _widgetStateEngine->registerWidget(groupBox, AnimationHover | AnimationFocus);
        }
    }

    // sliders
    else if (auto scrollBar = qobject_cast<QScrollBar *>(widget)) {
        _scrollBarEngine->registerWidget(scrollBar, AnimationHover | AnimationFocus);
    } else if (auto slider = qobject_cast<QSlider *>(widget)) {
        _widgetStateEngine->registerWidget(slider, AnimationHover | AnimationFocus);
    } else if (auto dial = qobject_cast<QDial *>(widget)) {
        _dialEngine->registerWidget(dial, AnimationHover | AnimationFocus);
    }

    // progress bar
    else if (qobject_cast<QProgressBar *>(widget)) {
        _busyIndicatorEngine->registerWidget(widget);
    }

    // combo box
    else if (qobject_cast<QComboBox *>(widget)) {
        _comboBoxEngine->registerWidget(widget, AnimationHover);
        _inputWidgetEngine->registerWidget(widget, AnimationHover | AnimationFocus);
    }

    // spinbox
    else if (qobject_cast<QSpinBox *>(widget)) {
        _spinBoxEngine->registerWidget(widget);
        _inputWidgetEngine->registerWidget(widget, AnimationHover | AnimationFocus);
    }

    // editors
    else if (qobject_cast<QLineEdit *>(widget)) {
        _inputWidgetEngine->registerWidget(widget, AnimationHover | AnimationFocus);
    } else if (qobject_cast<QTextEdit *>(widget)) {
        _inputWidgetEngine->registerWidget(widget, AnimationHover | AnimationFocus);
    } else if (widget->inherits("KTextEditor::View")) {
        _inputWidgetEngine->registerWidget(widget, AnimationHover | AnimationFocus);
    }

    // header views
    // need to come before abstract item view, otherwise is skipped
    else if (auto headerView = qobject_cast<QHeaderView *>(widget)) {
        _headerViewEngine->registerWidget(headerView);
    }

    // lists
    else if (auto itemView = qobject_cast<QAbstractItemView *>(widget)) {
        _inputWidgetEngine->registerWidget(itemView, AnimationHover | AnimationFocus);
    }

    // tabbar
    else if (auto tabBar = qobject_cast<QTabBar *>(widget)) {
        _tabBarEngine->registerWidget(tabBar);
    }

    // scrollarea
    else if (QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(widget)) {
        if (scrollArea->frameShadow() == QFrame::Sunken && (scrollArea->focusPolicy() & Qt::StrongFocus)) {
            _inputWidgetEngine->registerWidget(scrollArea, AnimationHover | AnimationFocus);
        }
    }

    // stacked widgets
    if (QStackedWidget *stack = qobject_cast<QStackedWidget *>(widget)) {
        _stackedWidgetEngine->registerWidget(stack);
    }
}

//____________________________________________________________
void Animations::unregisterWidget(QObject *widget) const
{
    if (!widget) {
        return;
    }

    _widgetEnabilityEngine->unregisterWidget(widget);
    _spinBoxEngine->unregisterWidget(widget);
    _comboBoxEngine->unregisterWidget(widget);
    _busyIndicatorEngine->unregisterWidget(widget);

    // the following allows some optimization of widget unregistration
    // it assumes that a widget can be registered atmost in one of the
    // engines stored in the list.
    for (const BaseEngine::Pointer &engine : std::as_const(_engines)) {
        if (engine && engine.data()->unregisterWidget(widget)) {
            break;
        }
    }
}

//_______________________________________________________________
void Animations::unregisterEngine(QObject *object)
{
    int index(_engines.indexOf(qobject_cast<BaseEngine *>(object)));
    if (index >= 0) {
        _engines.removeAt(index);
    }
}

//_______________________________________________________________
void Animations::registerEngine(BaseEngine *engine)
{
    _engines.append(engine);
    connect(engine, &QObject::destroyed, this, &Animations::unregisterEngine);
}
}
