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
    m_internalSettings = InternalSettingsPtr(new InternalSettings());

    // track shadows changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui->shadowSizeActive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->shadowStrengthActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->shadowColorActive, &KColorButton::changed, this, &ShadowStyle::updateChanged, Qt::ConnectionType::DirectConnection);

    connect(m_ui->shadowSizeInactive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->shadowStrengthInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->shadowColorInactive, &KColorButton::changed, this, &ShadowStyle::updateChanged, Qt::ConnectionType::DirectConnection);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ShadowStyle::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ShadowStyle::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ShadowStyle::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ShadowStyle::~ShadowStyle()
{
    delete m_ui;
}

void ShadowStyle::loadMain(const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;

        // load from rc files
        m_internalSettings->load();
    }

    // load shadows
    if (m_internalSettings->shadowSize(true) <= InternalSettings::EnumShadowSize::ShadowVeryLarge) {
        m_ui->shadowSizeActive->setCurrentIndex(m_internalSettings->shadowSize(true));
    } else {
        m_ui->shadowSizeActive->setCurrentIndex(InternalSettings::EnumShadowSize::ShadowLarge);
    }

    if (m_internalSettings->shadowSize(false) <= InternalSettings::EnumShadowSize::ShadowVeryLarge) {
        m_ui->shadowSizeInactive->setCurrentIndex(m_internalSettings->shadowSize(false));
    } else {
        m_ui->shadowSizeInactive->setCurrentIndex(InternalSettings::EnumShadowSize::ShadowLarge);
    }

    m_ui->shadowStrengthActive->setValue(qRound(qreal(m_internalSettings->shadowStrength(true) * 100) / 255));
    m_ui->shadowStrengthInactive->setValue(qRound(qreal(m_internalSettings->shadowStrength(false) * 100) / 255));

    m_ui->shadowColorActive->setColor(m_internalSettings->shadowColor(true));
    m_ui->shadowColorInactive->setColor(m_internalSettings->shadowColor(false));

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void ShadowStyle::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setShadowSize(true, m_ui->shadowSizeActive->currentIndex());
    m_internalSettings->setShadowSize(false, m_ui->shadowSizeInactive->currentIndex());

    m_internalSettings->setShadowStrength(true, qRound(qreal(m_ui->shadowStrengthActive->value() * 255) / 100));
    m_internalSettings->setShadowStrength(false, qRound(qreal(m_ui->shadowStrengthInactive->value() * 255) / 100));

    m_internalSettings->setShadowColor(true, m_ui->shadowColorActive->color());
    m_internalSettings->setShadowColor(false, m_ui->shadowColorInactive->color());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        DBusMessages::updateDecorationColorCache();
        DBusMessages::kwinReloadConfig();
        // DBusMessages::kstyleReloadDecorationConfig(); //should reload anyway

        static_cast<ConfigWidget *>(m_parent)->generateSystemIcons(); // system icons could have a shadow colour override
    }
}

void ShadowStyle::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    loadMain(true);

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

    if (m_ui->shadowSizeActive->currentIndex() != m_internalSettings->shadowSize(true))
        modified = true;
    else if (m_ui->shadowSizeInactive->currentIndex() != m_internalSettings->shadowSize(false))
        modified = true;
    else if (qRound(qreal(m_ui->shadowStrengthActive->value() * 255) / 100) != m_internalSettings->shadowStrength(true))
        modified = true;
    else if (qRound(qreal(m_ui->shadowStrengthInactive->value() * 255) / 100) != m_internalSettings->shadowStrength(false))
        modified = true;
    else if (m_ui->shadowColorActive->color() != m_internalSettings->shadowColor(true))
        modified = true;
    else if (m_ui->shadowColorInactive->color() != m_internalSettings->shadowColor(false))
        modified = true;

    setChanged(modified);
}

void ShadowStyle::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

}
