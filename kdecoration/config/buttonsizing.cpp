/*
 * SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttonsizing.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "presetsmodel.h"

namespace Breeze
{

ButtonSizing::ButtonSizing(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent)
    : QDialog(parent)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui.setupUi(this);

    // track ui changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui.scaleBackgroundPercent, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonWidthMarginLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonWidthMarginRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonSpacingRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonSpacingRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.integratedRoundedRectangleBottomPadding, SIGNAL(valueChanged(double)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.lockFullHeightButtonWidthMargins, &QAbstractButton::toggled, this, &ButtonSizing::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.lockButtonSpacingLeftRight, &QAbstractButton::toggled, this, &ButtonSizing::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.lockFullHeightButtonSpacingLeftRight, &QAbstractButton::toggled, this, &ButtonSizing::updateChanged, Qt::ConnectionType::DirectConnection);

    // connect dual controls with same values
    connect(m_ui.fullHeightButtonWidthMarginLeft,
            SIGNAL(valueChanged(int)),
            SLOT(fullHeightButtonWidthMarginLeftChanged()),
            Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonWidthMarginRight,
            SIGNAL(valueChanged(int)),
            SLOT(fullHeightButtonWidthMarginRightChanged()),
            Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(buttonSpacingLeftChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonSpacingRight, SIGNAL(valueChanged(int)), SLOT(buttonSpacingRightChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(fullHeightButtonSpacingLeftChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonSpacingRight, SIGNAL(valueChanged(int)), SLOT(fullHeightButtonSpacingRightChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui.buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ButtonSizing::defaults);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ButtonSizing::load);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ButtonSizing::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ButtonSizing::~ButtonSizing()
{
}

void ButtonSizing::load()
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    m_ui.scaleBackgroundPercent->setValue(m_internalSettings->scaleBackgroundPercent());
    m_ui.fullHeightButtonWidthMarginLeft->setValue(m_internalSettings->fullHeightButtonWidthMarginLeft());
    m_ui.fullHeightButtonWidthMarginRight->setValue(m_internalSettings->fullHeightButtonWidthMarginRight());
    m_ui.buttonSpacingRight->setValue(m_internalSettings->buttonSpacingRight());
    m_ui.buttonSpacingLeft->setValue(m_internalSettings->buttonSpacingLeft());
    m_ui.fullHeightButtonSpacingRight->setValue(m_internalSettings->fullHeightButtonSpacingRight());
    m_ui.fullHeightButtonSpacingLeft->setValue(m_internalSettings->fullHeightButtonSpacingLeft());
    m_ui.integratedRoundedRectangleBottomPadding->setValue(m_internalSettings->integratedRoundedRectangleBottomPadding());
    m_ui.lockFullHeightButtonWidthMargins->setChecked(m_internalSettings->lockFullHeightButtonWidthMargins());
    m_ui.lockButtonSpacingLeftRight->setChecked(m_internalSettings->lockButtonSpacingLeftRight());
    m_ui.lockFullHeightButtonSpacingLeftRight->setChecked(m_internalSettings->lockFullHeightButtonSpacingLeftRight());

    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void ButtonSizing::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setScaleBackgroundPercent(m_ui.scaleBackgroundPercent->value());
    m_internalSettings->setFullHeightButtonWidthMarginLeft(m_ui.fullHeightButtonWidthMarginLeft->value());
    m_internalSettings->setFullHeightButtonWidthMarginRight(m_ui.fullHeightButtonWidthMarginRight->value());
    m_internalSettings->setButtonSpacingRight(m_ui.buttonSpacingRight->value());
    m_internalSettings->setButtonSpacingLeft(m_ui.buttonSpacingLeft->value());
    m_internalSettings->setFullHeightButtonSpacingRight(m_ui.fullHeightButtonSpacingRight->value());
    m_internalSettings->setFullHeightButtonSpacingLeft(m_ui.fullHeightButtonSpacingLeft->value());
    m_internalSettings->setIntegratedRoundedRectangleBottomPadding(m_ui.integratedRoundedRectangleBottomPadding->value());
    m_internalSettings->setLockFullHeightButtonWidthMargins(m_ui.lockFullHeightButtonWidthMargins->isChecked());
    m_internalSettings->setLockButtonSpacingLeftRight(m_ui.lockButtonSpacingLeftRight->isChecked());
    m_internalSettings->setLockFullHeightButtonSpacingLeftRight(m_ui.lockFullHeightButtonSpacingLeftRight->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig)
        DBusMessages::kwinReloadConfig();
}

void ButtonSizing::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui.scaleBackgroundPercent->setValue(m_internalSettings->scaleBackgroundPercent());
    m_ui.fullHeightButtonWidthMarginLeft->setValue(m_internalSettings->fullHeightButtonWidthMarginLeft());
    m_ui.fullHeightButtonWidthMarginRight->setValue(m_internalSettings->fullHeightButtonWidthMarginRight());
    m_ui.buttonSpacingRight->setValue(m_internalSettings->buttonSpacingRight());
    m_ui.buttonSpacingLeft->setValue(m_internalSettings->buttonSpacingLeft());
    m_ui.fullHeightButtonSpacingRight->setValue(m_internalSettings->fullHeightButtonSpacingRight());
    m_ui.fullHeightButtonSpacingLeft->setValue(m_internalSettings->fullHeightButtonSpacingLeft());
    m_ui.integratedRoundedRectangleBottomPadding->setValue(m_internalSettings->integratedRoundedRectangleBottomPadding());
    m_ui.lockFullHeightButtonWidthMargins->setChecked(m_internalSettings->lockFullHeightButtonWidthMargins());
    m_ui.lockButtonSpacingLeftRight->setChecked(m_internalSettings->lockButtonSpacingLeftRight());
    m_ui.lockFullHeightButtonSpacingLeftRight->setChecked(m_internalSettings->lockFullHeightButtonSpacingLeftRight());

    setChanged(!isDefaults());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

bool ButtonSizing::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("ButtonSizing"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void ButtonSizing::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // Q_EMIT changed(value);
}

void ButtonSizing::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui.scaleBackgroundPercent->value() != m_internalSettings->scaleBackgroundPercent())
        modified = true;
    else if (m_ui.fullHeightButtonWidthMarginLeft->value() != m_internalSettings->fullHeightButtonWidthMarginLeft())
        modified = true;
    else if (m_ui.fullHeightButtonWidthMarginRight->value() != m_internalSettings->fullHeightButtonWidthMarginRight())
        modified = true;
    else if (m_ui.buttonSpacingRight->value() != m_internalSettings->buttonSpacingRight())
        modified = true;
    else if (m_ui.buttonSpacingLeft->value() != m_internalSettings->buttonSpacingLeft())
        modified = true;
    else if (m_ui.fullHeightButtonSpacingRight->value() != m_internalSettings->fullHeightButtonSpacingRight())
        modified = true;
    else if (m_ui.fullHeightButtonSpacingLeft->value() != m_internalSettings->fullHeightButtonSpacingLeft())
        modified = true;
    else if (m_ui.integratedRoundedRectangleBottomPadding->value() != m_internalSettings->integratedRoundedRectangleBottomPadding())
        modified = true;
    else if (m_ui.lockFullHeightButtonWidthMargins->isChecked() != m_internalSettings->lockFullHeightButtonWidthMargins())
        modified = true;
    else if (m_ui.lockButtonSpacingLeftRight->isChecked() != m_internalSettings->lockButtonSpacingLeftRight())
        modified = true;
    else if (m_ui.lockFullHeightButtonSpacingLeftRight->isChecked() != m_internalSettings->lockFullHeightButtonSpacingLeftRight())
        modified = true;

    setChanged(modified);
}

void ButtonSizing::accept()
{
    save();
    QDialog::accept();
}

void ButtonSizing::reject()
{
    load();
    QDialog::reject();
}

void ButtonSizing::setApplyButtonState(const bool on)
{
    m_ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void ButtonSizing::fullHeightButtonWidthMarginLeftChanged()
{
    if (m_ui.lockFullHeightButtonWidthMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonWidthMarginRight->setValue(m_ui.fullHeightButtonWidthMarginLeft->value());
}

void ButtonSizing::fullHeightButtonWidthMarginRightChanged()
{
    if (m_ui.lockFullHeightButtonWidthMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonWidthMarginLeft->setValue(m_ui.fullHeightButtonWidthMarginRight->value());
}

void ButtonSizing::buttonSpacingLeftChanged()
{
    if (m_ui.lockButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.buttonSpacingRight->setValue(m_ui.buttonSpacingLeft->value());
}

void ButtonSizing::buttonSpacingRightChanged()
{
    if (m_ui.lockButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.buttonSpacingLeft->setValue(m_ui.buttonSpacingRight->value());
}

void ButtonSizing::fullHeightButtonSpacingLeftChanged()
{
    if (m_ui.lockFullHeightButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonSpacingRight->setValue(m_ui.fullHeightButtonSpacingLeft->value());
}

void ButtonSizing::fullHeightButtonSpacingRightChanged()
{
    if (m_ui.lockFullHeightButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonSpacingLeft->setValue(m_ui.fullHeightButtonSpacingRight->value());
}

void ButtonSizing::updateLockIcons()
{
    m_ui.lockFullHeightButtonWidthMargins->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui.lockButtonSpacingLeftRight->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui.lockFullHeightButtonSpacingLeftRight->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
}
}
