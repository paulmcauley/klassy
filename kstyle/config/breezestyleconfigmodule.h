/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezestyleconfigmodule_h
#define breezestyleconfigmodule_h

#include "breezestyleconfig.h"

#include <KCModule>

namespace Breeze
{

//* configuration module
class ConfigurationModule : public KCModule
{
    Q_OBJECT

public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ConfigurationModule(QObject *parent, const KPluginMetaData &data);
#else
    ConfigurationModule(QWidget *parent, const QVariantList &args);
#endif

public Q_SLOTS:

    void defaults() override;
    void load() override;
    void save() override;

private:
    //* configuration
    StyleConfig *m_config;
};
}

#endif
