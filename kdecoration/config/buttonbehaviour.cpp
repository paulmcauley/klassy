/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttonbehaviour.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPushButton>

namespace Breeze
{

ButtonBehaviour::ButtonBehaviour(KSharedConfig::Ptr config, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_ButtonBehaviour)
    , m_configuration(config)
{
    m_ui->setupUi(this);

    // track ui changes
    connect(m_ui->alwaysShow, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowIconHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowIconCloseButtonBackgroundHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowIconCloseButtonOutlineHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowIconOutlineHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowIconBackgroundHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowIconBackgroundOutlineHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowBackgroundHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->alwaysShowBackgroundOutlineHighlightUsing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ButtonBehaviour::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ButtonBehaviour::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ButtonBehaviour::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ButtonBehaviour::~ButtonBehaviour()
{
    delete m_ui;
}

void ButtonBehaviour::loadMain(const QString loadPreset)
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    if (loadPreset.isEmpty()) { // normal cases
        m_internalSettings->load();
    } else { // loading preset
        PresetsModel::loadPreset(m_internalSettings.data(), m_configuration.data(), loadPreset);
    }

    m_ui->alwaysShow->setCurrentIndex(m_internalSettings->alwaysShow());
    m_ui->alwaysShowIconHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconHighlightUsing());
    m_ui->alwaysShowIconCloseButtonBackgroundHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconCloseButtonBackgroundHighlightUsing());
    m_ui->alwaysShowIconCloseButtonOutlineHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconCloseButtonOutlineHighlightUsing());
    m_ui->alwaysShowIconOutlineHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconOutlineHighlightUsing());
    m_ui->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing->setCurrentIndex(
        m_internalSettings->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing());
    m_ui->alwaysShowIconBackgroundHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconBackgroundHighlightUsing());
    m_ui->alwaysShowIconBackgroundOutlineHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconBackgroundOutlineHighlightUsing());
    m_ui->alwaysShowBackgroundHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowBackgroundHighlightUsing());
    m_ui->alwaysShowBackgroundOutlineHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowBackgroundOutlineHighlightUsing());

    // TODO:add all lock variants
    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void ButtonBehaviour::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setAlwaysShow(m_ui->alwaysShow->currentIndex());
    m_internalSettings->setAlwaysShowIconHighlightUsing(m_ui->alwaysShowIconHighlightUsing->currentIndex());
    m_internalSettings->setAlwaysShowIconCloseButtonBackgroundHighlightUsing(m_ui->alwaysShowIconCloseButtonBackgroundHighlightUsing->currentIndex());
    m_internalSettings->setAlwaysShowIconCloseButtonOutlineHighlightUsing(m_ui->alwaysShowIconCloseButtonOutlineHighlightUsing->currentIndex());
    m_internalSettings->setAlwaysShowIconOutlineHighlightUsing(m_ui->alwaysShowIconOutlineHighlightUsing->currentIndex());
    m_internalSettings->setAlwaysShowIconOutlineCloseButtonBackgroundHighlightUsing(
        m_ui->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing->currentIndex());
    m_internalSettings->setAlwaysShowIconBackgroundHighlightUsing(m_ui->alwaysShowIconBackgroundHighlightUsing->currentIndex());
    m_internalSettings->setAlwaysShowIconBackgroundOutlineHighlightUsing(m_ui->alwaysShowIconBackgroundOutlineHighlightUsing->currentIndex());
    m_internalSettings->setAlwaysShowBackgroundHighlightUsing(m_ui->alwaysShowBackgroundHighlightUsing->currentIndex());
    m_internalSettings->setAlwaysShowBackgroundOutlineHighlightUsing(m_ui->alwaysShowBackgroundOutlineHighlightUsing->currentIndex());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        ConfigWidget::kwinReloadConfig();
        ConfigWidget::kstyleReloadConfig();
    }
}

void ButtonBehaviour::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui->alwaysShow->setCurrentIndex(m_internalSettings->alwaysShow());
    m_ui->alwaysShowIconHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconHighlightUsing());
    m_ui->alwaysShowIconCloseButtonBackgroundHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconCloseButtonBackgroundHighlightUsing());
    m_ui->alwaysShowIconCloseButtonOutlineHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconCloseButtonOutlineHighlightUsing());
    m_ui->alwaysShowIconOutlineHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconOutlineHighlightUsing());
    m_ui->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing->setCurrentIndex(
        m_internalSettings->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing());
    m_ui->alwaysShowIconBackgroundHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconBackgroundHighlightUsing());
    m_ui->alwaysShowIconBackgroundOutlineHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowIconBackgroundOutlineHighlightUsing());
    m_ui->alwaysShowBackgroundHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowBackgroundHighlightUsing());
    m_ui->alwaysShowBackgroundOutlineHighlightUsing->setCurrentIndex(m_internalSettings->alwaysShowBackgroundOutlineHighlightUsing());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

void ButtonBehaviour::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // Q_EMIT changed(value);
}

void ButtonBehaviour::accept()
{
    save();
    QDialog::accept();
}

void ButtonBehaviour::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    // track modifications
    bool modified(false);

    if (m_ui->alwaysShow->currentIndex() != m_internalSettings->alwaysShow())
        modified = true;
    else if (m_ui->alwaysShowIconHighlightUsing->currentIndex() != m_internalSettings->alwaysShowIconHighlightUsing())
        modified = true;
    else if (m_ui->alwaysShowIconCloseButtonBackgroundHighlightUsing->currentIndex() != m_internalSettings->alwaysShowIconCloseButtonBackgroundHighlightUsing())
        modified = true;
    else if (m_ui->alwaysShowIconCloseButtonOutlineHighlightUsing->currentIndex() != m_internalSettings->alwaysShowIconCloseButtonOutlineHighlightUsing())
        modified = true;
    else if (m_ui->alwaysShowIconOutlineHighlightUsing->currentIndex() != m_internalSettings->alwaysShowIconOutlineHighlightUsing())
        modified = true;
    else if (m_ui->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing->currentIndex()
             != m_internalSettings->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing())
        modified = true;
    else if (m_ui->alwaysShowIconBackgroundHighlightUsing->currentIndex() != m_internalSettings->alwaysShowIconBackgroundHighlightUsing())
        modified = true;
    else if (m_ui->alwaysShowIconBackgroundOutlineHighlightUsing->currentIndex() != m_internalSettings->alwaysShowIconBackgroundOutlineHighlightUsing())
        modified = true;
    else if (m_ui->alwaysShowBackgroundHighlightUsing->currentIndex() != m_internalSettings->alwaysShowBackgroundHighlightUsing())
        modified = true;
    else if (m_ui->alwaysShowBackgroundOutlineHighlightUsing->currentIndex() != m_internalSettings->alwaysShowBackgroundOutlineHighlightUsing())
        modified = true;

    setChanged(modified);
}

void ButtonBehaviour::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

}
