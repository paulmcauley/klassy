//////////////////////////////////////////////////////////////////////////////
// fullheightrectanglesizing.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "fullheightrectanglesizing.h"
#include <QDBusConnection>
#include <QDBusMessage>

namespace Breeze
{

FullHeightRectangleSizing::FullHeightRectangleSizing(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    // track ui changes
    connect(m_ui.fullHeightButtonWidthMarginLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.fullHeightButtonWidthMarginRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.fullHeightIntegratedRoundedRectangleBottomPadding, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    connect(m_ui.lockFullHeightButtonWidthMargins, &QAbstractButton::toggled, this, &FullHeightRectangleSizing::updateChanged);

    // connect dual controls with same values
    connect(m_ui.fullHeightButtonWidthMarginLeft, SIGNAL(valueChanged(int)), SLOT(fullHeightButtonWidthMarginLeftChanged()));
    connect(m_ui.fullHeightButtonWidthMarginRight, SIGNAL(valueChanged(int)), SLOT(fullHeightButtonWidthMarginRightChanged()));

    connect(m_ui.buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &FullHeightRectangleSizing::defaults);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &FullHeightRectangleSizing::load);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &FullHeightRectangleSizing::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

FullHeightRectangleSizing::~FullHeightRectangleSizing()
{
}

void FullHeightRectangleSizing::load()
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    m_ui.fullHeightButtonWidthMarginLeft->setValue(m_internalSettings->fullHeightButtonWidthMarginLeft());
    m_ui.fullHeightButtonWidthMarginRight->setValue(m_internalSettings->fullHeightButtonWidthMarginRight());
    m_ui.fullHeightIntegratedRoundedRectangleBottomPadding->setValue(m_internalSettings->fullHeightIntegratedRoundedRectangleBottomPadding());
    m_ui.lockFullHeightButtonWidthMargins->setChecked(m_internalSettings->lockFullHeightButtonWidthMargins());

    setChanged(false);

    m_loading = false;
}

void FullHeightRectangleSizing::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setFullHeightButtonWidthMarginLeft(m_ui.fullHeightButtonWidthMarginLeft->value());
    m_internalSettings->setFullHeightButtonWidthMarginRight(m_ui.fullHeightButtonWidthMarginRight->value());
    m_internalSettings->setFullHeightIntegratedRoundedRectangleBottomPadding(m_ui.fullHeightIntegratedRoundedRectangleBottomPadding->value());
    m_internalSettings->setLockFullHeightButtonWidthMargins(m_ui.lockFullHeightButtonWidthMargins->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig)
    // needed to tell kwin to reload when running from external kcmshell
    {
        QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
    }
}

void FullHeightRectangleSizing::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui.fullHeightButtonWidthMarginLeft->setValue(m_internalSettings->fullHeightButtonWidthMarginLeft());
    m_ui.fullHeightButtonWidthMarginRight->setValue(m_internalSettings->fullHeightButtonWidthMarginRight());
    m_ui.fullHeightIntegratedRoundedRectangleBottomPadding->setValue(m_internalSettings->fullHeightIntegratedRoundedRectangleBottomPadding());
    m_ui.lockFullHeightButtonWidthMargins->setChecked(m_internalSettings->lockFullHeightButtonWidthMargins());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

void FullHeightRectangleSizing::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // emit changed(value);
}

void FullHeightRectangleSizing::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    // track modifications
    bool modified(false);

    if (m_ui.fullHeightButtonWidthMarginLeft->value() != m_internalSettings->fullHeightButtonWidthMarginLeft())
        modified = true;
    else if (m_ui.fullHeightButtonWidthMarginRight->value() != m_internalSettings->fullHeightButtonWidthMarginRight())
        modified = true;
    else if (m_ui.fullHeightIntegratedRoundedRectangleBottomPadding->value() != m_internalSettings->fullHeightIntegratedRoundedRectangleBottomPadding())
        modified = true;
    else if (m_ui.lockFullHeightButtonWidthMargins->isChecked() != m_internalSettings->lockFullHeightButtonWidthMargins())
        modified = true;

    setChanged(modified);
}

void FullHeightRectangleSizing::accept()
{
    save();
    QDialog::accept();
}

void FullHeightRectangleSizing::setApplyButtonState(const bool on)
{
    m_ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void FullHeightRectangleSizing::fullHeightButtonWidthMarginLeftChanged()
{
    if (m_ui.lockFullHeightButtonWidthMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonWidthMarginRight->setValue(m_ui.fullHeightButtonWidthMarginLeft->value());
}

void FullHeightRectangleSizing::fullHeightButtonWidthMarginRightChanged()
{
    if (m_ui.lockFullHeightButtonWidthMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonWidthMarginLeft->setValue(m_ui.fullHeightButtonWidthMarginRight->value());
}

}
