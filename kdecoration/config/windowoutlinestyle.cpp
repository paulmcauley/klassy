/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "windowoutlinestyle.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <KColorButton>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPushButton>

namespace Breeze
{

WindowOutlineStyle::WindowOutlineStyle(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_WindowOutlineStyle)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
{
    m_ui->setupUi(this);

    // track ui changes
    connect(m_ui->thinWindowOutlineThickness, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    connect(m_ui->thinWindowOutlineStyleActive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->thinWindowOutlineStyleActive, SIGNAL(currentIndexChanged(int)), SLOT(thinWindowOutlineStyleActiveChanged()));
    connect(m_ui->thinWindowOutlineStyleInactive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->thinWindowOutlineStyleInactive, SIGNAL(currentIndexChanged(int)), SLOT(thinWindowOutlineStyleInactiveChanged()));
    connect(m_ui->lockThinWindowOutlineStyleActive, &QAbstractButton::toggled, this, &WindowOutlineStyle::updateChanged);
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
    connect(m_ui->thinWindowOutlineCustomColorActive, &KColorButton::changed, this, &WindowOutlineStyle::updateChanged);
    connect(m_ui->thinWindowOutlineCustomColorActive, &KColorButton::changed, this, &WindowOutlineStyle::thinWindowOutlineCustomColorActiveChanged);
    connect(m_ui->thinWindowOutlineCustomColorActive, &KColorButton::changed, m_ui->thinWindowOutlineCustomColorActive_2, &KColorButton::setColor);
    connect(m_ui->thinWindowOutlineCustomColorActive_2, &KColorButton::changed, m_ui->thinWindowOutlineCustomColorActive, &KColorButton::setColor);
    connect(m_ui->thinWindowOutlineCustomColorInactive, &KColorButton::changed, this, &WindowOutlineStyle::updateChanged);
    connect(m_ui->thinWindowOutlineCustomColorInactive, &KColorButton::changed, this, &WindowOutlineStyle::thinWindowOutlineCustomColorInactiveChanged);
    connect(m_ui->thinWindowOutlineCustomColorInactive, &KColorButton::changed, m_ui->thinWindowOutlineCustomColorInactive_2, &KColorButton::setColor);
    connect(m_ui->thinWindowOutlineCustomColorInactive_2, &KColorButton::changed, m_ui->thinWindowOutlineCustomColorInactive, &KColorButton::setColor);
    connect(m_ui->lockThinWindowOutlineCustomColorActive, &QAbstractButton::toggled, this, &WindowOutlineStyle::updateChanged);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &WindowOutlineStyle::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &WindowOutlineStyle::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &WindowOutlineStyle::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

WindowOutlineStyle::~WindowOutlineStyle()
{
    delete m_ui;
}

void WindowOutlineStyle::load()
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    m_ui->thinWindowOutlineThickness->setValue(m_internalSettings->thinWindowOutlineThickness());
    m_ui->thinWindowOutlineStyleActive->setCurrentIndex(m_internalSettings->thinWindowOutlineStyleActive());
    m_ui->thinWindowOutlineStyleInactive->setCurrentIndex(m_internalSettings->thinWindowOutlineStyleInactive());
    m_ui->lockThinWindowOutlineStyleActive->setChecked(m_internalSettings->lockThinWindowOutlineStyleActiveInactive());
    m_ui->lockThinWindowOutlineStyleInactive->setChecked(m_internalSettings->lockThinWindowOutlineStyleActiveInactive());
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
    m_ui->thinWindowOutlineCustomColorActive->setColor(m_internalSettings->thinWindowOutlineCustomColorActive());
    m_ui->thinWindowOutlineCustomColorActive_2->setColor(m_ui->thinWindowOutlineCustomColorActive->color());
    m_ui->thinWindowOutlineCustomColorInactive->setColor(m_internalSettings->thinWindowOutlineCustomColorInactive());
    m_ui->thinWindowOutlineCustomColorInactive_2->setColor(m_ui->thinWindowOutlineCustomColorInactive->color());
    m_ui->lockThinWindowOutlineCustomColorActive->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorActive_2->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorInactive->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorInactive_2->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());

    // TODO:add all lock variants
    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void WindowOutlineStyle::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setThinWindowOutlineThickness(m_ui->thinWindowOutlineThickness->value());
    m_internalSettings->setThinWindowOutlineStyleActive(m_ui->thinWindowOutlineStyleActive->currentIndex());
    m_internalSettings->setThinWindowOutlineStyleInactive(m_ui->thinWindowOutlineStyleInactive->currentIndex());
    m_internalSettings->setLockThinWindowOutlineStyleActiveInactive(m_ui->lockThinWindowOutlineStyleActive->isChecked());
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
    m_internalSettings->setThinWindowOutlineCustomColorActive(m_ui->thinWindowOutlineCustomColorActive->color());
    m_internalSettings->setThinWindowOutlineCustomColorInactive(m_ui->thinWindowOutlineCustomColorInactive->color());
    m_internalSettings->setLockThinWindowOutlineCustomColorActiveInactive(m_ui->lockThinWindowOutlineCustomColorActive->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig)
        ConfigWidget::kwinReloadConfig();
}

void WindowOutlineStyle::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui->thinWindowOutlineThickness->setValue(m_internalSettings->thinWindowOutlineThickness());
    m_ui->thinWindowOutlineStyleActive->setCurrentIndex(m_internalSettings->thinWindowOutlineStyleActive());
    m_ui->thinWindowOutlineStyleInactive->setCurrentIndex(m_internalSettings->thinWindowOutlineStyleInactive());
    m_ui->lockThinWindowOutlineStyleActive->setChecked(m_internalSettings->lockThinWindowOutlineStyleActiveInactive());
    m_ui->lockThinWindowOutlineStyleInactive->setChecked(m_internalSettings->lockThinWindowOutlineStyleActiveInactive());
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
    m_ui->thinWindowOutlineCustomColorActive->setColor(m_internalSettings->thinWindowOutlineCustomColorActive());
    m_ui->thinWindowOutlineCustomColorActive_2->setColor(m_ui->thinWindowOutlineCustomColorActive->color());
    m_ui->thinWindowOutlineCustomColorInactive->setColor(m_internalSettings->thinWindowOutlineCustomColorInactive());
    m_ui->thinWindowOutlineCustomColorInactive_2->setColor(m_ui->thinWindowOutlineCustomColorInactive->color());
    m_ui->lockThinWindowOutlineCustomColorActive->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorActive->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorActive_2->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorInactive->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());
    m_ui->lockThinWindowOutlineCustomColorInactive_2->setChecked(m_internalSettings->lockThinWindowOutlineCustomColorActiveInactive());

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
    // Q_EMIT changed(value);
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
    else if (m_ui->thinWindowOutlineStyleActive->currentIndex() != m_internalSettings->thinWindowOutlineStyleActive())
        modified = true;
    else if (m_ui->thinWindowOutlineStyleInactive->currentIndex() != m_internalSettings->thinWindowOutlineStyleInactive())
        modified = true;
    else if (m_ui->lockThinWindowOutlineStyleActive->isChecked() != m_internalSettings->lockThinWindowOutlineStyleActiveInactive())
        modified = true;
    else if (m_ui->windowOutlineShadowColorOpacity->value() != m_internalSettings->windowOutlineShadowColorOpacity())
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
    else if (m_ui->thinWindowOutlineCustomColorActive->color() != m_internalSettings->thinWindowOutlineCustomColorActive())
        modified = true;
    else if (m_ui->thinWindowOutlineCustomColorInactive->color() != m_internalSettings->thinWindowOutlineCustomColorInactive())
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

}
