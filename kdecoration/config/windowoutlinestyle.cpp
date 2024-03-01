/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "windowoutlinestyle.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "presetsmodel.h"
#include <KColorButton>
#include <QPushButton>

namespace Breeze
{

WindowOutlineStyle::WindowOutlineStyle(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_ui(new Ui_WindowOutlineStyle)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    // track ui changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui->thinWindowOutlineThickness, SIGNAL(valueChanged(double)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineStyleActive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineStyleActive,
            SIGNAL(currentIndexChanged(int)),
            SLOT(thinWindowOutlineStyleActiveChanged()),
            Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineStyleInactive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineStyleInactive,
            SIGNAL(currentIndexChanged(int)),
            SLOT(thinWindowOutlineStyleInactiveChanged()),
            Qt::ConnectionType::DirectConnection);
    connect(m_ui->lockThinWindowOutlineStyleActive, &QAbstractButton::toggled, this, &WindowOutlineStyle::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineShadowColorOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineContrastOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineContrastOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineAccentColorOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineAccentColorOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineAccentWithContrastOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineAccentWithContrastOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineCustomColorOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineCustomColorOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineCustomWithContrastOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineCustomWithContrastOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineCustomColorActive, &KColorButton::changed, this, &WindowOutlineStyle::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineCustomColorActive,
            &KColorButton::changed,
            this,
            &WindowOutlineStyle::thinWindowOutlineCustomColorActiveChanged,
            Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineCustomColorActive, &KColorButton::changed, m_ui->thinWindowOutlineCustomColorActive_2, &KColorButton::setColor);
    connect(m_ui->thinWindowOutlineCustomColorActive_2, &KColorButton::changed, m_ui->thinWindowOutlineCustomColorActive, &KColorButton::setColor);
    connect(m_ui->thinWindowOutlineCustomColorInactive, &KColorButton::changed, this, &WindowOutlineStyle::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineCustomColorInactive,
            &KColorButton::changed,
            this,
            &WindowOutlineStyle::thinWindowOutlineCustomColorInactiveChanged,
            Qt::ConnectionType::DirectConnection);
    connect(m_ui->thinWindowOutlineCustomColorInactive, &KColorButton::changed, m_ui->thinWindowOutlineCustomColorInactive_2, &KColorButton::setColor);
    connect(m_ui->thinWindowOutlineCustomColorInactive_2, &KColorButton::changed, m_ui->thinWindowOutlineCustomColorInactive, &KColorButton::setColor);
    connect(m_ui->lockThinWindowOutlineCustomColorActive,
            &QAbstractButton::toggled,
            this,
            &WindowOutlineStyle::updateChanged,
            Qt::ConnectionType::DirectConnection);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &WindowOutlineStyle::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &WindowOutlineStyle::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &WindowOutlineStyle::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

WindowOutlineStyle::~WindowOutlineStyle()
{
    delete m_ui;
}

void WindowOutlineStyle::loadMain(const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr(new InternalSettings());
        m_internalSettings->load();
    }

    m_ui->thinWindowOutlineThickness->setValue(m_internalSettings->thinWindowOutlineThickness());
    m_ui->thinWindowOutlineStyleActive->setCurrentIndex(m_internalSettings->thinWindowOutlineStyle(true));
    m_ui->thinWindowOutlineStyleInactive->setCurrentIndex(m_internalSettings->thinWindowOutlineStyle(false));
    m_ui->lockThinWindowOutlineStyleActive->setChecked(m_internalSettings->lockThinWindowOutlineStyleActiveInactive());
    m_ui->lockThinWindowOutlineStyleInactive->setChecked(m_internalSettings->lockThinWindowOutlineStyleActiveInactive());
    m_ui->windowOutlineShadowColorOpacity->setValue(m_internalSettings->windowOutlineShadowColorOpacity());
    m_ui->windowOutlineShadowColorOpacity_2->setValue(m_ui->windowOutlineShadowColorOpacity->value());
    m_ui->windowOutlineContrastOpacityActive->setValue(m_internalSettings->windowOutlineContrastOpacity(true));
    m_ui->windowOutlineContrastOpacityActive_2->setValue(m_ui->windowOutlineContrastOpacityActive->value());
    m_ui->windowOutlineContrastOpacityInactive->setValue(m_internalSettings->windowOutlineContrastOpacity(false));
    m_ui->windowOutlineContrastOpacityInactive_2->setValue(m_ui->windowOutlineContrastOpacityInactive->value());
    m_ui->windowOutlineAccentColorOpacityActive->setValue(m_internalSettings->windowOutlineAccentColorOpacity(true));
    m_ui->windowOutlineAccentColorOpacityActive_2->setValue(m_ui->windowOutlineAccentColorOpacityActive->value());
    m_ui->windowOutlineAccentColorOpacityInactive->setValue(m_internalSettings->windowOutlineAccentColorOpacity(false));
    m_ui->windowOutlineAccentColorOpacityInactive_2->setValue(m_ui->windowOutlineAccentColorOpacityInactive->value());
    m_ui->windowOutlineAccentWithContrastOpacityActive->setValue(m_internalSettings->windowOutlineAccentWithContrastOpacity(true));
    m_ui->windowOutlineAccentWithContrastOpacityActive_2->setValue(m_ui->windowOutlineAccentWithContrastOpacityActive->value());
    m_ui->windowOutlineAccentWithContrastOpacityInactive->setValue(m_internalSettings->windowOutlineAccentWithContrastOpacity(false));
    m_ui->windowOutlineAccentWithContrastOpacityInactive_2->setValue(m_ui->windowOutlineAccentWithContrastOpacityInactive->value());
    m_ui->windowOutlineCustomColorOpacityActive->setValue(m_internalSettings->windowOutlineCustomColorOpacity(true));
    m_ui->windowOutlineCustomColorOpacityActive_2->setValue(m_ui->windowOutlineCustomColorOpacityActive->value());
    m_ui->windowOutlineCustomColorOpacityInactive->setValue(m_internalSettings->windowOutlineCustomColorOpacity(false));
    m_ui->windowOutlineCustomColorOpacityInactive_2->setValue(m_ui->windowOutlineCustomColorOpacityInactive->value());
    m_ui->windowOutlineCustomWithContrastOpacityActive->setValue(m_internalSettings->windowOutlineCustomWithContrastOpacity(true));
    m_ui->windowOutlineCustomWithContrastOpacityActive_2->setValue(m_ui->windowOutlineCustomWithContrastOpacityActive->value());
    m_ui->windowOutlineCustomWithContrastOpacityInactive->setValue(m_internalSettings->windowOutlineCustomWithContrastOpacity(false));
    m_ui->windowOutlineCustomWithContrastOpacityInactive_2->setValue(m_ui->windowOutlineCustomWithContrastOpacityInactive->value());
    m_ui->thinWindowOutlineCustomColorActive->setColor(m_internalSettings->thinWindowOutlineCustomColor(true));
    m_ui->thinWindowOutlineCustomColorActive_2->setColor(m_ui->thinWindowOutlineCustomColorActive->color());
    m_ui->thinWindowOutlineCustomColorInactive->setColor(m_internalSettings->thinWindowOutlineCustomColor(false));
    m_ui->thinWindowOutlineCustomColorInactive_2->setColor(m_ui->thinWindowOutlineCustomColorInactive->color());
    m_ui->lockThinWindowOutlineCustomColorActive->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorActive_2->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorInactive->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorInactive_2->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void WindowOutlineStyle::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setThinWindowOutlineThickness(m_ui->thinWindowOutlineThickness->value());
    m_internalSettings->setThinWindowOutlineStyle(true, m_ui->thinWindowOutlineStyleActive->currentIndex());
    m_internalSettings->setThinWindowOutlineStyle(false, m_ui->thinWindowOutlineStyleInactive->currentIndex());
    m_internalSettings->setLockThinWindowOutlineStyleActiveInactive(m_ui->lockThinWindowOutlineStyleActive->isChecked());
    m_internalSettings->setWindowOutlineShadowColorOpacity(qreal(m_ui->windowOutlineShadowColorOpacity->value()));
    m_internalSettings->setWindowOutlineContrastOpacity(true, qreal(m_ui->windowOutlineContrastOpacityActive->value()));
    m_internalSettings->setWindowOutlineContrastOpacity(false, qreal(m_ui->windowOutlineContrastOpacityInactive->value()));
    m_internalSettings->setWindowOutlineAccentColorOpacity(true, qreal(m_ui->windowOutlineAccentColorOpacityActive->value()));
    m_internalSettings->setWindowOutlineAccentColorOpacity(false, qreal(m_ui->windowOutlineAccentColorOpacityInactive->value()));
    m_internalSettings->setWindowOutlineAccentWithContrastOpacity(true, qreal(m_ui->windowOutlineAccentWithContrastOpacityActive->value()));
    m_internalSettings->setWindowOutlineAccentWithContrastOpacity(false, qreal(m_ui->windowOutlineAccentWithContrastOpacityInactive->value()));
    m_internalSettings->setWindowOutlineCustomColorOpacity(true, qreal(m_ui->windowOutlineCustomColorOpacityActive->value()));
    m_internalSettings->setWindowOutlineCustomColorOpacity(false, qreal(m_ui->windowOutlineCustomColorOpacityInactive->value()));
    m_internalSettings->setWindowOutlineCustomWithContrastOpacity(true, qreal(m_ui->windowOutlineCustomWithContrastOpacityActive->value()));
    m_internalSettings->setWindowOutlineCustomWithContrastOpacity(false, qreal(m_ui->windowOutlineCustomWithContrastOpacityInactive->value()));
    m_internalSettings->setThinWindowOutlineCustomColor(true, m_ui->thinWindowOutlineCustomColorActive->color());
    m_internalSettings->setThinWindowOutlineCustomColor(false, m_ui->thinWindowOutlineCustomColorInactive->color());
    m_internalSettings->setLockThinWindowOutlineCustomColorActiveInactive(m_ui->lockThinWindowOutlineCustomColorActive->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        DBusMessages::updateDecorationColorCache();
        DBusMessages::kwinReloadConfig();

        static_cast<ConfigWidget *>(m_parent)->generateSystemIcons();
    }
}

void WindowOutlineStyle::defaults()
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

bool WindowOutlineStyle::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("WindowOutlineStyle"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void WindowOutlineStyle::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    Q_EMIT changed(value);
}

void WindowOutlineStyle::accept()
{
    save();
    QDialog::accept();
}

void WindowOutlineStyle::reject()
{
    load();
    QDialog::reject();
}

void WindowOutlineStyle::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui->thinWindowOutlineThickness->value() != m_internalSettings->thinWindowOutlineThickness())
        modified = true;
    else if (m_ui->thinWindowOutlineStyleActive->currentIndex() != m_internalSettings->thinWindowOutlineStyle(true))
        modified = true;
    else if (m_ui->thinWindowOutlineStyleInactive->currentIndex() != m_internalSettings->thinWindowOutlineStyle(false))
        modified = true;
    else if (m_ui->lockThinWindowOutlineStyleActive->isChecked() != m_internalSettings->lockThinWindowOutlineStyleActiveInactive())
        modified = true;
    else if (m_ui->windowOutlineShadowColorOpacity->value() != m_internalSettings->windowOutlineShadowColorOpacity())
        modified = true;
    else if (m_ui->windowOutlineContrastOpacityActive->value() != m_internalSettings->windowOutlineContrastOpacity(true))
        modified = true;
    else if (m_ui->windowOutlineContrastOpacityInactive->value() != m_internalSettings->windowOutlineContrastOpacity(false))
        modified = true;
    else if (m_ui->windowOutlineAccentColorOpacityActive->value() != m_internalSettings->windowOutlineAccentColorOpacity(true))
        modified = true;
    else if (m_ui->windowOutlineAccentColorOpacityInactive->value() != m_internalSettings->windowOutlineAccentColorOpacity(false))
        modified = true;
    else if (m_ui->windowOutlineAccentWithContrastOpacityActive->value() != m_internalSettings->windowOutlineAccentWithContrastOpacity(true))
        modified = true;
    else if (m_ui->windowOutlineAccentWithContrastOpacityInactive->value() != m_internalSettings->windowOutlineAccentWithContrastOpacity(false))
        modified = true;
    else if (m_ui->windowOutlineCustomColorOpacityActive->value() != m_internalSettings->windowOutlineCustomColorOpacity(true))
        modified = true;
    else if (m_ui->windowOutlineCustomColorOpacityInactive->value() != m_internalSettings->windowOutlineCustomColorOpacity(false))
        modified = true;
    else if (m_ui->windowOutlineCustomWithContrastOpacityActive->value() != m_internalSettings->windowOutlineCustomWithContrastOpacity(true))
        modified = true;
    else if (m_ui->windowOutlineCustomWithContrastOpacityInactive->value() != m_internalSettings->windowOutlineCustomWithContrastOpacity(false))
        modified = true;
    else if (m_ui->thinWindowOutlineCustomColorActive->color() != m_internalSettings->thinWindowOutlineCustomColor(true))
        modified = true;
    else if (m_ui->thinWindowOutlineCustomColorInactive->color() != m_internalSettings->thinWindowOutlineCustomColor(false))
        modified = true;
    else if (m_ui->lockThinWindowOutlineCustomColorActive->isChecked() != m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive())
        modified = true;

    setChanged(modified);
}

void WindowOutlineStyle::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void WindowOutlineStyle::thinWindowOutlineStyleActiveChanged()
{
    if (m_ui->lockThinWindowOutlineStyleActive->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->thinWindowOutlineStyleInactive->setCurrentIndex(m_ui->thinWindowOutlineStyleActive->currentIndex());
}

void WindowOutlineStyle::thinWindowOutlineStyleInactiveChanged()
{
    if (m_ui->lockThinWindowOutlineStyleInactive->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->thinWindowOutlineStyleActive->setCurrentIndex(m_ui->thinWindowOutlineStyleInactive->currentIndex());
}

void WindowOutlineStyle::thinWindowOutlineCustomColorActiveChanged()
{
    if (m_ui->lockThinWindowOutlineCustomColorActive->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->thinWindowOutlineCustomColorInactive->setColor(m_ui->thinWindowOutlineCustomColorActive->color());
}

void WindowOutlineStyle::thinWindowOutlineCustomColorInactiveChanged()
{
    if (m_ui->lockThinWindowOutlineCustomColorInactive->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->thinWindowOutlineCustomColorActive->setColor(m_ui->thinWindowOutlineCustomColorInactive->color());
}

void WindowOutlineStyle::updateLockIcons()
{
    m_ui->lockThinWindowOutlineStyleActive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockThinWindowOutlineStyleInactive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
}
}
