/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "shadowstyle.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "presetsmodel.h"
#include <QPushButton>

namespace Breeze
{

ShadowStyle::ShadowStyle(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_ui(new Ui_ShadowStyle)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    // track shadows changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui->shadowSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->shadowStrength, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->shadowColor, &KColorButton::changed, this, &ShadowStyle::updateChanged, Qt::ConnectionType::DirectConnection);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ShadowStyle::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ShadowStyle::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ShadowStyle::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ShadowStyle::~ShadowStyle()
{
    delete m_ui;
}

void ShadowStyle::load()
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // load shadows
    if (m_internalSettings->shadowSize() <= InternalSettings::EnumShadowSize::ShadowVeryLarge) {
        m_ui->shadowSize->setCurrentIndex(m_internalSettings->shadowSize());
    } else {
        m_ui->shadowSize->setCurrentIndex(InternalSettings::EnumShadowSize::ShadowLarge);
    }

    m_ui->shadowStrength->setValue(qRound(qreal(m_internalSettings->shadowStrength() * 100) / 255));
    m_ui->shadowColor->setColor(m_internalSettings->shadowColor());

    // TODO:add all lock variants
    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void ShadowStyle::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setShadowSize(m_ui->shadowSize->currentIndex());
    m_internalSettings->setShadowStrength(qRound(qreal(m_ui->shadowStrength->value() * 255) / 100));
    m_internalSettings->setShadowColor(m_ui->shadowColor->color());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        DBusMessages::updateDecorationColorCache();
        DBusMessages::kwinReloadConfig();
        // DBusMessages::kstyleReloadDecorationConfig(); //should reload anyway

        static_cast<ConfigWidget *>(m_parent)->generateSystemIcons();
    }
}

void ShadowStyle::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui->shadowSize->setCurrentIndex(m_internalSettings->shadowSize());
    m_ui->shadowStrength->setValue(qRound(qreal(m_internalSettings->shadowStrength() * 100) / 255));
    m_ui->shadowColor->setColor(m_internalSettings->shadowColor());

    setChanged(!isDefaults());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

bool ShadowStyle::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("ShadowStyle"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void ShadowStyle::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    Q_EMIT changed(value);
}

void ShadowStyle::accept()
{
    save();
    QDialog::accept();
}

void ShadowStyle::reject()
{
    load();
    QDialog::reject();
}

void ShadowStyle::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui->shadowSize->currentIndex() != m_internalSettings->shadowSize())
        modified = true;
    else if (qRound(qreal(m_ui->shadowStrength->value() * 255) / 100) != m_internalSettings->shadowStrength())
        modified = true;
    else if (m_ui->shadowColor->color() != m_internalSettings->shadowColor())
        modified = true;

    setChanged(modified);
}

void ShadowStyle::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

}
