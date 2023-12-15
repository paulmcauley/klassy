/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttoncolors.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <KColorButton>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPushButton>

namespace Breeze
{

ButtonColors::ButtonColors(KSharedConfig::Ptr config, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_ButtonColors)
    , m_configuration(config)
{
    m_ui->setupUi(this);

    // track ui changes
    connect(m_ui->backgroundColors, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->translucentButtonBackgrounds, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->redAlwaysShownClose, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->lockButtonBackgroundColorsActive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ButtonColors::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ButtonColors::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ButtonColors::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ButtonColors::~ButtonColors()
{
    delete m_ui;
}

void ButtonColors::loadMain(const QString loadPreset)
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    if (loadPreset.isEmpty()) { // normal cases
        m_internalSettings->load();
    } else { // loading preset
        PresetsModel::loadPreset(m_internalSettings.data(), m_configuration.data(), loadPreset);
    }

    m_ui->backgroundColors->setCurrentIndex(m_internalSettings->backgroundColors());
    m_ui->translucentButtonBackgrounds->setChecked(m_internalSettings->translucentButtonBackgrounds());
    m_ui->redAlwaysShownClose->setChecked(m_internalSettings->redAlwaysShownClose());
    m_ui->lockButtonBackgroundColorsActive->setChecked(m_internalSettings->lockButtonBackgroundColorsActive());

    // TODO:add all lock variants
    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void ButtonColors::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setBackgroundColors(m_ui->backgroundColors->currentIndex());
    m_internalSettings->setTranslucentButtonBackgrounds(m_ui->translucentButtonBackgrounds->isChecked());
    m_internalSettings->setRedAlwaysShownClose(m_ui->redAlwaysShownClose->isChecked());
    m_internalSettings->setLockButtonBackgroundColorsActive(m_ui->lockButtonBackgroundColorsActive->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        ConfigWidget::kwinReloadConfig();
        ConfigWidget::kstyleReloadConfig();
    }
}

void ButtonColors::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui->backgroundColors->setCurrentIndex(m_internalSettings->backgroundColors());
    m_ui->translucentButtonBackgrounds->setChecked(m_internalSettings->translucentButtonBackgrounds());
    m_ui->redAlwaysShownClose->setChecked(m_internalSettings->redAlwaysShownClose());
    m_ui->lockButtonBackgroundColorsActive->setChecked(m_internalSettings->lockButtonBackgroundColorsActive());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

void ButtonColors::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // Q_EMIT changed(value);
}

void ButtonColors::accept()
{
    save();
    QDialog::accept();
}

void ButtonColors::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    // track modifications
    bool modified(false);
    if (m_ui->backgroundColors->currentIndex() != m_internalSettings->backgroundColors())
        modified = true;
    else if (m_ui->translucentButtonBackgrounds->isChecked() != m_internalSettings->translucentButtonBackgrounds())
        modified = true;
    else if (m_ui->redAlwaysShownClose->isChecked() != m_internalSettings->redAlwaysShownClose())
        modified = true;
    else if (m_ui->lockButtonBackgroundColorsActive->isChecked() != m_internalSettings->lockButtonBackgroundColorsActive()) modified = true;

    setChanged(modified);
}

void ButtonColors::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

}
