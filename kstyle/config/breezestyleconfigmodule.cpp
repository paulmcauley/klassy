/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezestyleconfigmodule.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY(BreezeStyleConfigFactory, registerPlugin<Breeze::ConfigurationModule>();)

#include "breezestyleconfigmodule.moc"

namespace Breeze
{

//_______________________________________________________________________
ConfigurationModule::ConfigurationModule(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    setLayout(new QVBoxLayout(this));
    setButtons(KCModule::Default | KCModule::Apply);
    layout()->addWidget(m_config = new StyleConfig(this));
    connect(m_config, static_cast<void (StyleConfig::*)(bool)>(&StyleConfig::changed), this, static_cast<void (KCModule::*)(bool)>(&KCModule::changed));
}

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
