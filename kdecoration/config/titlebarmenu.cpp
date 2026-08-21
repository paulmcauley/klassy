/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "titlebarmenu.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include <KColorScheme>
#include <QPushButton>
#include <QStandardItemModel>
#include <qnamespace.h>

namespace Breeze
{

TitleBarMenu::TitleBarMenu(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_ui(new Ui_TitleBarMenu)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    // track ui changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui->menuShows, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuPosition, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuUnisonHovering, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuReplacesApplicationMenuButton, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuEnableBlurEffect, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui->buttonShape, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonHorizontalMargin, SIGNAL(valueChanged(qreal)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonHorizontalPadding, SIGNAL(valueChanged(qreal)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonUseSystemMenuFont, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonAllowDraggingWindow, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui->searchEnabled, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->searchIgnoresDisabled, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->searchIgnoresSubMenus, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->searchIgnoresTopLevel, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);

    // Ensure certain position options can only be selected in certain show styles:

    auto onShapeChangeUpdatePositions = [this]() {
        QStandardItemModel *integratedMenuPositionModel = qobject_cast<QStandardItemModel *>(m_ui->menuPosition->model());
        QStandardItem *centerItem = integratedMenuPositionModel->item(InternalSettings::EnumIntegratedMenuPosition::Center);
        QStandardItem *centerFullWidthItem = integratedMenuPositionModel->item(InternalSettings::EnumIntegratedMenuPosition::CenterFullWidth);
        const bool isReplaceTitle = m_ui->menuShows->currentIndex() == InternalSettings::EnumIntegratedMenuShowStyle::ReplaceTitleOnHover;
        centerItem->setEnabled(isReplaceTitle);
        centerFullWidthItem->setEnabled(isReplaceTitle);
        const int currentIndex = m_ui->menuPosition->currentIndex();
        if (!isReplaceTitle
            && (currentIndex == InternalSettings::EnumIntegratedMenuPosition::Center
                || currentIndex == InternalSettings::EnumIntegratedMenuPosition::CenterFullWidth)) {
            m_ui->menuPosition->setCurrentIndex(InternalSettings::EnumIntegratedMenuPosition::Left);
        }
    };
    onShapeChangeUpdatePositions();
    connect(m_ui->menuShows, &QComboBox::currentIndexChanged, this, onShapeChangeUpdatePositions);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &TitleBarMenu::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &TitleBarMenu::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &TitleBarMenu::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

TitleBarMenu::~TitleBarMenu()
{
    delete m_ui;
}

void TitleBarMenu::loadMain(const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;
        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr(new InternalSettings());
        m_internalSettings->load();
    }

    m_ui->menuShows->setCurrentIndex(m_internalSettings->integratedMenuShowStyle());
    m_ui->menuPosition->setCurrentIndex(m_internalSettings->integratedMenuPosition());
    m_ui->menuUnisonHovering->setCurrentIndex(m_internalSettings->integratedMenuUnisonHovering());
    m_ui->menuReplacesApplicationMenuButton->setChecked(m_internalSettings->integratedMenuReplacesMenuButton());
    m_ui->menuEnableBlurEffect->setChecked(m_internalSettings->integratedMenuEnableBlur());

    m_ui->buttonShape->setCurrentIndex(m_internalSettings->integratedMenuButtonShape());
    m_ui->buttonHorizontalMargin->setValue(m_internalSettings->integratedMenuButtonHorizontalMargin());
    m_ui->buttonHorizontalPadding->setValue(m_internalSettings->integratedMenuButtonHorizontalPadding());
    m_ui->buttonUseSystemMenuFont->setChecked(m_internalSettings->integratedMenuButtonUseSystemMenuFont());
    m_ui->buttonAllowDraggingWindow->setChecked(m_internalSettings->integratedMenuButtonCanDragWindow());

    m_ui->searchEnabled->setChecked(m_internalSettings->integratedMenuSearchEnabled());
    m_ui->searchIgnoresDisabled->setChecked(m_internalSettings->integratedMenuSearchIgnoreDisabled());
    m_ui->searchIgnoresSubMenus->setChecked(m_internalSettings->integratedMenuSearchIgnoreSubMenus());
    m_ui->searchIgnoresTopLevel->setChecked(m_internalSettings->integratedMenuSearchIgnoreTopLevel());

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void TitleBarMenu::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    m_internalSettings->setIntegratedMenuShowStyle(m_ui->menuShows->currentIndex());
    m_internalSettings->setIntegratedMenuPosition(m_ui->menuPosition->currentIndex());
    m_internalSettings->setIntegratedMenuUnisonHovering(m_ui->menuUnisonHovering->currentIndex());
    m_internalSettings->setIntegratedMenuReplacesMenuButton(m_ui->menuReplacesApplicationMenuButton->isChecked());
    m_internalSettings->setIntegratedMenuEnableBlur(m_ui->menuEnableBlurEffect->isChecked());

    m_internalSettings->setIntegratedMenuButtonShape(m_ui->buttonShape->currentIndex());
    m_internalSettings->setIntegratedMenuButtonHorizontalMargin(m_ui->buttonHorizontalMargin->value());
    m_internalSettings->setIntegratedMenuButtonHorizontalPadding(m_ui->buttonHorizontalPadding->value());
    m_internalSettings->setIntegratedMenuButtonUseSystemMenuFont(m_ui->buttonUseSystemMenuFont->isChecked());
    m_internalSettings->setIntegratedMenuButtonCanDragWindow(m_ui->buttonAllowDraggingWindow->isChecked());

    m_internalSettings->setIntegratedMenuSearchEnabled(m_ui->searchEnabled->isChecked());
    m_internalSettings->setIntegratedMenuSearchIgnoreDisabled(m_ui->searchIgnoresDisabled->isChecked());
    m_internalSettings->setIntegratedMenuSearchIgnoreSubMenus(m_ui->searchIgnoresSubMenus->isChecked());
    m_internalSettings->setIntegratedMenuSearchIgnoreTopLevel(m_ui->searchIgnoresTopLevel->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        DBusMessages::updateDecorationColorCache();
        DBusMessages::kwinReloadConfig();
        // DBusMessages::kstyleReloadDecorationConfig(); //should reload anyway

        static_cast<ConfigWidget *>(m_parent)->generateSystemIcons();
    }
}

void TitleBarMenu::defaults()
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

bool TitleBarMenu::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("IntegratedMenu"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void TitleBarMenu::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    Q_EMIT changed(value);
}

void TitleBarMenu::accept()
{
    save();
    QDialog::accept();
}

void TitleBarMenu::reject()
{
    load();
    QDialog::reject();
}

void TitleBarMenu::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui->menuShows->currentIndex() != m_internalSettings->integratedMenuShowStyle())
        modified = true;
    else if (m_ui->menuPosition->currentIndex() != m_internalSettings->integratedMenuPosition())
        modified = true;
    else if (m_ui->menuUnisonHovering->currentIndex() != m_internalSettings->integratedMenuUnisonHovering())
        modified = true;
    else if (m_ui->menuReplacesApplicationMenuButton->isChecked() != m_internalSettings->integratedMenuReplacesMenuButton())
        modified = true;
    else if (m_ui->menuEnableBlurEffect->isChecked() != m_internalSettings->integratedMenuEnableBlur())
        modified = true;
    else if (m_ui->buttonShape->currentIndex() != m_internalSettings->integratedMenuButtonShape())
        modified = true;
    else if (m_ui->buttonHorizontalMargin->value() != m_internalSettings->integratedMenuButtonHorizontalMargin())
        modified = true;
    else if (m_ui->buttonHorizontalPadding->value() != m_internalSettings->integratedMenuButtonHorizontalPadding())
        modified = true;
    else if (m_ui->buttonUseSystemMenuFont->isChecked() != m_internalSettings->integratedMenuButtonUseSystemMenuFont())
        modified = true;
    else if (m_ui->buttonAllowDraggingWindow->isChecked() != m_internalSettings->integratedMenuButtonCanDragWindow())
        modified = true;
    else if (m_ui->searchEnabled->isChecked() != m_internalSettings->integratedMenuSearchEnabled())
        modified = true;
    else if (m_ui->searchIgnoresDisabled->isChecked() != m_internalSettings->integratedMenuSearchIgnoreDisabled())
        modified = true;
    else if (m_ui->searchIgnoresSubMenus->isChecked() != m_internalSettings->integratedMenuSearchIgnoreSubMenus())
        modified = true;
    else if (m_ui->searchIgnoresTopLevel->isChecked() != m_internalSettings->integratedMenuSearchIgnoreTopLevel())
        modified = true;

    setChanged(modified);
}

void TitleBarMenu::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

}
