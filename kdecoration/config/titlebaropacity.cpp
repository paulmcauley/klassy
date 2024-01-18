/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "titlebaropacity.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <KColorScheme>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPushButton>

namespace Breeze
{

TitleBarOpacity::TitleBarOpacity(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_TitleBarOpacity)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
{
    m_ui->setupUi(this);

    // track ui changes
    connect(m_ui->activeTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->opaqueMaximizedTitlebars, &QAbstractButton::toggled, this, &TitleBarOpacity::updateChanged);
    connect(m_ui->blurTransparentTitlebars, &QAbstractButton::toggled, this, &TitleBarOpacity::updateChanged);
    connect(m_ui->applyOpacityToHeader, &QAbstractButton::toggled, this, &TitleBarOpacity::updateChanged);

    // connect dual controls with same values
    connect(m_ui->activeTitlebarOpacity, SIGNAL(valueChanged(int)), m_ui->activeTitlebarOpacity_2, SLOT(setValue(int)));
    connect(m_ui->activeTitlebarOpacity_2, SIGNAL(valueChanged(int)), m_ui->activeTitlebarOpacity, SLOT(setValue(int)));
    connect(m_ui->inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), m_ui->inactiveTitlebarOpacity_2, SLOT(setValue(int)));
    connect(m_ui->inactiveTitlebarOpacity_2, SIGNAL(valueChanged(int)), m_ui->inactiveTitlebarOpacity, SLOT(setValue(int)));

    // only enable transparency options when transparency is setActiveTitlebarOpacity
    connect(m_ui->activeTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(setEnabledTransparentTitlebarOptions()));
    connect(m_ui->inactiveTitlebarOpacity, SIGNAL(valueChanged(int)), SLOT(setEnabledTransparentTitlebarOptions()));

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &TitleBarOpacity::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &TitleBarOpacity::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &TitleBarOpacity::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

TitleBarOpacity::~TitleBarOpacity()
{
    delete m_ui;
}

void TitleBarOpacity::load()
{
    m_loading = true;
    getTitlebarOpacityFromColorScheme();

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // if there is a non-opaque colour set in the system colour scheme then this overrides the control here and disables it
    if (m_translucentActiveSchemeColor) {
        m_ui->activeTitlebarOpacity->setValue(m_activeSchemeColorAlpha * 100);
        m_ui->activeTitlebarOpacity->setEnabled(false);
        m_ui->activeTitlebarOpacity_2->setEnabled(false);
    } else
        m_ui->activeTitlebarOpacity->setValue(m_internalSettings->activeTitlebarOpacity());
    m_ui->activeTitlebarOpacity_2->setValue(m_ui->activeTitlebarOpacity->value());
    if (m_translucentInactiveSchemeColor) {
        m_ui->inactiveTitlebarOpacity->setValue(m_inactiveSchemeColorAlpha * 100);
        m_ui->inactiveTitlebarOpacity->setEnabled(false);
        m_ui->inactiveTitlebarOpacity_2->setEnabled(false);
    } else
        m_ui->inactiveTitlebarOpacity->setValue(m_internalSettings->inactiveTitlebarOpacity());
    m_ui->inactiveTitlebarOpacity_2->setValue(m_ui->inactiveTitlebarOpacity->value());
    setEnabledTransparentTitlebarOptions();

    m_ui->opaqueMaximizedTitlebars->setChecked(m_internalSettings->opaqueMaximizedTitlebars());
    m_ui->blurTransparentTitlebars->setChecked(m_internalSettings->blurTransparentTitlebars());
    m_ui->applyOpacityToHeader->setChecked(m_internalSettings->applyOpacityToHeader());

    // TODO:add all lock variants
    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void TitleBarOpacity::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    if (!m_translucentActiveSchemeColor || m_defaultsPressed)
        m_internalSettings->setActiveTitlebarOpacity(m_ui->activeTitlebarOpacity->value());
    if (!m_translucentInactiveSchemeColor || m_defaultsPressed)
        m_internalSettings->setInactiveTitlebarOpacity(m_ui->inactiveTitlebarOpacity->value());

    m_internalSettings->setOpaqueMaximizedTitlebars(m_ui->opaqueMaximizedTitlebars->isChecked());
    m_internalSettings->setBlurTransparentTitlebars(m_ui->blurTransparentTitlebars->isChecked());
    m_internalSettings->setApplyOpacityToHeader(m_ui->applyOpacityToHeader->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        ConfigWidget::kwinReloadConfig();
        ConfigWidget::kstyleReloadConfig();
    }
}

void TitleBarOpacity::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui->activeTitlebarOpacity->setValue(m_internalSettings->activeTitlebarOpacity());
    m_ui->activeTitlebarOpacity_2->setValue(m_ui->activeTitlebarOpacity->value());
    m_ui->inactiveTitlebarOpacity->setValue(m_internalSettings->inactiveTitlebarOpacity());
    m_ui->inactiveTitlebarOpacity_2->setValue(m_ui->inactiveTitlebarOpacity->value());
    setEnabledTransparentTitlebarOptions();

    m_ui->opaqueMaximizedTitlebars->setChecked(m_internalSettings->opaqueMaximizedTitlebars());
    m_ui->blurTransparentTitlebars->setChecked(m_internalSettings->blurTransparentTitlebars());
    m_ui->applyOpacityToHeader->setChecked(m_internalSettings->applyOpacityToHeader());

    setChanged(!isDefaults());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

bool TitleBarOpacity::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("TitleBarOpacity"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void TitleBarOpacity::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // Q_EMIT changed(value);
}

void TitleBarOpacity::accept()
{
    save();
    QDialog::accept();
}

void TitleBarOpacity::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if ((!m_translucentActiveSchemeColor) && (m_ui->activeTitlebarOpacity->value() != m_internalSettings->activeTitlebarOpacity()))
        modified = true;
    else if ((!m_translucentInactiveSchemeColor) && (m_ui->inactiveTitlebarOpacity->value() != m_internalSettings->inactiveTitlebarOpacity()))
        modified = true;
    else if (m_ui->opaqueMaximizedTitlebars->isChecked() != m_internalSettings->opaqueMaximizedTitlebars())
        modified = true;
    else if (m_ui->blurTransparentTitlebars->isChecked() != m_internalSettings->blurTransparentTitlebars())
        modified = true;
    else if (m_ui->applyOpacityToHeader->isChecked() != m_internalSettings->applyOpacityToHeader())
        modified = true;

    setChanged(modified);
}

void TitleBarOpacity::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

// only enable blurTransparentTitlebars and opaqueMaximizedTitlebars options if transparent titlebars are enabled
void TitleBarOpacity::setEnabledTransparentTitlebarOptions()
{
    if (m_ui->activeTitlebarOpacity->value() != 100 || m_ui->inactiveTitlebarOpacity->value() != 100) {
        m_ui->opaqueMaximizedTitlebars->setEnabled(true);
        m_ui->blurTransparentTitlebars->setEnabled(true);
        m_ui->applyOpacityToHeader->setEnabled(true);
    } else {
        m_ui->opaqueMaximizedTitlebars->setEnabled(false);
        m_ui->blurTransparentTitlebars->setEnabled(false);
        m_ui->applyOpacityToHeader->setEnabled(false);
    }
}

void TitleBarOpacity::getTitlebarOpacityFromColorScheme()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    bool colorSchemeHasHeaderColor = KColorScheme::isColorSetSupported(config, KColorScheme::Header);

    // get the alpha values from the system colour scheme
    if (colorSchemeHasHeaderColor) {
        KColorScheme activeHeader = KColorScheme(QPalette::Active, KColorScheme::Header, config);
        m_translucentActiveSchemeColor = !activeHeader.background().isOpaque();
        m_activeSchemeColorAlpha = activeHeader.background().color().alphaF();

        KColorScheme inactiveHeader = KColorScheme(QPalette::Inactive, KColorScheme::Header, config);
        m_translucentInactiveSchemeColor = !inactiveHeader.background().isOpaque();
        m_inactiveSchemeColorAlpha = inactiveHeader.background().color().alphaF();
    } else {
        KConfigGroup wmConfig(config, QStringLiteral("WM"));
        if (wmConfig.exists()) {
            QColor activeTitlebarColor;
            QColor inactiveTitlebarColor;
            activeTitlebarColor = wmConfig.readEntry("activeBackground", QColorConstants::Black);
            inactiveTitlebarColor = wmConfig.readEntry("inactiveBackground", QColorConstants::Black);

            m_translucentActiveSchemeColor = (activeTitlebarColor.alpha() != 255);
            m_translucentInactiveSchemeColor = (inactiveTitlebarColor.alpha() != 255);
            m_activeSchemeColorAlpha = activeTitlebarColor.alphaF();
            m_inactiveSchemeColorAlpha = inactiveTitlebarColor.alphaF();
        }
    }
}

}
