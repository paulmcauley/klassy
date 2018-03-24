#ifndef breezeconfigwidget_h
#define breezeconfigwidget_h
//////////////////////////////////////////////////////////////////////////////
// breezeconfigurationui.h
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "ui_breezeconfigurationui.h"
#include "breezeexceptionlistwidget.h"
#include "breezesettings.h"
#include "breeze.h"

#include <KCModule>
#include <KSharedConfig>

#include <QWidget>
#include <QSharedPointer>

namespace Breeze
{

    //_____________________________________________
    class ConfigWidget: public KCModule
    {

        Q_OBJECT

        public:

        //* constructor
        explicit ConfigWidget( QWidget*, const QVariantList& );

        //* destructor
        virtual ~ConfigWidget() = default;

        //* default
        void defaults() override;

        //* load configuration
        void load() override;

        //* save configuration
        void save() override;

        protected Q_SLOTS:

        //* update changed state
        virtual void updateChanged();

        protected:

        //* set changed state
        void setChanged( bool );

        private:

        //* ui
        Ui_BreezeConfigurationUI m_ui;

        //* kconfiguration object
        KSharedConfig::Ptr m_configuration;

        //* internal exception
        InternalSettingsPtr m_internalSettings;

        //* changed state
        bool m_changed;

    };

}

#endif
