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
    connect(m_ui->windowOutlineThickness, SIGNAL(valueChanged(double)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineSnapToWholePixel, &QAbstractButton::toggled, this, &WindowOutlineStyle::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineOverlap, &QAbstractButton::toggled, this, &WindowOutlineStyle::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineStyleActive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineStyleActive, SIGNAL(currentIndexChanged(int)), SLOT(windowOutlineStyleActiveChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineStyleInactive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineStyleInactive,
            SIGNAL(currentIndexChanged(int)),
            SLOT(windowOutlineStyleInactiveChanged()),
            Qt::ConnectionType::DirectConnection);
    connect(m_ui->lockWindowOutlineStyleActive, &QAbstractButton::toggled, this, &WindowOutlineStyle::updateChanged, Qt::ConnectionType::DirectConnection);
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
    connect(m_ui->windowOutlineCustomColorActive, &KColorButton::changed, this, &WindowOutlineStyle::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineCustomColorActive,
            &KColorButton::changed,
            this,
            &WindowOutlineStyle::windowOutlineCustomColorActiveChanged,
            Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineCustomColorActive, &KColorButton::changed, m_ui->windowOutlineCustomColorActive_2, &KColorButton::setColor);
    connect(m_ui->windowOutlineCustomColorActive_2, &KColorButton::changed, m_ui->windowOutlineCustomColorActive, &KColorButton::setColor);
    connect(m_ui->windowOutlineCustomColorInactive, &KColorButton::changed, this, &WindowOutlineStyle::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineCustomColorInactive,
            &KColorButton::changed,
            this,
            &WindowOutlineStyle::windowOutlineCustomColorInactiveChanged,
            Qt::ConnectionType::DirectConnection);
    connect(m_ui->windowOutlineCustomColorInactive, &KColorButton::changed, m_ui->windowOutlineCustomColorInactive_2, &KColorButton::setColor);
    connect(m_ui->windowOutlineCustomColorInactive_2, &KColorButton::changed, m_ui->windowOutlineCustomColorInactive, &KColorButton::setColor);
    connect(m_ui->lockWindowOutlineCustomColorActive,
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

    m_ui->windowOutlineThickness->setValue(m_internalSettings->windowOutlineThickness());
    m_ui->windowOutlineSnapToWholePixel->setChecked(m_internalSettings->windowOutlineSnapToWholePixel());
    m_ui->windowOutlineOverlap->setChecked(m_internalSettings->windowOutlineOverlap());
    m_ui->windowOutlineStyleActive->setCurrentIndex(m_internalSettings->windowOutlineStyle(true));
    m_ui->windowOutlineStyleInactive->setCurrentIndex(m_internalSettings->windowOutlineStyle(false));
    m_ui->lockWindowOutlineStyleActive->setChecked(m_internalSettings->lockWindowOutlineStyleActiveInactive());
    m_ui->lockWindowOutlineStyleInactive->setChecked(m_internalSettings->lockWindowOutlineStyleActiveInactive());
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
    m_ui->windowOutlineCustomColorActive->setColor(m_internalSettings->windowOutlineCustomColor(true));
    m_ui->windowOutlineCustomColorActive_2->setColor(m_ui->windowOutlineCustomColorActive->color());
    m_ui->windowOutlineCustomColorInactive->setColor(m_internalSettings->windowOutlineCustomColor(false));
    m_ui->windowOutlineCustomColorInactive_2->setColor(m_ui->windowOutlineCustomColorInactive->color());
    m_ui->lockWindowOutlineCustomColorActive->setChecked(m_internalSettings->lockWindowOutlineCustomColorActiveInactive());
    m_ui->lockWindowOutlineCustomColorActive_2->setChecked(m_internalSettings->lockWindowOutlineCustomColorActiveInactive());
    m_ui->lockWindowOutlineCustomColorInactive->setChecked(m_internalSettings->lockWindowOutlineCustomColorActiveInactive());
    m_ui->lockWindowOutlineCustomColorInactive_2->setChecked(m_internalSettings->lockWindowOutlineCustomColorActiveInactive());

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
    m_internalSettings->setWindowOutlineThickness(m_ui->windowOutlineThickness->value());
    m_internalSettings->setWindowOutlineSnapToWholePixel(m_ui->windowOutlineSnapToWholePixel->isChecked());
    m_internalSettings->setWindowOutlineOverlap(m_ui->windowOutlineOverlap->isChecked());
    m_internalSettings->setWindowOutlineStyle(true, m_ui->windowOutlineStyleActive->currentIndex());
    m_internalSettings->setWindowOutlineStyle(false, m_ui->windowOutlineStyleInactive->currentIndex());
    m_internalSettings->setLockWindowOutlineStyleActiveInactive(m_ui->lockWindowOutlineStyleActive->isChecked());
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
    m_internalSettings->setWindowOutlineCustomColor(true, m_ui->windowOutlineCustomColorActive->color());
    m_internalSettings->setWindowOutlineCustomColor(false, m_ui->windowOutlineCustomColorInactive->color());
    m_internalSettings->setLockWindowOutlineCustomColorActiveInactive(m_ui->lockWindowOutlineCustomColorActive->isChecked());

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

    if (m_ui->windowOutlineThickness->value() != m_internalSettings->windowOutlineThickness())
        modified = true;
    else if (m_ui->windowOutlineSnapToWholePixel->isChecked() != m_internalSettings->windowOutlineSnapToWholePixel())
        modified = true;
    else if (m_ui->windowOutlineOverlap->isChecked() != m_internalSettings->windowOutlineOverlap())
        modified = true;
    else if (m_ui->windowOutlineStyleActive->currentIndex() != m_internalSettings->windowOutlineStyle(true))
        modified = true;
    else if (m_ui->windowOutlineStyleInactive->currentIndex() != m_internalSettings->windowOutlineStyle(false))
        modified = true;
    else if (m_ui->lockWindowOutlineStyleActive->isChecked() != m_internalSettings->lockWindowOutlineStyleActiveInactive())
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
    else if (m_ui->windowOutlineCustomColorActive->color() != m_internalSettings->windowOutlineCustomColor(true))
        modified = true;
    else if (m_ui->windowOutlineCustomColorInactive->color() != m_internalSettings->windowOutlineCustomColor(false))
        modified = true;
    else if (m_ui->lockWindowOutlineCustomColorActive->isChecked() != m_internalSettings->lockWindowOutlineCustomColorActiveInactive())
        modified = true;

    setChanged(modified);
}

void WindowOutlineStyle::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void WindowOutlineStyle::windowOutlineStyleActiveChanged()
{
    if (m_ui->lockWindowOutlineStyleActive->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->windowOutlineStyleInactive->setCurrentIndex(m_ui->windowOutlineStyleActive->currentIndex());
}

void WindowOutlineStyle::windowOutlineStyleInactiveChanged()
{
    if (m_ui->lockWindowOutlineStyleInactive->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->windowOutlineStyleActive->setCurrentIndex(m_ui->windowOutlineStyleInactive->currentIndex());
}

void WindowOutlineStyle::windowOutlineCustomColorActiveChanged()
{
    if (m_ui->lockWindowOutlineCustomColorActive->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->windowOutlineCustomColorInactive->setColor(m_ui->windowOutlineCustomColorActive->color());
}

void WindowOutlineStyle::windowOutlineCustomColorInactiveChanged()
{
    if (m_ui->lockWindowOutlineCustomColorInactive->isChecked() && !m_processingDefaults && !m_loading)
        m_ui->windowOutlineCustomColorActive->setColor(m_ui->windowOutlineCustomColorInactive->color());
}

void WindowOutlineStyle::updateLockIcons()
{
    m_ui->lockWindowOutlineStyleActive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockWindowOutlineStyleInactive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockWindowOutlineCustomColorActive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockWindowOutlineCustomColorInactive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockWindowOutlineCustomColorActive_2->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockWindowOutlineCustomColorInactive_2->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
}
}
