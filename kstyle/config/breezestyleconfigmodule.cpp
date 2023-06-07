/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezestyleconfigmodule.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(Breeze::ConfigurationModule, "klassystyleconfig.json")

#include "breezestyleconfigmodule.moc"

namespace Breeze
{

//_______________________________________________________________________
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
ConfigurationModule::ConfigurationModule(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
{
    widget()->setLayout(new QVBoxLayout);
    widget()->layout()->addWidget(m_config = new StyleConfig(widget()));
    setButtons(KCModule::Default | KCModule::Apply);
    connect(m_config, &StyleConfig::changed, this, &KCModule::setNeedsSave);
}
#else
ConfigurationModule::ConfigurationModule(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    setLayout(new QVBoxLayout(this));
    setButtons(KCModule::Default | KCModule::Apply);
    layout()->addWidget(m_config = new StyleConfig(this));
    connect(m_config, static_cast<void (StyleConfig::*)(bool)>(&StyleConfig::changed), this, static_cast<void (KCModule::*)(bool)>(&KCModule::changed));
}
#endif

//_______________________________________________________________________
void ConfigurationModule::defaults()
{
    m_config->defaults();
    KCModule::defaults();
}

//_______________________________________________________________________
void ConfigurationModule::load()
{
    m_config->load();
    KCModule::load();
}

//_______________________________________________________________________
void ConfigurationModule::save()
{
    m_config->save();
    KCModule::save();
}
}
