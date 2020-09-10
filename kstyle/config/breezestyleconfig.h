/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezestyleconfig_h
#define breezestyleconfig_h

#include "ui_breezestyleconfig.h"

namespace Breeze
{

    class StyleConfig: public QWidget, Ui::BreezeStyleConfig
    {

        Q_OBJECT

        public:

        //* constructor
        explicit StyleConfig(QWidget*);

        //* destructor
        virtual ~StyleConfig()
        {}

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

    };

}
#endif
