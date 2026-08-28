/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "titlebarappmenubar.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include <KColorScheme>
#include <QPushButton>
#include <QStandardItemModel>
#include <qnamespace.h>
#include <qobjectdefs.h>

namespace Breeze
{

TitleBarAppMenuBar::TitleBarAppMenuBar(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_ui(new Ui_TitleBarAppMenuBar)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    m_ui->menuBlurCornerRadiusIcon->setPixmap(QIcon::fromTheme(QStringLiteral("tool_curve")).pixmap(16, 16));
    m_ui->buttonCornerRadiusIcon->setPixmap(QIcon::fromTheme(QStringLiteral("tool_curve")).pixmap(16, 16));

    // track ui changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui->menuBehaviour, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuPosition, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuUnisonHovering, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuReplacesApplicationMenuButton, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuEnableBlurEffect, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuBlurCornerRadius, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->menuBlurCustomCornerRadius, SIGNAL(valueChanged(qreal)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui->buttonShape, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonHorizontalMargin, SIGNAL(valueChanged(qreal)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonHorizontalPadding, SIGNAL(valueChanged(qreal)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonCornerRadius, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonCustomCornerRadius, SIGNAL(valueChanged(qreal)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonUseSystemMenuFont, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->buttonAllowDraggingWindow, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui->searchEnabled, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->searchIgnoresDisabled, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->searchIgnoresSubMenus, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui->searchIgnoresTopLevel, SIGNAL(checkStateChanged(Qt::CheckState)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);

    // Ensure certain position options can only be selected in certain show styles:
    auto onShapeChangeUpdatePositions = [this]() {
        QStandardItemModel *appMenuBarPositionModel = qobject_cast<QStandardItemModel *>(m_ui->menuPosition->model());
        QStandardItem *centerItem = appMenuBarPositionModel->item(InternalSettings::EnumAppMenuBarPosition::Center);
        QStandardItem *centerFullWidthItem = appMenuBarPositionModel->item(InternalSettings::EnumAppMenuBarPosition::CenterFullWidth);
        const bool isReplaceTitle = m_ui->menuBehaviour->currentIndex() == InternalSettings::EnumAppMenuBarBehaviour::ReplaceTitleOnHover;
        centerItem->setEnabled(isReplaceTitle);
        centerFullWidthItem->setEnabled(isReplaceTitle);
        const int currentIndex = m_ui->menuPosition->currentIndex();
        if (!isReplaceTitle
            && (currentIndex == InternalSettings::EnumAppMenuBarPosition::Center
                || currentIndex == InternalSettings::EnumAppMenuBarPosition::CenterFullWidth)) {
            m_ui->menuPosition->setCurrentIndex(InternalSettings::EnumAppMenuBarPosition::Left);
        }
    };
    onShapeChangeUpdatePositions();
    connect(m_ui->menuBehaviour, &QComboBox::currentIndexChanged, this, onShapeChangeUpdatePositions);

    connect(m_ui->menuBlurCornerRadius, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_ui->menuBlurCustomCornerRadius->setVisible(index == InternalSettings::EnumAppMenuBarBlurCornerRadius::AMBBCR_Custom);
    });
    connect(m_ui->buttonCornerRadius, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_ui->buttonCustomCornerRadius->setVisible(index == InternalSettings::EnumAppMenuBarButtonCornerRadius::AMBCR_Custom);
    });

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &TitleBarAppMenuBar::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &TitleBarAppMenuBar::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &TitleBarAppMenuBar::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

TitleBarAppMenuBar::~TitleBarAppMenuBar()
{
    delete m_ui;
}

void TitleBarAppMenuBar::loadMain(const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;
        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr(new InternalSettings());
        m_internalSettings->load();
    }

    // Load configuration data
    m_ui->menuBehaviour->setCurrentIndex(m_internalSettings->appMenuBarBehaviour());
    m_ui->menuPosition->setCurrentIndex(m_internalSettings->appMenuBarPosition());
    m_ui->menuUnisonHovering->setCurrentIndex(m_internalSettings->appMenuBarUnisonHovering());
    m_ui->menuReplacesApplicationMenuButton->setChecked(m_internalSettings->appMenuBarReplacesMenuButton());
    m_ui->menuEnableBlurEffect->setChecked(m_internalSettings->appMenuBarEnableBlur());
    m_ui->menuBlurCornerRadius->setCurrentIndex(m_internalSettings->appMenuBarBlurCornerRadius());
    m_ui->menuBlurCustomCornerRadius->setValue(m_internalSettings->appMenuBarBlurCustomCornerRadius());

    m_ui->buttonShape->setCurrentIndex(m_internalSettings->appMenuBarButtonShape());
    m_ui->buttonHorizontalMargin->setValue(m_internalSettings->appMenuBarButtonHorizontalMargin());
    m_ui->buttonHorizontalPadding->setValue(m_internalSettings->appMenuBarButtonHorizontalPadding());
    m_ui->buttonCornerRadius->setCurrentIndex(m_internalSettings->appMenuBarButtonCornerRadius());
    m_ui->buttonCustomCornerRadius->setValue(m_internalSettings->appMenuBarButtonCustomCornerRadius());
    m_ui->buttonUseSystemMenuFont->setChecked(m_internalSettings->appMenuBarButtonUseSystemMenuFont());
    m_ui->buttonAllowDraggingWindow->setChecked(m_internalSettings->appMenuBarButtonCanDragWindow());

    m_ui->searchEnabled->setChecked(m_internalSettings->appMenuBarSearchEnabled());
    m_ui->searchIgnoresDisabled->setChecked(m_internalSettings->appMenuBarSearchIgnoreDisabled());
    m_ui->searchIgnoresSubMenus->setChecked(m_internalSettings->appMenuBarSearchIgnoreSubMenus());
    m_ui->searchIgnoresTopLevel->setChecked(m_internalSettings->appMenuBarSearchIgnoreTopLevel());

    // Set up UI
    m_ui->menuBlurCustomCornerRadius->setVisible(m_internalSettings->appMenuBarBlurCornerRadius()
                                                 == InternalSettings::EnumAppMenuBarBlurCornerRadius::AMBBCR_Custom);
    m_ui->menuBlurCornerRadiusIcon->setEnabled(m_ui->menuEnableBlurEffect->isChecked());
    m_ui->menuBlurCornerRadiusLabel->setEnabled(m_ui->menuEnableBlurEffect->isChecked());
    m_ui->menuBlurCornerRadius->setEnabled(m_ui->menuEnableBlurEffect->isChecked());
    m_ui->menuBlurCustomCornerRadius->setEnabled(m_ui->menuEnableBlurEffect->isChecked());
    m_ui->buttonCustomCornerRadius->setVisible(m_internalSettings->appMenuBarButtonCornerRadius()
                                               == InternalSettings::EnumAppMenuBarButtonCornerRadius::AMBCR_Custom);

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void TitleBarAppMenuBar::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    m_internalSettings->setAppMenuBarBehaviour(m_ui->menuBehaviour->currentIndex());
    m_internalSettings->setAppMenuBarPosition(m_ui->menuPosition->currentIndex());
    m_internalSettings->setAppMenuBarUnisonHovering(m_ui->menuUnisonHovering->currentIndex());
    m_internalSettings->setAppMenuBarReplacesMenuButton(m_ui->menuReplacesApplicationMenuButton->isChecked());
    m_internalSettings->setAppMenuBarEnableBlur(m_ui->menuEnableBlurEffect->isChecked());
    m_internalSettings->setAppMenuBarBlurCornerRadius(m_ui->menuBlurCornerRadius->currentIndex());
    m_internalSettings->setAppMenuBarBlurCustomCornerRadius(m_ui->menuBlurCustomCornerRadius->value());

    m_internalSettings->setAppMenuBarButtonShape(m_ui->buttonShape->currentIndex());
    m_internalSettings->setAppMenuBarButtonHorizontalMargin(m_ui->buttonHorizontalMargin->value());
    m_internalSettings->setAppMenuBarButtonHorizontalPadding(m_ui->buttonHorizontalPadding->value());
    m_internalSettings->setAppMenuBarButtonCornerRadius(m_ui->buttonCornerRadius->currentIndex());
    m_internalSettings->setAppMenuBarButtonCustomCornerRadius(m_ui->buttonCustomCornerRadius->value());
    m_internalSettings->setAppMenuBarButtonUseSystemMenuFont(m_ui->buttonUseSystemMenuFont->isChecked());
    m_internalSettings->setAppMenuBarButtonCanDragWindow(m_ui->buttonAllowDraggingWindow->isChecked());

    m_internalSettings->setAppMenuBarSearchEnabled(m_ui->searchEnabled->isChecked());
    m_internalSettings->setAppMenuBarSearchIgnoreDisabled(m_ui->searchIgnoresDisabled->isChecked());
    m_internalSettings->setAppMenuBarSearchIgnoreSubMenus(m_ui->searchIgnoresSubMenus->isChecked());
    m_internalSettings->setAppMenuBarSearchIgnoreTopLevel(m_ui->searchIgnoresTopLevel->isChecked());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        DBusMessages::kwinReloadConfig();
    }
}

void TitleBarAppMenuBar::defaults()
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

bool TitleBarAppMenuBar::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("TitleBarAppMenuBar"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void TitleBarAppMenuBar::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    Q_EMIT changed(value);
}

void TitleBarAppMenuBar::accept()
{
    save();
    QDialog::accept();
}

void TitleBarAppMenuBar::reject()
{
    load();
    QDialog::reject();
}

void TitleBarAppMenuBar::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui->menuBehaviour->currentIndex() != m_internalSettings->appMenuBarBehaviour())
        modified = true;
    else if (m_ui->menuPosition->currentIndex() != m_internalSettings->appMenuBarPosition())
        modified = true;
    else if (m_ui->menuUnisonHovering->currentIndex() != m_internalSettings->appMenuBarUnisonHovering())
        modified = true;
    else if (m_ui->menuReplacesApplicationMenuButton->isChecked() != m_internalSettings->appMenuBarReplacesMenuButton())
        modified = true;
    else if (m_ui->menuEnableBlurEffect->isChecked() != m_internalSettings->appMenuBarEnableBlur())
        modified = true;
    else if (m_ui->menuBlurCornerRadius->currentIndex() != m_internalSettings->appMenuBarBlurCornerRadius())
        modified = true;
    else if (m_ui->menuBlurCustomCornerRadius->value() != m_internalSettings->appMenuBarBlurCustomCornerRadius())
        modified = true;
    else if (m_ui->buttonShape->currentIndex() != m_internalSettings->appMenuBarButtonShape())
        modified = true;
    else if (m_ui->buttonHorizontalMargin->value() != m_internalSettings->appMenuBarButtonHorizontalMargin())
        modified = true;
    else if (m_ui->buttonHorizontalPadding->value() != m_internalSettings->appMenuBarButtonHorizontalPadding())
        modified = true;
    else if (m_ui->buttonCornerRadius->currentIndex() != m_internalSettings->appMenuBarButtonCornerRadius())
        modified = true;
    else if (m_ui->buttonCustomCornerRadius->value() != m_internalSettings->appMenuBarButtonCustomCornerRadius())
        modified = true;
    else if (m_ui->buttonUseSystemMenuFont->isChecked() != m_internalSettings->appMenuBarButtonUseSystemMenuFont())
        modified = true;
    else if (m_ui->buttonAllowDraggingWindow->isChecked() != m_internalSettings->appMenuBarButtonCanDragWindow())
        modified = true;
    else if (m_ui->searchEnabled->isChecked() != m_internalSettings->appMenuBarSearchEnabled())
        modified = true;
    else if (m_ui->searchIgnoresDisabled->isChecked() != m_internalSettings->appMenuBarSearchIgnoreDisabled())
        modified = true;
    else if (m_ui->searchIgnoresSubMenus->isChecked() != m_internalSettings->appMenuBarSearchIgnoreSubMenus())
        modified = true;
    else if (m_ui->searchIgnoresTopLevel->isChecked() != m_internalSettings->appMenuBarSearchIgnoreTopLevel())
        modified = true;

    setChanged(modified);
}

void TitleBarAppMenuBar::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

}
