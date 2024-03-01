/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "titlebarspacing.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "presetsmodel.h"
#include <QPushButton>

namespace Breeze
{

TitleBarSpacing::TitleBarSpacing(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_ui(new Ui_TitleBarSpacing)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    // track ui changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui->titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleSidePadding, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleBarTopMargin, SIGNAL(valueChanged(double)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleBarBottomMargin, SIGNAL(valueChanged(double)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->percentMaximizedTopBottomMargins, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleBarLeftMargin, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleBarRightMargin, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->lockTitleBarTopBottomMargins, &QAbstractButton::toggled, this, &TitleBarSpacing::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->lockTitleBarLeftRightMargins, &QAbstractButton::toggled, this, &TitleBarSpacing::updateChanged, Qt::ConnectionType::DirectConnection);

    // connect dual controls with same values
    connect(m_ui->titleBarTopMargin, SIGNAL(valueChanged(double)), SLOT(titleBarTopMarginChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleBarBottomMargin, SIGNAL(valueChanged(double)), SLOT(titleBarBottomMarginChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleBarLeftMargin, SIGNAL(valueChanged(int)), SLOT(titleBarLeftMarginChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleBarRightMargin, SIGNAL(valueChanged(int)), SLOT(titleBarRightMarginChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &TitleBarSpacing::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &TitleBarSpacing::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &TitleBarSpacing::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

TitleBarSpacing::~TitleBarSpacing()
{
    delete m_ui;
}

void TitleBarSpacing::load()
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    m_ui->titleAlignment->setCurrentIndex(m_internalSettings->titleAlignment());
    m_ui->titleSidePadding->setValue(m_internalSettings->titleSidePadding());
    m_ui->titleBarTopMargin->setValue(m_internalSettings->titleBarTopMargin());
    m_ui->titleBarBottomMargin->setValue(m_internalSettings->titleBarBottomMargin());
    m_ui->percentMaximizedTopBottomMargins->setValue(m_internalSettings->percentMaximizedTopBottomMargins());
    m_ui->titleBarLeftMargin->setValue(m_internalSettings->titleBarLeftMargin());
    m_ui->titleBarRightMargin->setValue(m_internalSettings->titleBarRightMargin());
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
    m_internalSettings->setTitleBarTopMargin(m_ui->titleBarTopMargin->value());
    m_internalSettings->setTitleBarBottomMargin(m_ui->titleBarBottomMargin->value());
    m_internalSettings->setPercentMaximizedTopBottomMargins(m_ui->percentMaximizedTopBottomMargins->value());
    m_internalSettings->setTitleBarLeftMargin(m_ui->titleBarLeftMargin->value());
    m_internalSettings->setTitleBarRightMargin(m_ui->titleBarRightMargin->value());
    m_internalSettings->setLockTitleBarTopBottomMargins(m_ui->lockTitleBarTopBottomMargins->isChecked());
    m_internalSettings->setLockTitleBarLeftRightMargins(m_ui->lockTitleBarLeftRightMargins->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig)
        DBusMessages::kwinReloadConfig();
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
    m_ui->titleBarTopMargin->setValue(m_internalSettings->titleBarTopMargin());
    m_ui->titleBarBottomMargin->setValue(m_internalSettings->titleBarBottomMargin());
    m_ui->percentMaximizedTopBottomMargins->setValue(m_internalSettings->percentMaximizedTopBottomMargins());
    m_ui->titleBarLeftMargin->setValue(m_internalSettings->titleBarLeftMargin());
    m_ui->titleBarRightMargin->setValue(m_internalSettings->titleBarRightMargin());

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
    Q_EMIT changed(value);
}

void TitleBarSpacing::accept()
{
    save();
    QDialog::accept();
}

void TitleBarSpacing::reject()
{
    load();
    QDialog::reject();
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
    else if (m_ui->titleBarTopMargin->value() != m_internalSettings->titleBarTopMargin())
        modified = true;
    else if (m_ui->titleBarBottomMargin->value() != m_internalSettings->titleBarBottomMargin())
        modified = true;
    else if (m_ui->percentMaximizedTopBottomMargins->value() != m_internalSettings->percentMaximizedTopBottomMargins())
        modified = true;
    else if (m_ui->titleBarLeftMargin->value() != m_internalSettings->titleBarLeftMargin())
        modified = true;
    else if (m_ui->titleBarRightMargin->value() != m_internalSettings->titleBarRightMargin())
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

void TitleBarSpacing::titleBarTopMarginChanged()
{
    if (m_ui->lockTitleBarTopBottomMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->titleBarBottomMargin->setValue(m_ui->titleBarTopMargin->value());
}

void TitleBarSpacing::titleBarBottomMarginChanged()
{
    if (m_ui->lockTitleBarTopBottomMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->titleBarTopMargin->setValue(m_ui->titleBarBottomMargin->value());
}

void TitleBarSpacing::titleBarLeftMarginChanged()
{
    if (m_ui->lockTitleBarLeftRightMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->titleBarRightMargin->setValue(m_ui->titleBarLeftMargin->value());
}

void TitleBarSpacing::titleBarRightMarginChanged()
{
    if (m_ui->lockTitleBarLeftRightMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->titleBarLeftMargin->setValue(m_ui->titleBarRightMargin->value());
}

void TitleBarSpacing::updateLockIcons()
{
    m_ui->lockTitleBarLeftRightMargins->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockTitleBarLeftRightMargins_2->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockTitleBarTopBottomMargins->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockTitleBarTopBottomMargins_2->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
}
}
