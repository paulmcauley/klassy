/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "titlebaropacity.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "decorationcolors.h"
#include "presetsmodel.h"
#include <KColorScheme>
#include <QPushButton>

namespace Breeze
{

TitleBarOpacity::TitleBarOpacity(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_ui(new Ui_TitleBarOpacity)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    // track ui changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui->titleBarOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->titleBarOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->opaqueMaximizedTitleBars, &QAbstractButton::toggled, this, &TitleBarOpacity::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->blurTransparentTitleBars, &QAbstractButton::toggled, this, &TitleBarOpacity::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->applyOpacityToHeader, &QAbstractButton::toggled, this, &TitleBarOpacity::updateChanged, Qt::ConnectionType::DirectConnection);

    // connect dual controls with same values
    connect(m_ui->titleBarOpacityActive, SIGNAL(valueChanged(int)), m_ui->titleBarOpacityActive_2, SLOT(setValue(int)));
    connect(m_ui->titleBarOpacityActive_2, SIGNAL(valueChanged(int)), m_ui->titleBarOpacityActive, SLOT(setValue(int)));
    connect(m_ui->titleBarOpacityInactive, SIGNAL(valueChanged(int)), m_ui->titleBarOpacityInactive_2, SLOT(setValue(int)));
    connect(m_ui->titleBarOpacityInactive_2, SIGNAL(valueChanged(int)), m_ui->titleBarOpacityInactive, SLOT(setValue(int)));

    connect(m_ui->overrideTitleBarOpacityActive, &QAbstractButton::toggled, this, &TitleBarOpacity::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->overrideTitleBarOpacityInactive, &QAbstractButton::toggled, this, &TitleBarOpacity::updateChanged, Qt::ConnectionType::DirectConnection);

    // only enable transparency options when transparency is setTitleBarOpacityActive
    connect(m_ui->titleBarOpacityActive, SIGNAL(valueChanged(int)), SLOT(setEnabledTransparentTitlebarOptions()));
    connect(m_ui->titleBarOpacityInactive, SIGNAL(valueChanged(int)), SLOT(setEnabledTransparentTitlebarOptions()));
    connect(m_ui->overrideTitleBarOpacityActive, &QAbstractButton::toggled, this, &TitleBarOpacity::setEnabledTransparentTitlebarOptions);
    connect(m_ui->overrideTitleBarOpacityInactive, &QAbstractButton::toggled, this, &TitleBarOpacity::setEnabledTransparentTitlebarOptions);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &TitleBarOpacity::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &TitleBarOpacity::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &TitleBarOpacity::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

TitleBarOpacity::~TitleBarOpacity()
{
    delete m_ui;
}

void TitleBarOpacity::loadMain(const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;
        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr(new InternalSettings());
        m_internalSettings->load();
    }

    getTitlebarOpacityFromColorScheme();
    m_ui->overrideTitleBarOpacityActive->setChecked(m_internalSettings->overrideTitleBarOpacityActive());
    m_ui->overrideTitleBarOpacityInactive->setChecked(m_internalSettings->overrideTitleBarOpacityInactive());

    // if there is a non-opaque colour set in the system colour scheme then this overrides the control here and disables it
    if (m_translucentActiveSchemeColor && !m_ui->overrideTitleBarOpacityActive->isChecked()) {
        m_ui->titleBarOpacityActive->setValue(m_activeSchemeColorAlpha * 100);
    } else {
        m_ui->titleBarOpacityActive->setValue(m_internalSettings->titleBarOpacityActive());
    }
    m_ui->titleBarOpacityActive_2->setValue(m_ui->titleBarOpacityActive->value());

    m_ui->overrideTitleBarOpacityInactive->setChecked(m_internalSettings->overrideTitleBarOpacityInactive());
    if (m_translucentInactiveSchemeColor && !m_ui->overrideTitleBarOpacityInactive->isChecked()) {
        m_ui->titleBarOpacityInactive->setValue(m_inactiveSchemeColorAlpha * 100);
    } else {
        m_ui->titleBarOpacityInactive->setValue(m_internalSettings->titleBarOpacityInactive());
    }
    m_ui->titleBarOpacityInactive_2->setValue(m_ui->titleBarOpacityInactive->value());
    setEnabledTransparentTitlebarOptions();

    m_ui->opaqueMaximizedTitleBars->setChecked(m_internalSettings->opaqueMaximizedTitleBars());
    m_ui->blurTransparentTitleBars->setChecked(m_internalSettings->blurTransparentTitleBars());
    m_ui->applyOpacityToHeader->setChecked(m_internalSettings->applyOpacityToHeader());

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void TitleBarOpacity::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    if (m_translucentActiveSchemeColor) {
        m_internalSettings->setOverrideTitleBarOpacityActive(m_ui->overrideTitleBarOpacityActive->isChecked());
    }
    if (m_translucentInactiveSchemeColor) {
        m_internalSettings->setOverrideTitleBarOpacityInactive(m_ui->overrideTitleBarOpacityInactive->isChecked());
    }
    // apply modifications from ui
    if (!m_translucentActiveSchemeColor || (m_translucentActiveSchemeColor && m_ui->overrideTitleBarOpacityActive->isChecked()))
        m_internalSettings->setTitleBarOpacityActive(m_ui->titleBarOpacityActive->value());
    if (!m_translucentInactiveSchemeColor || (m_translucentActiveSchemeColor && m_ui->overrideTitleBarOpacityActive->isChecked()))
        m_internalSettings->setTitleBarOpacityInactive(m_ui->titleBarOpacityInactive->value());

    m_internalSettings->setOpaqueMaximizedTitleBars(m_ui->opaqueMaximizedTitleBars->isChecked());
    m_internalSettings->setBlurTransparentTitleBars(m_ui->blurTransparentTitleBars->isChecked());
    m_internalSettings->setApplyOpacityToHeader(m_ui->applyOpacityToHeader->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        DBusMessages::updateDecorationColorCache();
        DBusMessages::kwinReloadConfig();
        // DBusMessages::kstyleReloadDecorationConfig(); //should reload anyway

        static_cast<ConfigWidget *>(m_parent)->generateSystemIcons();
    }
}

void TitleBarOpacity::defaults()
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
    Q_EMIT changed(value);
}

void TitleBarOpacity::accept()
{
    save();
    QDialog::accept();
}

void TitleBarOpacity::reject()
{
    load();
    QDialog::reject();
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

    if ((!m_translucentActiveSchemeColor) && (m_ui->titleBarOpacityActive->value() != m_internalSettings->titleBarOpacityActive()))
        modified = true;
    else if ((!m_translucentInactiveSchemeColor) && (m_ui->titleBarOpacityInactive->value() != m_internalSettings->titleBarOpacityInactive()))
        modified = true;
    else if (m_translucentActiveSchemeColor && m_ui->overrideTitleBarOpacityActive->isChecked()
             && (m_ui->titleBarOpacityActive->value() != m_internalSettings->titleBarOpacityActive()))
        modified = true;
    else if (m_translucentInactiveSchemeColor && m_ui->overrideTitleBarOpacityInactive->isChecked()
             && (m_ui->titleBarOpacityInactive->value() != m_internalSettings->titleBarOpacityInactive()))
        modified = true;
    else if (m_translucentActiveSchemeColor && (m_ui->overrideTitleBarOpacityActive->isChecked() != m_internalSettings->overrideTitleBarOpacityActive()))
        modified = true;
    else if (m_translucentInactiveSchemeColor && (m_ui->overrideTitleBarOpacityInactive->isChecked() != m_internalSettings->overrideTitleBarOpacityInactive()))
        modified = true;
    else if (m_ui->opaqueMaximizedTitleBars->isChecked() != m_internalSettings->opaqueMaximizedTitleBars())
        modified = true;
    else if (m_ui->blurTransparentTitleBars->isChecked() != m_internalSettings->blurTransparentTitleBars())
        modified = true;
    else if (m_ui->applyOpacityToHeader->isChecked() != m_internalSettings->applyOpacityToHeader())
        modified = true;

    setChanged(modified);
}

void TitleBarOpacity::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

// only enable blurTransparentTitleBars and opaqueMaximizedTitleBars options if transparent titlebars are enabled
void TitleBarOpacity::setEnabledTransparentTitlebarOptions()
{
    if (m_translucentActiveSchemeColor) {
        m_ui->overrideTitleBarOpacityActive->setVisible(true);
    } else {
        m_ui->overrideTitleBarOpacityActive->setVisible(false);
    }

    if (m_translucentActiveSchemeColor && !m_ui->overrideTitleBarOpacityActive->isChecked()) {
        m_ui->titleBarOpacityActive->setValue(m_activeSchemeColorAlpha * 100);
        m_ui->titleBarOpacityActive->setEnabled(false);
        m_ui->titleBarOpacityActive_2->setEnabled(false);
        m_ui->titleBarOpacityActiveFromColorSchemeLabel->setVisible(true);
    } else {
        m_ui->titleBarOpacityActive->setEnabled(true);
        m_ui->titleBarOpacityActive_2->setEnabled(true);
        m_ui->titleBarOpacityActiveFromColorSchemeLabel->setVisible(false);
    }

    if (m_translucentInactiveSchemeColor) {
        m_ui->overrideTitleBarOpacityInactive->setVisible(true);
    } else {
        m_ui->overrideTitleBarOpacityInactive->setVisible(false);
    }

    if (m_translucentInactiveSchemeColor && !m_ui->overrideTitleBarOpacityInactive->isChecked()) {
        m_ui->titleBarOpacityInactive->setValue(m_inactiveSchemeColorAlpha * 100);
        m_ui->titleBarOpacityInactive->setEnabled(false);
        m_ui->titleBarOpacityInactive_2->setEnabled(false);
        m_ui->titleBarOpacityInactiveFromColorSchemeLabel->setVisible(true);
    } else {
        m_ui->titleBarOpacityInactive->setEnabled(true);
        m_ui->titleBarOpacityInactive_2->setEnabled(true);
        m_ui->titleBarOpacityInactiveFromColorSchemeLabel->setVisible(false);
    }

    if (m_ui->titleBarOpacityActive->value() != 100 || m_ui->titleBarOpacityInactive->value() != 100) {
        m_ui->opaqueMaximizedTitleBars->setEnabled(true);
        m_ui->blurTransparentTitleBars->setEnabled(true);
        m_ui->applyOpacityToHeader->setEnabled(true);
    } else {
        m_ui->opaqueMaximizedTitleBars->setEnabled(false);
        m_ui->blurTransparentTitleBars->setEnabled(false);
        m_ui->applyOpacityToHeader->setEnabled(false);
    }
}

void TitleBarOpacity::getTitlebarOpacityFromColorScheme()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    QColor activeTitlebarColor;
    QColor inactiveTitlebarColor;
    QColor activeTitleBarTextColor;
    QColor inactiveTitleBarTextColor;

    DecorationColors::readSystemTitleBarColors(config, activeTitlebarColor, inactiveTitlebarColor, activeTitleBarTextColor, inactiveTitleBarTextColor);

    Q_UNUSED(activeTitleBarTextColor);
    Q_UNUSED(inactiveTitleBarTextColor);

    bool matchTitleBarToApplicationColor = static_cast<ConfigWidget *>(m_parent)->matchTitleBarToApplicationColor();
    if (matchTitleBarToApplicationColor) {
        QPalette palette(QApplication::palette());
        activeTitlebarColor = palette.color(QPalette::ColorGroup::Active, QPalette::ColorRole::Window);
        inactiveTitlebarColor = palette.color(QPalette::ColorGroup::Inactive, QPalette::ColorRole::Window);
    }

    m_translucentActiveSchemeColor = (activeTitlebarColor.alpha() != 255);
    m_translucentInactiveSchemeColor = (inactiveTitlebarColor.alpha() != 255);
    m_activeSchemeColorAlpha = activeTitlebarColor.alphaF();
    m_inactiveSchemeColorAlpha = inactiveTitlebarColor.alphaF();
}

bool TitleBarOpacity::event(QEvent *ev)
{
    if (ev->type() == QEvent::ApplicationPaletteChange) {
        // overwrite handling of palette change
        load();
        return QWidget::event(ev);
    }

    // Make sure the rest of events are handled
    return QWidget::event(ev);
}
}
