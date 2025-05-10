/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "breeze.h"
#include "ui_breezestyleconfig.h"

#include <KPageWidget>

namespace Breeze
{

class StyleConfig : public QWidget, Ui::BreezeStyleConfig
{
    Q_OBJECT

public:
    //* constructor
    explicit StyleConfig(QWidget *);

    //* destructor
    virtual ~StyleConfig()
    {
    }

Q_SIGNALS:

    //* emitted whenever one option is changed.
    void changed(bool);

public Q_SLOTS:

    //* load setup from config data
    void load();

    //* save current state
    void save();

    //* restore all default values
    void defaults();

    //* reset to saved configuration
    void reset();

protected Q_SLOTS:

    //* update modified state when option is checked/unchecked
    void updateChanged();

    void kPageWidgetChanged(KPageWidgetItem *current, KPageWidgetItem *before);

    void setFrameCustomCornerRadiusVisible();

    //* enable/disable _autoHideArrows checkbox depending on if scrollbar arrow button type is selected
    void setEnabledAutoHideArrows();

private:
    bool isDefaults();

    //* kconfiguration object
    KSharedConfig::Ptr _configuration;
};

}
