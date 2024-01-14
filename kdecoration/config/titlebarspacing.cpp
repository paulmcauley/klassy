/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "titlebarspacing.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPushButton>

namespace Breeze
{

TitleBarSpacing::TitleBarSpacing(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_TitleBarSpacing)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
{
    m_ui->setupUi(this);

    // track ui changes
    connect(m_ui->titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->titleSidePadding, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->titlebarTopMargin, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    connect(m_ui->titlebarBottomMargin, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    connect(m_ui->percentMaximizedTopBottomMargins, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->titlebarLeftMargin, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->titlebarRightMargin, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->lockTitleBarTopBottomMargins, &QAbstractButton::toggled, this, &TitleBarSpacing::updateChanged);
    connect(m_ui->lockTitleBarLeftRightMargins, &QAbstractButton::toggled, this, &TitleBarSpacing::updateChanged);

    // connect dual controls with same values
    connect(m_ui->titlebarTopMargin, SIGNAL(valueChanged(double)), SLOT(titlebarTopMarginChanged()));
    connect(m_ui->titlebarBottomMargin, SIGNAL(valueChanged(double)), SLOT(titlebarBottomMarginChanged()));
    connect(m_ui->titlebarLeftMargin, SIGNAL(valueChanged(int)), SLOT(titlebarLeftMarginChanged()));
    connect(m_ui->titlebarRightMargin, SIGNAL(valueChanged(int)), SLOT(titlebarRightMarginChanged()));

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &TitleBarSpacing::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &TitleBarSpacing::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &TitleBarSpacing::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

TitleBarSpacing::~TitleBarSpacing()
{
    delete m_ui;
}

void TitleBarSpacing::loadMain(const QString loadPreset)
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    if (loadPreset.isEmpty()) { // normal cases
        m_internalSettings->load();
    } else { // loading preset
        PresetsModel::loadPreset(m_internalSettings.data(), m_presetsConfiguration.data(), loadPreset);
    }

    m_ui->titleAlignment->setCurrentIndex(m_internalSettings->titleAlignment());
    m_ui->titleSidePadding->setValue(m_internalSettings->titleSidePadding());
    m_ui->titlebarTopMargin->setValue(m_internalSettings->titlebarTopMargin());
    m_ui->titlebarBottomMargin->setValue(m_internalSettings->titlebarBottomMargin());
    m_ui->percentMaximizedTopBottomMargins->setValue(m_internalSettings->percentMaximizedTopBottomMargins());
    m_ui->titlebarLeftMargin->setValue(m_internalSettings->titlebarLeftMargin());
    m_ui->titlebarRightMargin->setValue(m_internalSettings->titlebarRightMargin());
    m_ui->lockTitleBarTopBottomMargins->setChecked(m_internalSettings->lockTitleBarTopBottomMargins());
    m_ui->lockTitleBarLeftRightMargins->setChecked(m_internalSettings->lockTitleBarLeftRightMargins());

    // TODO:add all lock variants
    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void TitleBarSpacing::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setTitleAlignment(m_ui->titleAlignment->currentIndex());
    m_internalSettings->setTitleSidePadding(m_ui->titleSidePadding->value());
    m_internalSettings->setTitlebarTopMargin(m_ui->titlebarTopMargin->value());
    m_internalSettings->setTitlebarBottomMargin(m_ui->titlebarBottomMargin->value());
    m_internalSettings->setPercentMaximizedTopBottomMargins(m_ui->percentMaximizedTopBottomMargins->value());
    m_internalSettings->setTitlebarLeftMargin(m_ui->titlebarLeftMargin->value());
    m_internalSettings->setTitlebarRightMargin(m_ui->titlebarRightMargin->value());
    m_internalSettings->setLockTitleBarTopBottomMargins(m_ui->lockTitleBarTopBottomMargins->isChecked());
    m_internalSettings->setLockTitleBarLeftRightMargins(m_ui->lockTitleBarLeftRightMargins->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig)
        ConfigWidget::kwinReloadConfig();
}

void TitleBarSpacing::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui->titleAlignment->setCurrentIndex(m_internalSettings->titleAlignment());
    m_ui->titleSidePadding->setValue(m_internalSettings->titleSidePadding());
    m_ui->titlebarTopMargin->setValue(m_internalSettings->titlebarTopMargin());
    m_ui->titlebarBottomMargin->setValue(m_internalSettings->titlebarBottomMargin());
    m_ui->percentMaximizedTopBottomMargins->setValue(m_internalSettings->percentMaximizedTopBottomMargins());
    m_ui->titlebarLeftMargin->setValue(m_internalSettings->titlebarLeftMargin());
    m_ui->titlebarRightMargin->setValue(m_internalSettings->titlebarRightMargin());

    setChanged(!isDefaults());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

bool TitleBarSpacing::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("TitleBarSpacing"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void TitleBarSpacing::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // Q_EMIT changed(value);
}

void TitleBarSpacing::accept()
{
    save();
    QDialog::accept();
}

void TitleBarSpacing::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui->titleAlignment->currentIndex() != m_internalSettings->titleAlignment())
        modified = true;
    else if (m_ui->titleSidePadding->value() != m_internalSettings->titleSidePadding())
        modified = true;
    else if (m_ui->titlebarTopMargin->value() != m_internalSettings->titlebarTopMargin())
        modified = true;
    else if (m_ui->titlebarBottomMargin->value() != m_internalSettings->titlebarBottomMargin())
        modified = true;
    else if (m_ui->percentMaximizedTopBottomMargins->value() != m_internalSettings->percentMaximizedTopBottomMargins())
        modified = true;
    else if (m_ui->titlebarLeftMargin->value() != m_internalSettings->titlebarLeftMargin())
        modified = true;
    else if (m_ui->titlebarRightMargin->value() != m_internalSettings->titlebarRightMargin())
        modified = true;
    else if (m_ui->lockTitleBarTopBottomMargins->isChecked() != m_internalSettings->lockTitleBarTopBottomMargins())
        modified = true;
    else if (m_ui->lockTitleBarLeftRightMargins->isChecked() != m_internalSettings->lockTitleBarLeftRightMargins())
        modified = true;

    setChanged(modified);
}

void TitleBarSpacing::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void TitleBarSpacing::titlebarTopMarginChanged()
{
    if (m_ui->lockTitleBarTopBottomMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->titlebarBottomMargin->setValue(m_ui->titlebarTopMargin->value());
}

void TitleBarSpacing::titlebarBottomMarginChanged()
{
    if (m_ui->lockTitleBarTopBottomMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->titlebarTopMargin->setValue(m_ui->titlebarBottomMargin->value());
}

void TitleBarSpacing::titlebarLeftMarginChanged()
{
    if (m_ui->lockTitleBarLeftRightMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->titlebarRightMargin->setValue(m_ui->titlebarLeftMargin->value());
}

void TitleBarSpacing::titlebarRightMarginChanged()
{
    if (m_ui->lockTitleBarLeftRightMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->titlebarLeftMargin->setValue(m_ui->titlebarRightMargin->value());
}

}
