/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "windowoutlineopacity.h"
#include "presetsmodel.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPushButton>

namespace Breeze
{

WindowOutlineOpacity::WindowOutlineOpacity(KSharedConfig::Ptr config, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_WindowOutlineOpacity)
    , m_configuration(config)
{
    m_ui->setupUi(this);

    // track ui changes
    connect(m_ui->windowOutlineShadowColorOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineContrastOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineContrastOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineAccentColorOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineAccentColorOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineAccentWithContrastOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineAccentWithContrastOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineCustomColorOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineCustomColorOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineCustomWithContrastOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->windowOutlineCustomWithContrastOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &WindowOutlineOpacity::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &WindowOutlineOpacity::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &WindowOutlineOpacity::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

WindowOutlineOpacity::~WindowOutlineOpacity()
{
    delete m_ui;
}

void WindowOutlineOpacity::loadMain(const QString loadPreset)
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    if (loadPreset.isEmpty()) { // normal cases
        m_internalSettings->load();
    } else { // loading preset
        PresetsModel::readPreset(m_internalSettings.data(), m_configuration.data(), loadPreset);
    }

    m_ui->windowOutlineShadowColorOpacity->setValue(m_internalSettings->windowOutlineShadowColorOpacity() * 100);
    m_ui->windowOutlineShadowColorOpacity_2->setValue(m_ui->windowOutlineShadowColorOpacity->value());
    m_ui->windowOutlineContrastOpacityActive->setValue(m_internalSettings->windowOutlineContrastOpacityActive() * 100);
    m_ui->windowOutlineContrastOpacityActive_2->setValue(m_ui->windowOutlineContrastOpacityActive->value());
    m_ui->windowOutlineContrastOpacityInactive->setValue(m_internalSettings->windowOutlineContrastOpacityInactive() * 100);
    m_ui->windowOutlineContrastOpacityInactive_2->setValue(m_ui->windowOutlineContrastOpacityInactive->value());
    m_ui->windowOutlineAccentColorOpacityActive->setValue(m_internalSettings->windowOutlineAccentColorOpacityActive() * 100);
    m_ui->windowOutlineAccentColorOpacityActive_2->setValue(m_ui->windowOutlineAccentColorOpacityActive->value());
    m_ui->windowOutlineAccentColorOpacityInactive->setValue(m_internalSettings->windowOutlineAccentColorOpacityInactive() * 100);
    m_ui->windowOutlineAccentColorOpacityInactive_2->setValue(m_ui->windowOutlineAccentColorOpacityInactive->value());
    m_ui->windowOutlineAccentWithContrastOpacityActive->setValue(m_internalSettings->windowOutlineAccentWithContrastOpacityActive() * 100);
    m_ui->windowOutlineAccentWithContrastOpacityActive_2->setValue(m_ui->windowOutlineAccentWithContrastOpacityActive->value());
    m_ui->windowOutlineAccentWithContrastOpacityInactive->setValue(m_internalSettings->windowOutlineAccentWithContrastOpacityInactive() * 100);
    m_ui->windowOutlineAccentWithContrastOpacityInactive_2->setValue(m_ui->windowOutlineAccentWithContrastOpacityInactive->value());
    m_ui->windowOutlineCustomColorOpacityActive->setValue(m_internalSettings->windowOutlineCustomColorOpacityActive() * 100);
    m_ui->windowOutlineCustomColorOpacityActive_2->setValue(m_ui->windowOutlineCustomColorOpacityActive->value());
    m_ui->windowOutlineCustomColorOpacityInactive->setValue(m_internalSettings->windowOutlineCustomColorOpacityInactive() * 100);
    m_ui->windowOutlineCustomColorOpacityInactive_2->setValue(m_ui->windowOutlineCustomColorOpacityInactive->value());
    m_ui->windowOutlineCustomWithContrastOpacityActive->setValue(m_internalSettings->windowOutlineCustomWithContrastOpacityActive() * 100);
    m_ui->windowOutlineCustomWithContrastOpacityActive_2->setValue(m_ui->windowOutlineCustomWithContrastOpacityActive->value());
    m_ui->windowOutlineCustomWithContrastOpacityInactive->setValue(m_internalSettings->windowOutlineCustomWithContrastOpacityInactive() * 100);
    m_ui->windowOutlineCustomWithContrastOpacityInactive_2->setValue(m_ui->windowOutlineCustomWithContrastOpacityInactive->value());

    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void WindowOutlineOpacity::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setWindowOutlineShadowColorOpacity(qreal(m_ui->windowOutlineShadowColorOpacity->value()) / 100);
    m_internalSettings->setWindowOutlineContrastOpacityActive(qreal(m_ui->windowOutlineContrastOpacityActive->value()) / 100);
    m_internalSettings->setWindowOutlineContrastOpacityInactive(qreal(m_ui->windowOutlineContrastOpacityInactive->value()) / 100);
    m_internalSettings->setWindowOutlineAccentColorOpacityActive(qreal(m_ui->windowOutlineAccentColorOpacityActive->value()) / 100);
    m_internalSettings->setWindowOutlineAccentColorOpacityInactive(qreal(m_ui->windowOutlineAccentColorOpacityInactive->value()) / 100);
    m_internalSettings->setWindowOutlineAccentWithContrastOpacityActive(qreal(m_ui->windowOutlineAccentWithContrastOpacityActive->value()) / 100);
    m_internalSettings->setWindowOutlineAccentWithContrastOpacityInactive(qreal(m_ui->windowOutlineAccentWithContrastOpacityInactive->value()) / 100);
    m_internalSettings->setWindowOutlineCustomColorOpacityActive(qreal(m_ui->windowOutlineCustomColorOpacityActive->value()) / 100);
    m_internalSettings->setWindowOutlineCustomColorOpacityInactive(qreal(m_ui->windowOutlineCustomColorOpacityInactive->value()) / 100);
    m_internalSettings->setWindowOutlineCustomWithContrastOpacityActive(qreal(m_ui->windowOutlineCustomWithContrastOpacityActive->value()) / 100);
    m_internalSettings->setWindowOutlineCustomWithContrastOpacityInactive(qreal(m_ui->windowOutlineCustomWithContrastOpacityInactive->value()) / 100);

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig)
    // needed to tell kwin to reload when running from external kcmshell
    {
        QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
    }
}

void WindowOutlineOpacity::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui->windowOutlineShadowColorOpacity->setValue(m_internalSettings->windowOutlineShadowColorOpacity() * 100);
    m_ui->windowOutlineShadowColorOpacity_2->setValue(m_ui->windowOutlineShadowColorOpacity->value());
    m_ui->windowOutlineContrastOpacityActive->setValue(m_internalSettings->windowOutlineContrastOpacityActive() * 100);
    m_ui->windowOutlineContrastOpacityActive_2->setValue(m_ui->windowOutlineContrastOpacityActive->value());
    m_ui->windowOutlineContrastOpacityInactive->setValue(m_internalSettings->windowOutlineContrastOpacityInactive() * 100);
    m_ui->windowOutlineContrastOpacityInactive_2->setValue(m_ui->windowOutlineContrastOpacityInactive->value());
    m_ui->windowOutlineAccentColorOpacityActive->setValue(m_internalSettings->windowOutlineAccentColorOpacityActive() * 100);
    m_ui->windowOutlineAccentColorOpacityActive_2->setValue(m_ui->windowOutlineAccentColorOpacityActive->value());
    m_ui->windowOutlineAccentColorOpacityInactive->setValue(m_internalSettings->windowOutlineAccentColorOpacityInactive() * 100);
    m_ui->windowOutlineAccentColorOpacityInactive_2->setValue(m_ui->windowOutlineAccentColorOpacityInactive->value());
    m_ui->windowOutlineAccentWithContrastOpacityActive->setValue(m_internalSettings->windowOutlineAccentWithContrastOpacityActive() * 100);
    m_ui->windowOutlineAccentWithContrastOpacityActive_2->setValue(m_ui->windowOutlineAccentWithContrastOpacityActive->value());
    m_ui->windowOutlineAccentWithContrastOpacityInactive->setValue(m_internalSettings->windowOutlineAccentWithContrastOpacityInactive() * 100);
    m_ui->windowOutlineAccentWithContrastOpacityInactive_2->setValue(m_ui->windowOutlineAccentWithContrastOpacityInactive->value());
    m_ui->windowOutlineCustomColorOpacityActive->setValue(m_internalSettings->windowOutlineCustomColorOpacityActive() * 100);
    m_ui->windowOutlineCustomColorOpacityActive_2->setValue(m_ui->windowOutlineCustomColorOpacityActive->value());
    m_ui->windowOutlineCustomColorOpacityInactive->setValue(m_internalSettings->windowOutlineCustomColorOpacityInactive() * 100);
    m_ui->windowOutlineCustomColorOpacityInactive_2->setValue(m_ui->windowOutlineCustomColorOpacityInactive->value());
    m_ui->windowOutlineCustomWithContrastOpacityActive->setValue(m_internalSettings->windowOutlineCustomWithContrastOpacityActive() * 100);
    m_ui->windowOutlineCustomWithContrastOpacityActive_2->setValue(m_ui->windowOutlineCustomWithContrastOpacityActive->value());
    m_ui->windowOutlineCustomWithContrastOpacityInactive->setValue(m_internalSettings->windowOutlineCustomWithContrastOpacityInactive() * 100);
    m_ui->windowOutlineCustomWithContrastOpacityInactive_2->setValue(m_ui->windowOutlineCustomWithContrastOpacityInactive->value());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

void WindowOutlineOpacity::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // emit changed(value);
}

void WindowOutlineOpacity::accept()
{
    save();
    QDialog::accept();
}

void WindowOutlineOpacity::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    // track modifications
    bool modified(false);

    if (m_ui->windowOutlineShadowColorOpacity->value() != m_internalSettings->windowOutlineShadowColorOpacity())
        modified = true;
    else if (m_ui->windowOutlineContrastOpacityActive->value() != m_internalSettings->windowOutlineContrastOpacityActive())
        modified = true;
    else if (m_ui->windowOutlineContrastOpacityInactive->value() != m_internalSettings->windowOutlineContrastOpacityInactive())
        modified = true;
    else if (m_ui->windowOutlineAccentColorOpacityActive->value() != m_internalSettings->windowOutlineAccentColorOpacityActive())
        modified = true;
    else if (m_ui->windowOutlineAccentColorOpacityInactive->value() != m_internalSettings->windowOutlineAccentColorOpacityInactive())
        modified = true;
    else if (m_ui->windowOutlineAccentWithContrastOpacityActive->value() != m_internalSettings->windowOutlineAccentWithContrastOpacityActive())
        modified = true;
    else if (m_ui->windowOutlineAccentWithContrastOpacityInactive->value() != m_internalSettings->windowOutlineAccentWithContrastOpacityInactive())
        modified = true;
    else if (m_ui->windowOutlineCustomColorOpacityActive->value() != m_internalSettings->windowOutlineCustomColorOpacityActive())
        modified = true;
    else if (m_ui->windowOutlineCustomColorOpacityInactive->value() != m_internalSettings->windowOutlineCustomColorOpacityInactive())
        modified = true;
    else if (m_ui->windowOutlineCustomWithContrastOpacityActive->value() != m_internalSettings->windowOutlineCustomWithContrastOpacityActive())
        modified = true;
    else if (m_ui->windowOutlineCustomWithContrastOpacityInactive->value() != m_internalSettings->windowOutlineCustomWithContrastOpacityInactive())
        modified = true;

    setChanged(modified);
}

void WindowOutlineOpacity::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

}
