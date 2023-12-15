/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "shadowstyle.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPushButton>

namespace Breeze
{

ShadowStyle::ShadowStyle(KSharedConfig::Ptr config, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_ShadowStyle)
    , m_configuration(config)
{
    m_ui->setupUi(this);

    // track shadows changes
    connect(m_ui->shadowSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->shadowStrength, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->shadowColor, &KColorButton::changed, this, &ShadowStyle::updateChanged);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ShadowStyle::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ShadowStyle::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ShadowStyle::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ShadowStyle::~ShadowStyle()
{
    delete m_ui;
}

void ShadowStyle::loadMain(const QString loadPreset)
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    if (loadPreset.isEmpty()) { // normal cases
        m_internalSettings->load();
    } else { // loading preset
        PresetsModel::loadPreset(m_internalSettings.data(), m_configuration.data(), loadPreset);
    }

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
        ConfigWidget::kwinReloadConfig();
        ConfigWidget::kstyleReloadConfig();
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

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

void ShadowStyle::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // Q_EMIT changed(value);
}

void ShadowStyle::accept()
{
    save();
    QDialog::accept();
}

void ShadowStyle::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

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
