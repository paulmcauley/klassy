/*
 * SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttonsizing.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "presetsmodel.h"

namespace Breeze
{

ButtonSizing::ButtonSizing(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui.setupUi(this);

    // track ui changes
    // direct connections are used in several places so the slot can detect the immediate m_loading status (not available in a queued connection)
    connect(m_ui.scaleBackgroundPercent, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonWidthMarginLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonWidthMarginRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonSpacingRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonSpacingRight, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.integratedRoundedRectangleBottomPadding, SIGNAL(valueChanged(double)), SLOT(updateChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.lockFullHeightButtonWidthMargins, &QAbstractButton::toggled, this, &ButtonSizing::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.lockButtonSpacingLeftRight, &QAbstractButton::toggled, this, &ButtonSizing::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.lockFullHeightButtonSpacingLeftRight, &QAbstractButton::toggled, this, &ButtonSizing::updateChanged, Qt::ConnectionType::DirectConnection);

    connect(m_ui.closeFullHeightButtonWidthMarginRelative,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &ButtonSizing::updateChanged,
            Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonCornerRadius, qOverload<int>(&QComboBox::currentIndexChanged), this, &ButtonSizing::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonCornerRadius,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &ButtonSizing::setButtonCustomCornerRadiusVisible,
            Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonCustomCornerRadius,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,
            &ButtonSizing::updateChanged,
            Qt::ConnectionType::DirectConnection);

    // connect dual controls with same values
    connect(m_ui.fullHeightButtonWidthMarginLeft,
            SIGNAL(valueChanged(int)),
            SLOT(fullHeightButtonWidthMarginLeftChanged()),
            Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonWidthMarginRight,
            SIGNAL(valueChanged(int)),
            SLOT(fullHeightButtonWidthMarginRightChanged()),
            Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(buttonSpacingLeftChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.buttonSpacingRight, SIGNAL(valueChanged(int)), SLOT(buttonSpacingRightChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonSpacingLeft, SIGNAL(valueChanged(int)), SLOT(fullHeightButtonSpacingLeftChanged()), Qt::ConnectionType::DirectConnection);
    connect(m_ui.fullHeightButtonSpacingRight, SIGNAL(valueChanged(int)), SLOT(fullHeightButtonSpacingRightChanged()), Qt::ConnectionType::DirectConnection);

    connect(m_ui.buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ButtonSizing::defaults);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ButtonSizing::load);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ButtonSizing::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ButtonSizing::~ButtonSizing()
{
}

void ButtonSizing::loadMain(const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr(new InternalSettings());
        m_internalSettings->load();
    }

    m_ui.scaleBackgroundPercent->setValue(m_internalSettings->scaleBackgroundPercent());
    m_ui.fullHeightButtonWidthMarginLeft->setValue(m_internalSettings->fullHeightButtonWidthMarginLeft());
    m_ui.fullHeightButtonWidthMarginRight->setValue(m_internalSettings->fullHeightButtonWidthMarginRight());
    m_ui.buttonSpacingRight->setValue(m_internalSettings->buttonSpacingRight());
    m_ui.buttonSpacingLeft->setValue(m_internalSettings->buttonSpacingLeft());
    m_ui.fullHeightButtonSpacingRight->setValue(m_internalSettings->fullHeightButtonSpacingRight());
    m_ui.fullHeightButtonSpacingLeft->setValue(m_internalSettings->fullHeightButtonSpacingLeft());
    m_ui.integratedRoundedRectangleBottomPadding->setValue(m_internalSettings->integratedRoundedRectangleBottomPadding());
    m_ui.lockFullHeightButtonWidthMargins->setChecked(m_internalSettings->lockFullHeightButtonWidthMargins());
    m_ui.lockButtonSpacingLeftRight->setChecked(m_internalSettings->lockButtonSpacingLeftRight());
    m_ui.lockFullHeightButtonSpacingLeftRight->setChecked(m_internalSettings->lockFullHeightButtonSpacingLeftRight());

    m_ui.closeFullHeightButtonWidthMarginRelative->setValue(m_internalSettings->closeFullHeightButtonWidthMarginRelative());
    m_ui.buttonCornerRadius->setCurrentIndex(m_internalSettings->buttonCornerRadius());
    m_ui.buttonCustomCornerRadius->setValue(m_internalSettings->buttonCustomCornerRadius());

    setVisibleUiElements(); // needs to be at the end of load to get the correct value of m_ui.buttonCornerRadius

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void ButtonSizing::setVisibleUiElements()
{
    auto closeMargins = m_ui.closeFullHeightButtonWidthMarginRelativeLayout->contentsMargins();
    auto corneredOnlyMargins = m_ui.corneredOnlyLayout->contentsMargins();

    switch (m_buttonShape) {
    case InternalSettings::EnumButtonShape::ShapeFullHeightRectangle:
    case InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle:

        setWindowTitle(i18n("Button Width & Spacing - Klassy Settings"));
        m_ui.groupBox->setTitle(i18n("Full-height Rectangle Width && Spacing"));

        m_ui.scaleBackgroundPercentLabel->setVisible(false);
        m_ui.scaleBackgroundPercent->setVisible(false);

        m_ui.fullHeightButtonWidthMarginLeftLabel->setVisible(true);
        m_ui.fullHeightButtonWidthMarginLeft->setVisible(true);
        m_ui.line->setVisible(true);
        m_ui.lockFullHeightButtonWidthMargins->setVisible(true);
        m_ui.line_2->setVisible(true);
        m_ui.fullHeightButtonWidthMarginRightLabel->setVisible(true);
        m_ui.fullHeightButtonWidthMarginRight->setVisible(true);

        m_ui.closeFullHeightButtonWidthMarginRelative->setVisible(true);
        m_ui.closeFullHeightButtonWidthMarginRelativeLabel->setVisible(true);
        m_ui.closeFullHeightButtonWidthMarginRelativeLayout->setContentsMargins(closeMargins.left(), 12, closeMargins.right(), closeMargins.bottom());
        m_ui.closeFullHeightButtonWidthMarginRelativeLayout->invalidate();

        m_ui.fullHeightButtonSpacingLeftLabel->setVisible(true);
        m_ui.fullHeightButtonSpacingLeft->setVisible(true);
        m_ui.fullHeightButtonSpacingLeftLine->setVisible(true);
        m_ui.lockFullHeightButtonSpacingLeftRight->setVisible(true);
        m_ui.fullHeightButtonSpacingRightLabel->setVisible(true);
        m_ui.fullHeightButtonSpacingRight->setVisible(true);
        m_ui.fullHeightButtonSpacingRightLine->setVisible(true);

        m_ui.buttonSpacingLeftLabel->setVisible(false);
        m_ui.buttonSpacingLeft->setVisible(false);
        m_ui.buttonSpacingLeftLine->setVisible(false);
        m_ui.lockButtonSpacingLeftRight->setVisible(false);
        m_ui.buttonSpacingRightLabel->setVisible(false);
        m_ui.buttonSpacingRight->setVisible(false);
        m_ui.buttonSpacingRightLine->setVisible(false);

        m_ui.integratedRoundedRectangleBottomPadding->setVisible(false);
        m_ui.integratedRoundedRectangleBottomPaddingLabel->setVisible(false);
        m_ui.integratedOnlySpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_ui.integratedOnlyGrid->invalidate();

        m_ui.verticalSpacer_2->changeSize(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_ui.verticalSpacer_3->changeSize(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);
        break;

    case InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle:
    case InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangleGrouped:
        setWindowTitle(i18n("Button Width & Spacing - Klassy Settings"));
        m_ui.groupBox->setTitle(i18n("Integrated Rounded Rectangle Width && Spacing"));

        m_ui.scaleBackgroundPercentLabel->setVisible(false);
        m_ui.scaleBackgroundPercent->setVisible(false);

        m_ui.fullHeightButtonWidthMarginLeftLabel->setVisible(true);
        m_ui.fullHeightButtonWidthMarginLeft->setVisible(true);
        m_ui.line->setVisible(true);
        m_ui.lockFullHeightButtonWidthMargins->setVisible(true);
        m_ui.line_2->setVisible(true);
        m_ui.fullHeightButtonWidthMarginRightLabel->setVisible(true);
        m_ui.fullHeightButtonWidthMarginRight->setVisible(true);

        m_ui.closeFullHeightButtonWidthMarginRelative->setVisible(true);
        m_ui.closeFullHeightButtonWidthMarginRelativeLabel->setVisible(true);
        m_ui.closeFullHeightButtonWidthMarginRelativeLayout->setContentsMargins(closeMargins.left(), 20, closeMargins.right(), closeMargins.bottom());
        m_ui.closeFullHeightButtonWidthMarginRelativeLayout->invalidate();

        m_ui.fullHeightButtonSpacingLeftLabel->setVisible(true);
        m_ui.fullHeightButtonSpacingLeft->setVisible(true);
        m_ui.fullHeightButtonSpacingLeftLine->setVisible(true);
        m_ui.lockFullHeightButtonSpacingLeftRight->setVisible(true);
        m_ui.fullHeightButtonSpacingRightLabel->setVisible(true);
        m_ui.fullHeightButtonSpacingRight->setVisible(true);
        m_ui.fullHeightButtonSpacingRightLine->setVisible(true);

        m_ui.buttonSpacingLeftLabel->setVisible(false);
        m_ui.buttonSpacingLeft->setVisible(false);
        m_ui.buttonSpacingLeftLine->setVisible(false);
        m_ui.lockButtonSpacingLeftRight->setVisible(false);
        m_ui.buttonSpacingRightLabel->setVisible(false);
        m_ui.buttonSpacingRight->setVisible(false);
        m_ui.buttonSpacingRightLine->setVisible(false);

        m_ui.integratedRoundedRectangleBottomPadding->setVisible(true);
        m_ui.integratedRoundedRectangleBottomPaddingLabel->setVisible(true);
        m_ui.integratedOnlySpacer->changeSize(0, 30, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        m_ui.integratedOnlyGrid->invalidate();

        m_ui.verticalSpacer_2->changeSize(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_ui.verticalSpacer_3->changeSize(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);
        break;

    default:
        setWindowTitle(i18n("Button Size & Spacing - Klassy Settings"));
        m_ui.groupBox->setTitle(i18n("Button Size && Spacing"));

        m_ui.scaleBackgroundPercentLabel->setVisible(true);
        m_ui.scaleBackgroundPercent->setVisible(true);

        m_ui.fullHeightButtonWidthMarginLeftLabel->setVisible(false);
        m_ui.fullHeightButtonWidthMarginLeft->setVisible(false);
        m_ui.line->setVisible(false);
        m_ui.lockFullHeightButtonWidthMargins->setVisible(false);
        m_ui.line_2->setVisible(false);
        m_ui.fullHeightButtonWidthMarginRightLabel->setVisible(false);
        m_ui.fullHeightButtonWidthMarginRight->setVisible(false);

        m_ui.closeFullHeightButtonWidthMarginRelative->setVisible(false);
        m_ui.closeFullHeightButtonWidthMarginRelativeLabel->setVisible(false);
        m_ui.closeFullHeightButtonWidthMarginRelativeLayout->setContentsMargins(closeMargins.left(), 0, closeMargins.right(), closeMargins.bottom());
        m_ui.closeFullHeightButtonWidthMarginRelativeLayout->invalidate();

        m_ui.fullHeightButtonSpacingLeftLabel->setVisible(false);
        m_ui.fullHeightButtonSpacingLeft->setVisible(false);
        m_ui.fullHeightButtonSpacingLeftLine->setVisible(false);
        m_ui.lockFullHeightButtonSpacingLeftRight->setVisible(false);
        m_ui.fullHeightButtonSpacingRightLabel->setVisible(false);
        m_ui.fullHeightButtonSpacingRight->setVisible(false);
        m_ui.fullHeightButtonSpacingRightLine->setVisible(false);

        m_ui.buttonSpacingLeftLabel->setVisible(true);
        m_ui.buttonSpacingLeft->setVisible(true);
        m_ui.buttonSpacingLeftLine->setVisible(true);
        m_ui.lockButtonSpacingLeftRight->setVisible(true);
        m_ui.buttonSpacingRightLabel->setVisible(true);
        m_ui.buttonSpacingRight->setVisible(true);
        m_ui.buttonSpacingRightLine->setVisible(true);

        m_ui.integratedRoundedRectangleBottomPadding->setVisible(false);
        m_ui.integratedRoundedRectangleBottomPaddingLabel->setVisible(false);
        m_ui.integratedOnlySpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_ui.integratedOnlyGrid->invalidate();

        m_ui.verticalSpacer_2->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_ui.verticalSpacer_3->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    // set corner radius options
    if (m_buttonShape == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle
        || m_buttonShape == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle
        || m_buttonShape == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangleGrouped
        || m_buttonShape == InternalSettings::EnumButtonShape::ShapeSmallRoundedSquare) {
        m_ui.cornerRadiusIcon->setPixmap(QIcon::fromTheme(QStringLiteral("tool_curve")).pixmap(16, 16));
        m_ui.cornerRadiusIcon->setVisible(true);
        m_ui.cornerRadiusLabel->setVisible(true);
        m_ui.buttonCornerRadius->setVisible(true);
        if (m_ui.buttonCornerRadius->currentIndex()) {
            m_ui.buttonCustomCornerRadius->setVisible(true);
        } else {
            m_ui.buttonCustomCornerRadius->setVisible(false);
        }
        m_ui.corneredOnlyLayout->setContentsMargins(corneredOnlyMargins.left(), 40, corneredOnlyMargins.top(), corneredOnlyMargins.right());
        m_ui.corneredOnlyLayout->invalidate();
    } else {
        m_ui.cornerRadiusIcon->setVisible(false);
        m_ui.cornerRadiusLabel->setVisible(false);
        m_ui.buttonCornerRadius->setVisible(false);
        m_ui.buttonCustomCornerRadius->setVisible(false);
        m_ui.corneredOnlyLayout->setContentsMargins(corneredOnlyMargins.left(), 0, corneredOnlyMargins.top(), corneredOnlyMargins.right());
        m_ui.corneredOnlyLayout->invalidate();
    }

    m_ui.gridLayout->invalidate();
    m_ui.groupBox->layout()->invalidate();
    adjustSize();
}

void ButtonSizing::setButtonCustomCornerRadiusVisible()
{
    bool visible = m_ui.buttonCornerRadius->currentIndex() && m_ui.buttonCornerRadius->isVisible();

    if (visible) {
        m_ui.buttonCustomCornerRadius->setVisible(true);
    } else {
        m_ui.buttonCustomCornerRadius->setVisible(false);
    }
}

void ButtonSizing::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setScaleBackgroundPercent(m_ui.scaleBackgroundPercent->value());
    m_internalSettings->setFullHeightButtonWidthMarginLeft(m_ui.fullHeightButtonWidthMarginLeft->value());
    m_internalSettings->setFullHeightButtonWidthMarginRight(m_ui.fullHeightButtonWidthMarginRight->value());
    m_internalSettings->setButtonSpacingRight(m_ui.buttonSpacingRight->value());
    m_internalSettings->setButtonSpacingLeft(m_ui.buttonSpacingLeft->value());
    m_internalSettings->setFullHeightButtonSpacingRight(m_ui.fullHeightButtonSpacingRight->value());
    m_internalSettings->setFullHeightButtonSpacingLeft(m_ui.fullHeightButtonSpacingLeft->value());
    m_internalSettings->setIntegratedRoundedRectangleBottomPadding(m_ui.integratedRoundedRectangleBottomPadding->value());
    m_internalSettings->setLockFullHeightButtonWidthMargins(m_ui.lockFullHeightButtonWidthMargins->isChecked());
    m_internalSettings->setLockButtonSpacingLeftRight(m_ui.lockButtonSpacingLeftRight->isChecked());
    m_internalSettings->setLockFullHeightButtonSpacingLeftRight(m_ui.lockFullHeightButtonSpacingLeftRight->isChecked());

    m_internalSettings->setCloseFullHeightButtonWidthMarginRelative(m_ui.closeFullHeightButtonWidthMarginRelative->value());
    m_internalSettings->setButtonCornerRadius(m_ui.buttonCornerRadius->currentIndex());
    m_internalSettings->setButtonCustomCornerRadius(m_ui.buttonCustomCornerRadius->value());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        DBusMessages::kwinReloadConfig();

        static_cast<ConfigWidget *>(m_parent)->generateSystemIcons();
    }
}

void ButtonSizing::defaults()
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

bool ButtonSizing::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("ButtonSizing"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void ButtonSizing::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    Q_EMIT changed(value);
}

void ButtonSizing::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui.scaleBackgroundPercent->value() != m_internalSettings->scaleBackgroundPercent())
        modified = true;
    else if (m_ui.fullHeightButtonWidthMarginLeft->value() != m_internalSettings->fullHeightButtonWidthMarginLeft())
        modified = true;
    else if (m_ui.fullHeightButtonWidthMarginRight->value() != m_internalSettings->fullHeightButtonWidthMarginRight())
        modified = true;
    else if (m_ui.buttonSpacingRight->value() != m_internalSettings->buttonSpacingRight())
        modified = true;
    else if (m_ui.buttonSpacingLeft->value() != m_internalSettings->buttonSpacingLeft())
        modified = true;
    else if (m_ui.fullHeightButtonSpacingRight->value() != m_internalSettings->fullHeightButtonSpacingRight())
        modified = true;
    else if (m_ui.fullHeightButtonSpacingLeft->value() != m_internalSettings->fullHeightButtonSpacingLeft())
        modified = true;
    else if (m_ui.integratedRoundedRectangleBottomPadding->value() != m_internalSettings->integratedRoundedRectangleBottomPadding())
        modified = true;
    else if (m_ui.lockFullHeightButtonWidthMargins->isChecked() != m_internalSettings->lockFullHeightButtonWidthMargins())
        modified = true;
    else if (m_ui.lockButtonSpacingLeftRight->isChecked() != m_internalSettings->lockButtonSpacingLeftRight())
        modified = true;
    else if (m_ui.lockFullHeightButtonSpacingLeftRight->isChecked() != m_internalSettings->lockFullHeightButtonSpacingLeftRight())
        modified = true;

    else if (m_ui.closeFullHeightButtonWidthMarginRelative->value() != m_internalSettings->closeFullHeightButtonWidthMarginRelative())
        modified = true;
    else if (m_ui.buttonCornerRadius->currentIndex() != m_internalSettings->buttonCornerRadius())
        modified = true;
    else if (qAbs(m_ui.buttonCustomCornerRadius->value() - m_internalSettings->buttonCustomCornerRadius()) > 0.001)
        modified = true;

    setChanged(modified);
}

void ButtonSizing::accept()
{
    save();
    QDialog::accept();
}

void ButtonSizing::reject()
{
    load();
    QDialog::reject();
}

void ButtonSizing::setApplyButtonState(const bool on)
{
    m_ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void ButtonSizing::fullHeightButtonWidthMarginLeftChanged()
{
    if (m_ui.lockFullHeightButtonWidthMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonWidthMarginRight->setValue(m_ui.fullHeightButtonWidthMarginLeft->value());
}

void ButtonSizing::fullHeightButtonWidthMarginRightChanged()
{
    if (m_ui.lockFullHeightButtonWidthMargins->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonWidthMarginLeft->setValue(m_ui.fullHeightButtonWidthMarginRight->value());
}

void ButtonSizing::buttonSpacingLeftChanged()
{
    if (m_ui.lockButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.buttonSpacingRight->setValue(m_ui.buttonSpacingLeft->value());
}

void ButtonSizing::buttonSpacingRightChanged()
{
    if (m_ui.lockButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.buttonSpacingLeft->setValue(m_ui.buttonSpacingRight->value());
}

void ButtonSizing::fullHeightButtonSpacingLeftChanged()
{
    if (m_ui.lockFullHeightButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonSpacingRight->setValue(m_ui.fullHeightButtonSpacingLeft->value());
}

void ButtonSizing::fullHeightButtonSpacingRightChanged()
{
    if (m_ui.lockFullHeightButtonSpacingLeftRight->isChecked() && !m_processingDefaults && !m_loading)
        m_ui.fullHeightButtonSpacingLeft->setValue(m_ui.fullHeightButtonSpacingRight->value());
}

void ButtonSizing::updateLockIcons()
{
    m_ui.lockFullHeightButtonWidthMargins->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui.lockButtonSpacingLeftRight->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui.lockFullHeightButtonSpacingLeftRight->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
}
}
