/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttoncolors.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include "renderdecorationbuttonicon.h"
#include "systemicontheme.h"
#include <KColorUtils>
#include <KDecoration2/DecorationSettings>
#include <QCheckBox>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QHBoxLayout>
#include <QMutableListIterator>
#include <QPushButton>
#include <QSlider>
#include <QWindow>
#include <memory>

namespace Breeze
{

using KDecoration2::DecorationButtonType;

ButtonColors::ButtonColors(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_ButtonColors)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    getButtonsOrderFromKwinConfig();

    this->resize(755, 400);
    m_ui->activeOverrideGroupBox->setVisible(false);
    m_ui->inactiveOverrideGroupBox->setVisible(false);
    connect(m_ui->buttonColorOverrideToggleActive, &QAbstractButton::toggled, this, &ButtonColors::showActiveOverrideGroupBox);
    connect(m_ui->buttonColorOverrideToggleInactive, &QAbstractButton::toggled, this, &ButtonColors::showInactiveOverrideGroupBox);
    connect(m_ui->buttonColorOverrideToggleActive, &QAbstractButton::clicked, this, &ButtonColors::resizeDialog);
    connect(m_ui->buttonColorOverrideToggleInactive, &QAbstractButton::clicked, this, &ButtonColors::resizeDialog);

    generateTableCells(m_ui->overrideColorTableActive);
    generateTableCells(m_ui->overrideColorTableInactive);

    // populate the horizontal header
    QStringList orderedHorizontalHeaderLabels;
    for (auto i = 0; i < m_allCustomizableButtonsOrder.count(); i++) { // get the horizontal header labels in the correct user-set order
        orderedHorizontalHeaderLabels.append(m_colorOverridableButtonTypesStrings.value(m_allCustomizableButtonsOrder[i]));
    }
    m_ui->overrideColorTableActive->setHorizontalHeaderLabels(orderedHorizontalHeaderLabels);
    m_ui->overrideColorTableInactive->setHorizontalHeaderLabels(orderedHorizontalHeaderLabels);
    m_ui->overrideColorTableActive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->overrideColorTableInactive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->overrideColorTableActive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->overrideColorTableInactive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    connect(m_ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(loadButtonPaletteColorsIcons()));

    // lockButtonColorsActive and lockButtonColorsInactive are set to mirror each other in the UI file
    connect(m_ui->lockButtonColorsActive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->lockButtonColorsInactive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);

    auto setIndexOfOtherIfLocked = [this](QComboBox *other, const int i) {
        if (m_loading || m_processingDefaults || !m_ui->lockButtonColorsActive->isChecked())
            return; // only do this for user interactions where the active/inactive lock is checked
        other->setCurrentIndex(i);
    };

    // track ui changes
    connect(m_ui->buttonIconColorsActive,
            SIGNAL(currentIndexChanged(int)),
            SLOT(refreshCloseButtonIconColorStateActive())); // important that refreshCloseButtonIconColorState is before updateChanged
    connect(m_ui->buttonIconColorsActive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->buttonIconColorsActive, qOverload<int>(&QComboBox::currentIndexChanged), [=](const int i) {
        setIndexOfOtherIfLocked(m_ui->buttonIconColorsInactive, i);
    });

    connect(m_ui->buttonIconColorsInactive,
            SIGNAL(currentIndexChanged(int)),
            SLOT(refreshCloseButtonIconColorStateInactive())); // important that refreshCloseButtonIconColorState is before updateChanged
    connect(m_ui->buttonIconColorsInactive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->buttonIconColorsInactive, qOverload<int>(&QComboBox::currentIndexChanged), [=](const int i) {
        setIndexOfOtherIfLocked(m_ui->buttonIconColorsActive, i);
    });

    connect(m_ui->buttonBackgroundColorsActive, SIGNAL(currentIndexChanged(int)), SLOT(setNegativeCloseBackgroundHoverPressStateActive()));
    connect(m_ui->buttonBackgroundColorsActive, SIGNAL(currentIndexChanged(int)), SLOT(refreshCloseButtonIconColorStateActive()));
    connect(m_ui->buttonBackgroundColorsActive, qOverload<int>(&QComboBox::currentIndexChanged), [=](const int i) {
        setIndexOfOtherIfLocked(m_ui->buttonBackgroundColorsInactive, i);
    });
    connect(m_ui->buttonBackgroundColorsActive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));

    connect(m_ui->buttonBackgroundColorsInactive, SIGNAL(currentIndexChanged(int)), SLOT(setNegativeCloseBackgroundHoverPressStateInactive()));
    connect(m_ui->buttonBackgroundColorsInactive, SIGNAL(currentIndexChanged(int)), SLOT(refreshCloseButtonIconColorStateInactive()));
    connect(m_ui->buttonBackgroundColorsInactive, qOverload<int>(&QComboBox::currentIndexChanged), [=](const int i) {
        setIndexOfOtherIfLocked(m_ui->buttonBackgroundColorsActive, i);
    });
    connect(m_ui->buttonBackgroundColorsInactive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));

    connect(m_ui->closeButtonIconColorActive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->closeButtonIconColorActive, qOverload<int>(&QComboBox::currentIndexChanged), [=](const int i) {
        setIndexOfOtherIfLocked(m_ui->closeButtonIconColorInactive, i);
    });

    connect(m_ui->closeButtonIconColorInactive, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui->closeButtonIconColorInactive, qOverload<int>(&QComboBox::currentIndexChanged), [=](const int i) {
        setIndexOfOtherIfLocked(m_ui->closeButtonIconColorActive, i);
    });

    auto setOtherCheckedIfLocked = [this](QAbstractButton *other, const bool v) {
        if (m_loading || m_processingDefaults || !m_ui->lockButtonColorsActive->isChecked())
            return; // only do this for user interactions where the active/inactive lock is checked
        other->setChecked(v);
    };

    connect(m_ui->translucentButtonBackgroundsActive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->translucentButtonBackgroundsActive, &QAbstractButton::toggled, this, &ButtonColors::showHideTranslucencySettingsActive);
    connect(m_ui->translucentButtonBackgroundsActive, &QAbstractButton::toggled, [=](const bool v) {
        setOtherCheckedIfLocked(m_ui->translucentButtonBackgroundsInactive, v);
    });

    connect(m_ui->translucentButtonBackgroundsInactive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->translucentButtonBackgroundsInactive, &QAbstractButton::toggled, this, &ButtonColors::showHideTranslucencySettingsInactive);
    connect(m_ui->translucentButtonBackgroundsInactive, &QAbstractButton::toggled, [=](const bool v) {
        setOtherCheckedIfLocked(m_ui->translucentButtonBackgroundsActive, v);
    });

    connect(m_ui->negativeCloseBackgroundHoverPressActive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->negativeCloseBackgroundHoverPressActive, &QAbstractButton::toggled, [=](const bool v) {
        setOtherCheckedIfLocked(m_ui->negativeCloseBackgroundHoverPressInactive, v);
    });

    connect(m_ui->negativeCloseBackgroundHoverPressInactive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->negativeCloseBackgroundHoverPressInactive, &QAbstractButton::toggled, [=](const bool v) {
        setOtherCheckedIfLocked(m_ui->negativeCloseBackgroundHoverPressActive, v);
    });

    connect(m_ui->blackWhiteIconOnPoorContrastActive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->blackWhiteIconOnPoorContrastActive, &QAbstractButton::toggled, [=](const bool v) {
        setOtherCheckedIfLocked(m_ui->blackWhiteIconOnPoorContrastInactive, v);
    });

    connect(m_ui->blackWhiteIconOnPoorContrastInactive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->blackWhiteIconOnPoorContrastInactive, &QAbstractButton::toggled, [=](const bool v) {
        setOtherCheckedIfLocked(m_ui->blackWhiteIconOnPoorContrastActive, v);
    });

    connect(m_ui->adjustBackgroundColorOnPoorContrastActive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->adjustBackgroundColorOnPoorContrastActive, &QAbstractButton::toggled, [=](const bool v) {
        setOtherCheckedIfLocked(m_ui->adjustBackgroundColorOnPoorContrastInactive, v);
    });

    connect(m_ui->adjustBackgroundColorOnPoorContrastInactive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->adjustBackgroundColorOnPoorContrastInactive, &QAbstractButton::toggled, [=](const bool v) {
        setOtherCheckedIfLocked(m_ui->adjustBackgroundColorOnPoorContrastActive, v);
    });

    auto setOtherValueIfLocked = [this](QSpinBox *other, const int v) {
        if (m_loading || m_processingDefaults || !m_ui->lockButtonColorsActive->isChecked())
            return; // only do this for user interactions where the active/inactive lock is checked
        other->setValue(v);
    };

    connect(m_ui->translucentButtonBackgroundsOpacityActive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->translucentButtonBackgroundsOpacityActive, qOverload<int>(&QSpinBox::valueChanged), [=](const int v) {
        setOtherValueIfLocked(m_ui->translucentButtonBackgroundsOpacityInactive, v);
    });

    connect(m_ui->translucentButtonBackgroundsOpacityInactive, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui->translucentButtonBackgroundsOpacityInactive, qOverload<int>(&QSpinBox::valueChanged), [=](const int v) {
        setOtherValueIfLocked(m_ui->translucentButtonBackgroundsOpacityActive, v);
    });

    connect(m_ui->buttonColorOverrideToggleActive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
    connect(m_ui->buttonColorOverrideToggleInactive, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ButtonColors::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ButtonColors::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ButtonColors::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

void ButtonColors::generateTableCells(QTableWidget *table)
{
    int numRows = m_overridableButtonColorStatesStrings.count();

    // generate the vertical header
    for (int rowIndex = 0; rowIndex < numRows; rowIndex++) {
        QTableWidgetItem *verticalHeaderItem = new QTableWidgetItem();
        verticalHeaderItem->setText(m_overridableButtonColorStatesStrings[rowIndex]);
        verticalHeaderItem->setToolTip(i18n("Lock to make all colours in this row the same"));
        verticalHeaderItem->setData(Qt::InitialSortOrderRole, rowIndex);
        table->insertRow(rowIndex);
        table->setVerticalHeaderItem(rowIndex, verticalHeaderItem);
    }

    connect(table->verticalHeader(), &QHeaderView::sectionClicked, this, &ButtonColors::tableVerticalHeaderSectionClicked);
    connect(table->verticalHeader(), &QHeaderView::sectionClicked, this, &ButtonColors::updateChanged);

    // gnerate the overrideColorTable UI
    // populate the checkboxes and KColorButtons
    for (int columnIndex = 0; columnIndex < m_colorOverridableButtonTypesStrings.count(); columnIndex++) {
        table->insertColumn(columnIndex);
        for (int rowIndex = 0; rowIndex < numRows; rowIndex++) {
            QHBoxLayout *hlayout = new QHBoxLayout();
            hlayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
            QCheckBox *checkBox = new QCheckBox();
            checkBox->setProperty("column", columnIndex);
            checkBox->setProperty("row", rowIndex);
            hlayout->addWidget(checkBox);
            KColorButton *colorButton = new KColorButton();
            colorButton->setProperty("column", columnIndex);
            colorButton->setProperty("row", rowIndex);
            colorButton->setAlphaChannelEnabled(true);
            connect(colorButton, &KColorButton::changed, this, &ButtonColors::updateChanged);
            colorButton->setVisible(false);
            hlayout->addWidget(colorButton);
            hlayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
            QWidget *w = new QWidget();
            w->setLayout(hlayout);
            table->setCellWidget(rowIndex, columnIndex, w);
            connect(checkBox, &QAbstractButton::toggled, colorButton, &QWidget::setVisible);
            connect(checkBox, &QAbstractButton::toggled, this, &ButtonColors::updateChanged);
            connect(checkBox, &QAbstractButton::toggled, this, &ButtonColors::resizeOverrideColorTable);
            connect(checkBox, &QAbstractButton::toggled, this, &ButtonColors::copyCellCheckedStatusToOtherTable);
            connect(colorButton, &KColorButton::changed, this, &ButtonColors::copyCellColorDataToOtherCells);
        }
    }
}

ButtonColors::~ButtonColors()
{
    delete m_ui;
}

void ButtonColors::loadMain(const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr(new InternalSettings());
        m_internalSettings->load();
    }

    m_ui->buttonIconColorsActive->setCurrentIndex(m_internalSettings->buttonIconColors(true));
    m_ui->buttonIconColorsInactive->setCurrentIndex(m_internalSettings->buttonIconColors(false));
    m_ui->buttonBackgroundColorsActive->setCurrentIndex(m_internalSettings->buttonBackgroundColors(true));
    m_ui->buttonBackgroundColorsInactive->setCurrentIndex(m_internalSettings->buttonBackgroundColors(false));
    m_ui->translucentButtonBackgroundsActive->setChecked(m_internalSettings->translucentButtonBackgrounds(true));
    m_ui->translucentButtonBackgroundsInactive->setChecked(m_internalSettings->translucentButtonBackgrounds(false));
    m_ui->negativeCloseBackgroundHoverPressActive->setChecked(m_internalSettings->negativeCloseBackgroundHoverPress(true));
    m_ui->negativeCloseBackgroundHoverPressInactive->setChecked(m_internalSettings->negativeCloseBackgroundHoverPress(false));
    m_ui->lockButtonColorsActive->setChecked(m_internalSettings->lockButtonColorsActiveInactive());
    m_ui->lockButtonColorsInactive->setChecked(m_internalSettings->lockButtonColorsActiveInactive());
    m_ui->blackWhiteIconOnPoorContrastActive->setChecked(m_internalSettings->blackWhiteIconOnPoorContrast(true));
    m_ui->blackWhiteIconOnPoorContrastInactive->setChecked(m_internalSettings->blackWhiteIconOnPoorContrast(false));
    m_ui->adjustBackgroundColorOnPoorContrastActive->setChecked(m_internalSettings->adjustBackgroundColorOnPoorContrast(true));
    m_ui->adjustBackgroundColorOnPoorContrastInactive->setChecked(m_internalSettings->adjustBackgroundColorOnPoorContrast(false));
    m_ui->translucentButtonBackgroundsOpacityActive->setValue(m_internalSettings->translucentButtonBackgroundsOpacity(true) * 100);
    m_ui->translucentButtonBackgroundsOpacityInactive->setValue(m_internalSettings->translucentButtonBackgroundsOpacity(false) * 100);

    setNegativeCloseBackgroundHoverPressStateActive();
    setNegativeCloseBackgroundHoverPressStateInactive();
    refreshCloseButtonIconColorStateActive();
    refreshCloseButtonIconColorStateInactive();
    loadCloseButtonIconColor(true); // refreshCloseButtonIconColorState must occur before loading closeButtonIconColor
    loadCloseButtonIconColor(false);
    showHideTranslucencySettings(true);
    showHideTranslucencySettings(false);
    decodeColorOverridableLockStatesAndLoadVerticalHeaderLocks();

    loadHorizontalHeaderIcons();
    // set the header widths to the same as the largest section
    int largestSection = 0;
    for (int columnIndex = 0; columnIndex < m_colorOverridableButtonTypesStrings.count(); columnIndex++) {
        int sectionSize = m_ui->overrideColorTableActive->horizontalHeader()->sectionSize(columnIndex);
        if (sectionSize > largestSection)
            largestSection = sectionSize;
    }
    m_ui->overrideColorTableActive->horizontalHeader()->setMinimumSectionSize(largestSection);
    m_ui->overrideColorTableInactive->horizontalHeader()->setMinimumSectionSize(largestSection);

    // load the override table cell data
    m_overrideColorsLoaded = {false, false};
    ColumnsLoaded overrideColorColumnLoaded = {false, false};

    for (int i = 0; i < InternalSettings::EnumButtonOverrideColorsButtonType::COUNT; i++) {
        overrideColorColumnLoaded = decodeColorsFlagsAndLoadColumn(m_ui->overrideColorTableActive,
                                                                   m_ui->overrideColorTableInactive,
                                                                   m_allCustomizableButtonsOrder.indexOf(static_cast<KDecoration2::DecorationButtonType>(i)),
                                                                   m_internalSettings->buttonOverrideColorsFlags(i),
                                                                   m_internalSettings->buttonOverrideColors(i));
        m_overrideColorsLoaded.active = m_overrideColorsLoaded.active || overrideColorColumnLoaded.active;
        m_overrideColorsLoaded.inactive = m_overrideColorsLoaded.inactive || overrideColorColumnLoaded.inactive;
    }

    m_ui->buttonColorOverrideToggleActive->setChecked(m_overrideColorsLoaded.active);
    m_ui->buttonColorOverrideToggleInactive->setChecked(m_overrideColorsLoaded.inactive);

    // add palette preview icons
    loadButtonPaletteColorsIcons();

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void ButtonColors::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setButtonIconColors(true, m_ui->buttonIconColorsActive->currentIndex());
    m_internalSettings->setButtonIconColors(false, m_ui->buttonIconColorsInactive->currentIndex());
    m_internalSettings->setCloseButtonIconColor(true, convertCloseButtonIconColorUiToSettingsIndex(true, m_ui->closeButtonIconColorActive->currentIndex()));
    m_internalSettings->setCloseButtonIconColor(false, convertCloseButtonIconColorUiToSettingsIndex(false, m_ui->closeButtonIconColorInactive->currentIndex()));
    m_internalSettings->setButtonBackgroundColors(true, m_ui->buttonBackgroundColorsActive->currentIndex());
    m_internalSettings->setButtonBackgroundColors(false, m_ui->buttonBackgroundColorsInactive->currentIndex());
    m_internalSettings->setTranslucentButtonBackgrounds(true, m_ui->translucentButtonBackgroundsActive->isChecked());
    m_internalSettings->setTranslucentButtonBackgrounds(false, m_ui->translucentButtonBackgroundsInactive->isChecked());
    m_internalSettings->setNegativeCloseBackgroundHoverPress(true, m_ui->negativeCloseBackgroundHoverPressActive->isChecked());
    m_internalSettings->setNegativeCloseBackgroundHoverPress(false, m_ui->negativeCloseBackgroundHoverPressInactive->isChecked());
    m_internalSettings->setLockButtonColorsActiveInactive(m_ui->lockButtonColorsActive->isChecked());
    m_internalSettings->setBlackWhiteIconOnPoorContrast(true, m_ui->blackWhiteIconOnPoorContrastActive->isChecked());
    m_internalSettings->setBlackWhiteIconOnPoorContrast(false, m_ui->blackWhiteIconOnPoorContrastInactive->isChecked());
    m_internalSettings->setAdjustBackgroundColorOnPoorContrast(true, m_ui->adjustBackgroundColorOnPoorContrastActive->isChecked());
    m_internalSettings->setAdjustBackgroundColorOnPoorContrast(false, m_ui->adjustBackgroundColorOnPoorContrastInactive->isChecked());
    m_internalSettings->setTranslucentButtonBackgroundsOpacity(true, m_ui->translucentButtonBackgroundsOpacityActive->value() / 100.0f);
    m_internalSettings->setTranslucentButtonBackgroundsOpacity(false, m_ui->translucentButtonBackgroundsOpacityInactive->value() / 100.0f);

    if (m_ui->buttonColorOverrideToggleActive->isChecked() || m_ui->buttonColorOverrideToggleInactive->isChecked())
        m_internalSettings->setButtonOverrideColorsLockStates(encodeColorOverridableLockStates());
    else
        m_internalSettings->setButtonOverrideColorsLockStates(0);

    // save the override table cell data
    for (int i = 0; i < InternalSettings::EnumButtonOverrideColorsButtonType::COUNT; i++) {
        QList<int> colorsList;
        uint32_t colorsFlags = 0;

        bool resetActive = !m_ui->buttonColorOverrideToggleActive->isChecked();
        bool resetInactive = !m_ui->buttonColorOverrideToggleInactive->isChecked();

        encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                               m_ui->overrideColorTableInactive,
                                               m_allCustomizableButtonsOrder.indexOf(static_cast<KDecoration2::DecorationButtonType>(i)),
                                               colorsFlags,
                                               colorsList,
                                               resetActive,
                                               resetInactive);
        m_internalSettings->setButtonOverrideColorsFlags(i, colorsFlags);
        m_internalSettings->setButtonOverrideColors(i, colorsList);
    }

    m_internalSettings->save();

    load(); // need to re-load in the case where m_ui->buttonColorActiveOverrideToggle is unchecked

    setChanged(false);

    if (reloadKwinConfig) {
        ConfigWidget::kwinReloadConfig();
        ConfigWidget::kstyleReloadConfig();
    }
}

void ButtonColors::defaults()
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

bool ButtonColors::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("ButtonColors"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void ButtonColors::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    // Q_EMIT changed(value);
}

void ButtonColors::accept()
{
    save();
    QDialog::accept();
}

void ButtonColors::reject()
{
    load();
    QDialog::reject();
}

void ButtonColors::updateChanged()
{
    // update the displayed colours any time the UI settings change
    loadButtonPaletteColorsIcons();

    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);
    if (m_ui->buttonIconColorsActive->currentIndex() != m_internalSettings->buttonIconColors(true))
        modified = true;
    else if (m_ui->buttonIconColorsInactive->currentIndex() != m_internalSettings->buttonIconColors(false))
        modified = true;
    else if (convertCloseButtonIconColorUiToSettingsIndex(true, m_ui->closeButtonIconColorActive->currentIndex())
             != m_internalSettings->closeButtonIconColor(true))
        modified = true;
    else if (convertCloseButtonIconColorUiToSettingsIndex(false, m_ui->closeButtonIconColorInactive->currentIndex())
             != m_internalSettings->closeButtonIconColor(false))
        modified = true;
    else if (m_ui->buttonBackgroundColorsActive->currentIndex() != m_internalSettings->buttonBackgroundColors(true))
        modified = true;
    else if (m_ui->buttonBackgroundColorsInactive->currentIndex() != m_internalSettings->buttonBackgroundColors(false))
        modified = true;
    else if (m_ui->translucentButtonBackgroundsActive->isChecked() != m_internalSettings->translucentButtonBackgrounds(true))
        modified = true;
    else if (m_ui->translucentButtonBackgroundsInactive->isChecked() != m_internalSettings->translucentButtonBackgrounds(false))
        modified = true;
    else if (m_ui->negativeCloseBackgroundHoverPressActive->isChecked() != m_internalSettings->negativeCloseBackgroundHoverPress(true))
        modified = true;
    else if (m_ui->negativeCloseBackgroundHoverPressInactive->isChecked() != m_internalSettings->negativeCloseBackgroundHoverPress(false))
        modified = true;
    else if (m_ui->lockButtonColorsActive->isChecked() != m_internalSettings->lockButtonColorsActiveInactive())
        modified = true;
    else if (m_ui->blackWhiteIconOnPoorContrastActive->isChecked() != m_internalSettings->blackWhiteIconOnPoorContrast(true))
        modified = true;
    else if (m_ui->blackWhiteIconOnPoorContrastInactive->isChecked() != m_internalSettings->blackWhiteIconOnPoorContrast(false))
        modified = true;
    else if (m_ui->adjustBackgroundColorOnPoorContrastActive->isChecked() != m_internalSettings->adjustBackgroundColorOnPoorContrast(true))
        modified = true;
    else if (m_ui->adjustBackgroundColorOnPoorContrastInactive->isChecked() != m_internalSettings->adjustBackgroundColorOnPoorContrast(false))
        modified = true;
    else if (!(qAbs(m_ui->translucentButtonBackgroundsOpacityActive->value() - (100 * m_internalSettings->translucentButtonBackgroundsOpacity(true))) < 0.001))
        modified = true;
    else if (!(qAbs(m_ui->translucentButtonBackgroundsOpacityInactive->value() - (100 * m_internalSettings->translucentButtonBackgroundsOpacity(false)))
               < 0.001))
        modified = true;
    else if (encodeColorOverridableLockStates() != m_internalSettings->buttonOverrideColorsLockStates())
        modified = true;

    if (!modified) {
        if (encodeColorOverridableLockStates() != m_internalSettings->buttonOverrideColorsLockStates())
            modified = true;
    }

    uint32_t colorsFlags;
    QList<int> colorsList;

    if (!modified) {
        bool resetActive = !m_ui->buttonColorOverrideToggleActive->isChecked();
        bool resetInactive = !m_ui->buttonColorOverrideToggleInactive->isChecked();
        for (int i = 0; i < InternalSettings::EnumButtonOverrideColorsButtonType::COUNT; i++) {
            encodeColorOverridableButtonTypeColumn(m_ui->overrideColorTableActive,
                                                   m_ui->overrideColorTableInactive,
                                                   m_allCustomizableButtonsOrder.indexOf(static_cast<KDecoration2::DecorationButtonType>(i)),
                                                   colorsFlags,
                                                   colorsList,
                                                   resetActive,
                                                   resetInactive);
            modified = modified || (colorsFlags != m_internalSettings->buttonOverrideColorsFlags(i));
            modified = modified || (colorsList != m_internalSettings->buttonOverrideColors(i));
        }

    } else if (!modified && m_overrideColorsLoaded.active && !m_ui->buttonColorOverrideToggleActive->isChecked()) {
        modified = true;
    } else if (!modified && m_overrideColorsLoaded.inactive && !m_ui->buttonColorOverrideToggleInactive->isChecked()) {
        modified = true;
    }

    setChanged(modified);
}

void ButtonColors::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void ButtonColors::refreshCloseButtonIconColorState(bool active)
{
    // As selected
    // Negative when hovered/pressed
    // White
    // White when hovered/pressed
    /*
        enum struct CloseButtonIconColorState{
            AsSelected = 1,
            NegativeWhenHoveredPressed = 2,
            White = 4,
            WhiteWhenHoveredPressed = 8
        }

    */

    QComboBox *buttonBackgroundColors = active ? m_ui->buttonBackgroundColorsActive : m_ui->buttonBackgroundColorsInactive;
    QComboBox *buttonIconColors = active ? m_ui->buttonIconColorsActive : m_ui->buttonIconColorsInactive;
    QComboBox *closeButtonIconColor = active ? m_ui->closeButtonIconColorActive : m_ui->closeButtonIconColorInactive;
    QLabel *closeButtonIconColorLabel = active ? m_ui->closeButtonIconColorLabelActive : m_ui->closeButtonIconColorLabelInactive;

    uint32_t closeButtonIconColorState = static_cast<uint32_t>(CloseButtonIconColorState::AsSelected);
    bool visible = false;
    QString negativeWhenHoveredPressedString;
    QString whiteString;
    QString WhiteWhenHoveredPressedString;

    bool negativeCloseBackground = buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
        || buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights;

    if (buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
        closeButtonIconColorLabel->setText(i18n("Traffic lights icon colours:"));
    } else {
        closeButtonIconColorLabel->setText(i18n("Close icon colour:"));
    }

    if (negativeCloseBackground) {
        visible = true;
        closeButtonIconColorState = closeButtonIconColorState | static_cast<uint32_t>(CloseButtonIconColorState::White)
            | static_cast<uint32_t>(CloseButtonIconColorState::WhiteWhenHoveredPressed);
        if (buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            whiteString = i18n("White close");
            WhiteWhenHoveredPressedString = i18n("White close when hovered/pressed");
        } else {
            whiteString = i18n("White");
            WhiteWhenHoveredPressedString = i18n("White when hovered/pressed");
        }
    }

    if (buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentNegativeClose
        || buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
        || buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
        visible = true;
        closeButtonIconColorState = closeButtonIconColorState | static_cast<uint32_t>(CloseButtonIconColorState::NegativeWhenHoveredPressed);

        if (buttonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            negativeWhenHoveredPressedString = i18n("Traffic lights when hovered/pressed");
        } else {
            negativeWhenHoveredPressedString = i18n("Negative when hovered/pressed");
        }
    }

    QString closeIconAsSelectedString;
    switch (buttonIconColors->currentIndex()) {
    case InternalSettings::EnumButtonIconColors::TitlebarText:
        closeIconAsSelectedString = i18n("Titlebar text");
        break;
    case InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose:
        closeIconAsSelectedString = i18n("Negative");
        break;
    case InternalSettings::EnumButtonIconColors::Accent:
        closeIconAsSelectedString = i18n("Accent");
        break;
    case InternalSettings::EnumButtonIconColors::AccentNegativeClose:
        closeIconAsSelectedString = i18n("Negative");
        break;
    case InternalSettings::EnumButtonIconColors::AccentTrafficLights:
        closeIconAsSelectedString = i18n("Traffic lights");
        break;
    }

    // remove all items from the closeButtonIconColor combobox
    int count = closeButtonIconColor->count();
    for (int i = 0; i < count; i++) {
        closeButtonIconColor->removeItem(0);
    }

    // add each item, depending on the state
    if (static_cast<uint32_t>(CloseButtonIconColorState::AsSelected) & closeButtonIconColorState) {
        closeButtonIconColor->addItem(closeIconAsSelectedString);
    }

    if (static_cast<uint32_t>(CloseButtonIconColorState::NegativeWhenHoveredPressed) & closeButtonIconColorState) {
        closeButtonIconColor->addItem(negativeWhenHoveredPressedString);
    }

    if (static_cast<uint32_t>(CloseButtonIconColorState::White) & closeButtonIconColorState) {
        closeButtonIconColor->addItem(whiteString);
    }

    if (static_cast<uint32_t>(CloseButtonIconColorState::WhiteWhenHoveredPressed) & closeButtonIconColorState) {
        closeButtonIconColor->addItem(WhiteWhenHoveredPressedString);
    }

    if (visible) {
        closeButtonIconColor->setVisible(true);
        closeButtonIconColorLabel->setVisible(true);
    } else {
        closeButtonIconColor->setVisible(false);
        closeButtonIconColorLabel->setVisible(false);
    }
    if (active) {
        m_closeButtonIconColorStateActive = closeButtonIconColorState;
    } else {
        m_closeButtonIconColorStateInactive = closeButtonIconColorState;
    }
}

int ButtonColors::convertCloseButtonIconColorUiToSettingsIndex(bool active, const int uiIndex)
{
    /*
    enum struct CloseButtonIconColorState{
        AsSelected = 1,
        NegativeWhenHoveredPressed = 2,
        White = 4,
        WhiteWhenHoveredPressed = 8
    }

*/

    uint32_t state = active ? m_closeButtonIconColorStateActive : m_closeButtonIconColorStateInactive;
    uint32_t selectedStateBit = 1;
    int uiItemCount = 0;
    int numConfigItems = 4;

    for (int i = 0; i < numConfigItems; i++, selectedStateBit = selectedStateBit << 1) {
        if (state & selectedStateBit) {
            if (uiItemCount == uiIndex) {
                return i;
            }
            uiItemCount++;
        }
    }
    return -1;
}

int ButtonColors::convertCloseButtonIconColorSettingsToUiIndex(bool active, const int settingsIndex)
{
    /*
    enum struct CloseButtonIconColorState{
        AsSelected = 1,
        NegativeWhenHoveredPressed = 2,
        White = 4,
        WhiteWhenHoveredPressed = 8
    }

*/
    uint32_t state = active ? m_closeButtonIconColorStateActive : m_closeButtonIconColorStateInactive;
    uint32_t selectedStateBit = 1;
    int uiItemCount = 0;
    int numConfigItems = 4;

    for (int i = 0; i < numConfigItems; i++, selectedStateBit = selectedStateBit << 1) {
        if (state & selectedStateBit) {
            if (i == settingsIndex) {
                return uiItemCount;
            }
            uiItemCount++;
        }
    }
    return -1;
}

void ButtonColors::loadCloseButtonIconColor(bool active)
{
    QComboBox *closeButtonIconColor = active ? m_ui->closeButtonIconColorActive : m_ui->closeButtonIconColorInactive;
    int uiIndex = convertCloseButtonIconColorSettingsToUiIndex(active, m_internalSettings->closeButtonIconColor(active));
    if (uiIndex < 0)
        uiIndex = 0;
    closeButtonIconColor->setCurrentIndex(uiIndex);
}

void ButtonColors::setNegativeCloseBackgroundHoverPressState(bool active)
{
    QComboBox *buttonBackgroundColors = active ? m_ui->buttonBackgroundColorsActive : m_ui->buttonBackgroundColorsInactive;
    QCheckBox *negativeCloseBackgroundHoverPress = active ? m_ui->negativeCloseBackgroundHoverPressActive : m_ui->negativeCloseBackgroundHoverPressInactive;

    bool drawCloseBackgroundNormally = m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::Backgrounds
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::BackgroundsAndOutlines
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsAndBackgrounds
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsAndCloseButtonBackground
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsBackgroundsAndOutlines
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsOutlinesAndCloseButtonBackground;

    bool drawBackgroundNormally = m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::Backgrounds
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::BackgroundsAndOutlines
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsAndBackgrounds
        || m_internalSettings->alwaysShow() == InternalSettings::EnumAlwaysShow::IconsBackgroundsAndOutlines;

    if (drawCloseBackgroundNormally
        && (buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
            || buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose)) {
        negativeCloseBackgroundHoverPress->setText(i18n("Negative close on hover/press only"));
        negativeCloseBackgroundHoverPress->setVisible(true);
    } else if (drawBackgroundNormally && buttonBackgroundColors->currentIndex() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        negativeCloseBackgroundHoverPress->setText(i18n("Traffic lights on hover/press only"));
        negativeCloseBackgroundHoverPress->setVisible(true);
    } else {
        negativeCloseBackgroundHoverPress->setVisible(false);
    }
}

void ButtonColors::showHideTranslucencySettings(bool active)
{
    QCheckBox *translucentButtonBackgrounds = active ? m_ui->translucentButtonBackgroundsActive : m_ui->translucentButtonBackgroundsInactive;
    QSlider *translucentButtonBackgroundsOpacitySlider =
        active ? m_ui->translucentButtonBackgroundsOpacitySliderActive : m_ui->translucentButtonBackgroundsOpacitySliderInactive;
    QLabel *opacitySliderLabel_1 = active ? m_ui->opacitySliderLabel_1_Active : m_ui->opacitySliderLabel_1_Inactive;
    QLabel *opacitySliderLabel_2 = active ? m_ui->opacitySliderLabel_2_Active : m_ui->opacitySliderLabel_2_Inactive;
    QSpinBox *translucentButtonBackgroundsOpacity =
        active ? m_ui->translucentButtonBackgroundsOpacityActive : m_ui->translucentButtonBackgroundsOpacityInactive;

    if (translucentButtonBackgrounds->isChecked()) {
        translucentButtonBackgroundsOpacitySlider->setVisible(true);
        opacitySliderLabel_1->setVisible(true);
        opacitySliderLabel_2->setVisible(true);
        translucentButtonBackgroundsOpacity->setVisible(true);
    } else {
        translucentButtonBackgroundsOpacitySlider->setVisible(false);
        opacitySliderLabel_1->setVisible(false);
        opacitySliderLabel_2->setVisible(false);
        translucentButtonBackgroundsOpacity->setVisible(false);
    }
}

void ButtonColors::resizeOverrideColorTable()
{
    QCheckBox *checkBox = qobject_cast<QCheckBox *>(QObject::sender());
    QTableWidget *table = nullptr;
    if (checkBox) {
        if (checkBox->parentWidget() && checkBox->parentWidget()->parentWidget() && checkBox->parentWidget()->parentWidget()->parentWidget()) {
            table = qobject_cast<QTableWidget *>(checkBox->parentWidget()->parentWidget()->parentWidget());
        }
    }

    if (table) {
        table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
}

void ButtonColors::showActiveOverrideGroupBox(const bool value)
{
    if (!value) {
        m_ui->activeOverrideGroupBox->setVisible(value);
    } else {
        m_ui->activeOverrideGroupBox->setVisible(value);
    }
}

void ButtonColors::showInactiveOverrideGroupBox(const bool value)
{
    if (!value) {
        m_ui->inactiveOverrideGroupBox->setVisible(value);
    } else {
        m_ui->inactiveOverrideGroupBox->setVisible(value);
    }
}

void ButtonColors::resizeDialog()
{
    if (!(m_ui->buttonColorOverrideToggleActive->isChecked() || m_ui->buttonColorOverrideToggleInactive->isChecked())) {
        this->setMaximumSize(755, 400);
        this->showNormal();
    } else {
        this->setMaximumSize(16777215, 16777215);
        this->showMaximized();
    }
}

void ButtonColors::tableVerticalHeaderSectionClicked(const int row)
{
    QHeaderView *headerview = qobject_cast<QHeaderView *>(QObject::sender());
    qDebug() << "sender: " << QObject::sender();
    if (!headerview)
        return;

    QTableWidget *table = qobject_cast<QTableWidget *>(headerview->parentWidget());
    if (table) {
        QTableWidgetItem *item = table->verticalHeaderItem(row);
        if (item) {
            if (item->checkState() == Qt::CheckState::Checked) {
                setTableVerticalHeaderSectionCheckedState(table, row, false);
            } else {
                setTableVerticalHeaderSectionCheckedState(table, row, true);
            }
        }
    }
}

void ButtonColors::setTableVerticalHeaderSectionCheckedState(QTableWidget *table, const int row, const bool checked)
{
    QTableWidgetItem *item = table->verticalHeaderItem(row);
    if (!item)
        return;
    item->setCheckState(checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    item->setIcon(checked ? static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Locked)
                          : static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Unlocked));
}

void ButtonColors::copyCellCheckedStatusToOtherTable()
{
    if (m_loading || m_processingDefaults || !m_ui->lockButtonColorsActive->isChecked())
        return; // only do this for user interactions where the active/inactive lock is checked
    QCheckBox *checkBox = qobject_cast<QCheckBox *>(QObject::sender());
    if (checkBox) {
        bool columnOk, rowOk;
        int column = checkBox->property("column").toInt(&columnOk);
        int row = checkBox->property("row").toInt(&rowOk);

        if (columnOk && rowOk) {
            if (checkBox->parentWidget() && checkBox->parentWidget()->parentWidget() && checkBox->parentWidget()->parentWidget()->parentWidget()) {
                QTableWidget *table = qobject_cast<QTableWidget *>(checkBox->parentWidget()->parentWidget()->parentWidget());
                if (table) {
                    QTableWidget *otherTable;
                    QCheckBox *otherOverrideToggle;
                    if (table == m_ui->overrideColorTableActive) {
                        otherTable = m_ui->overrideColorTableInactive;
                        otherOverrideToggle = m_ui->buttonColorOverrideToggleInactive;
                    } else if (table == m_ui->overrideColorTableInactive) {
                        otherTable = m_ui->overrideColorTableActive;
                        otherOverrideToggle = m_ui->buttonColorOverrideToggleActive;
                    } else
                        return;

                    QCheckBox *destinationCheckBox = nullptr;
                    KColorButton *destinationColorButton = nullptr;

                    if (checkBoxAndColorButtonAtTableCell(otherTable, column, row, destinationCheckBox, destinationColorButton)) {
                        destinationCheckBox->setChecked(checkBox->isChecked());
                        if (checkBox->isChecked())
                            otherOverrideToggle->setChecked(true);
                    }
                }
            }
        }
    }
}

// given a colorButton sending a changed signal to this slot, gets the table cell in which the colorButton was located and copies the same data to the
// equivalent inactive window table cell and to the rest of the row if the row lock is set
void ButtonColors::copyCellColorDataToOtherCells()
{
    if (m_loading || m_processingDefaults)
        return; // only do this for user interactions
    KColorButton *colorButton = qobject_cast<KColorButton *>(QObject::sender());
    if (colorButton) {
        bool columnOk, rowOk;
        int column = colorButton->property("column").toInt(&columnOk);
        int row = colorButton->property("row").toInt(&rowOk);

        if (columnOk && rowOk) {
            if (colorButton->parentWidget() && colorButton->parentWidget()->parentWidget() && colorButton->parentWidget()->parentWidget()->parentWidget()) {
                QTableWidget *table = qobject_cast<QTableWidget *>(colorButton->parentWidget()->parentWidget()->parentWidget());
                if (table) {
                    QTableWidget *otherTable;
                    QPushButton *activeInactiveLock;
                    if (table == m_ui->overrideColorTableActive) {
                        activeInactiveLock = m_ui->lockButtonColorsActive;
                        otherTable = m_ui->overrideColorTableInactive;
                    } else if (table == m_ui->overrideColorTableInactive) {
                        activeInactiveLock = m_ui->lockButtonColorsInactive;
                        otherTable = m_ui->overrideColorTableActive;
                    } else
                        return;

                    if (activeInactiveLock->isChecked()) {
                        QCheckBox *destinationCheckBox = nullptr;
                        KColorButton *destinationColorButton = nullptr;

                        if (checkBoxAndColorButtonAtTableCell(otherTable, column, row, destinationCheckBox, destinationColorButton)) {
                            destinationColorButton->setColor(colorButton->color());
                        }
                    }

                    QTableWidgetItem *item = table->verticalHeaderItem(row);
                    if (item) {
                        if (item->checkState() == Qt::CheckState::Checked) { // if the lock icon is locked for the row
                            for (int destinationColumn = 0; destinationColumn < m_colorOverridableButtonTypesStrings.count(); destinationColumn++) {
                                QCheckBox *destinationCheckBox = nullptr;
                                KColorButton *destinationColorButton = nullptr;

                                if (destinationColumn != column) { // copy the values to all other colorBoxes in the row
                                    if (checkBoxAndColorButtonAtTableCell(table, destinationColumn, row, destinationCheckBox, destinationColorButton)) {
                                        destinationColorButton->setColor(colorButton->color());
                                        destinationCheckBox->setChecked(true);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool ButtonColors::checkBoxAndColorButtonAtTableCell(QTableWidget *table,
                                                     const int column,
                                                     const int row,
                                                     QCheckBox *&outputCheckBox,
                                                     KColorButton *&outputColorButton)
{
    outputCheckBox = nullptr;
    outputColorButton = nullptr;

    if (!table)
        return false;
    QWidget *widget = table->cellWidget(row, column);
    if (!widget)
        return false;
    if (!widget->children().count())
        return false;

    QCheckBox *checkBox = nullptr;
    for (const auto &child : widget->children()) {
        checkBox = qobject_cast<QCheckBox *>(child);
        if (checkBox) {
            outputCheckBox = checkBox;
            break;
        }
    }
    if (!outputCheckBox)
        return false;

    KColorButton *colorButton = nullptr;
    for (const auto &child : widget->children()) {
        colorButton = qobject_cast<KColorButton *>(child);
        if (colorButton) {
            outputColorButton = colorButton;
            break;
        }
    }

    if (outputCheckBox && outputColorButton) {
        return true;
    }
    return false;
}

// colorsFlags storage bits: Active Window colours: bits 0-15, Inactive Window colours bits 16-31
// active icon bits 0-3, active background bits 4-7, active outline bits 8-11
// inactive icon bits 16-19, inactive background bits 20-23, inactive outline bits 24-27
// bit 3 etc. reserved for deactivated state colour
void ButtonColors::encodeColorOverridableButtonTypeColumn(QTableWidget *tableActive,
                                                          QTableWidget *tableInactive,
                                                          int column,
                                                          uint32_t &colorsFlags,
                                                          QList<int> &colorsList,
                                                          bool resetActive,
                                                          bool resetInactive)
{
    uint32_t bitMask = 0x00000001;
    colorsFlags = 0x00000000;
    uint32_t validColorsFlagsBits = 0x07770777;
    QTableWidget *table;
    bool reset;

    colorsList.clear();
    for (uint32_t i = 0, rowIndex = 0; i < 32; i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            QCheckBox *overrideCheckBox = nullptr;
            KColorButton *overrideColorButton = nullptr;
            if (i < 16) {
                table = tableActive;
                reset = resetActive;
            } else {
                if (i == 16)
                    rowIndex = 0; // reset the rows to 0 for referencing the inactive colours
                table = tableInactive;
                reset = resetInactive;
            }
            if (checkBoxAndColorButtonAtTableCell(table, column, rowIndex, overrideCheckBox, overrideColorButton)) {
                if (overrideCheckBox->isChecked() && !reset) { // set the bit of the row by OR-ing the bitmask with the existing value
                    colorsFlags = colorsFlags | bitMask;
                    colorsList.append(static_cast<int>(overrideColorButton->color().rgba())); // convert the colour to an int and store on the intList
                } else { // clear the bit by AND-ing with the inverse of the bitmask
                    colorsFlags = colorsFlags & (~bitMask);
                }
            }
            rowIndex++;
        }
    }
}

ColumnsLoaded ButtonColors::decodeColorsFlagsAndLoadColumn(QTableWidget *tableActive,
                                                           QTableWidget *tableInactive,
                                                           int column,
                                                           uint32_t colorsFlags,
                                                           const QList<int> &colorsList)
{
    uint32_t bitMask = 0x00000001;
    uint32_t validColorsFlagsBits = 0x07770777;
    QTableWidget *table;
    int colorsListIndex = 0;
    ColumnsLoaded columnsLoaded;
    columnsLoaded.active = false;
    columnsLoaded.inactive = false;
    for (uint32_t i = 0, rowIndex = 0; i < 32; i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            QCheckBox *overrideCheckBox = nullptr;
            KColorButton *overrideColorButton = nullptr;
            if (i < 16) {
                table = tableActive;
            } else {
                if (i == 16)
                    rowIndex = 0; // reset the rows to 0 for referencing the inactive colours
                table = tableInactive;
            }
            if (checkBoxAndColorButtonAtTableCell(table, column, rowIndex, overrideCheckBox, overrideColorButton)) {
                if (colorsFlags & bitMask) { // if the current bit in colorsFlags is 1
                    if (table == tableActive) {
                        columnsLoaded.active = true;
                    } else {
                        columnsLoaded.inactive = true;
                    }
                    overrideCheckBox->setChecked(true);
                    if (colorsList.count() && colorsListIndex < colorsList.count()) { // this if is to prevent against an unlikely corruption situation when
                                                                                      // colorsSet and colorsist are out of sync
                        QRgb color = QRgb(static_cast<QRgb>(colorsList[colorsListIndex++]));
                        QColor qcolor(color);
                        qcolor.setAlpha(qAlpha(color));
                        overrideColorButton->setColor(qcolor);
                    }
                } else {
                    overrideCheckBox->setChecked(false);
                    overrideColorButton->setColor(QColor());
                }
            }
            rowIndex++;
        }
    }
    return columnsLoaded;
}

uint32_t ButtonColors::encodeColorOverridableLockStates()
{
    uint32_t bitMask = 0x00000001;
    uint32_t lockFlags = 0x00000000;
    uint32_t validColorsFlagsBits = 0x07770777;
    QTableWidget *table;

    for (uint32_t i = 0, rowIndex = 0; i < 32; i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            if (i < 16) {
                table = m_ui->overrideColorTableActive;
            } else {
                if (i == 16)
                    rowIndex = 0; // reset the rows to 0 for referencing the inactive colours
                table = m_ui->overrideColorTableInactive;
            }
            QTableWidgetItem *item = table->verticalHeaderItem(rowIndex);
            if (item) {
                if (item->checkState() == Qt::CheckState::Checked) { // set the bit of the row by OR-ing the bitmask with the existing value
                    lockFlags = lockFlags | bitMask;
                } else { // clear the bit by AND-ing with the inverse of the bitmask
                    lockFlags = lockFlags & (~bitMask);
                }
            }
            rowIndex++;
        }
    }
    return lockFlags;
}

bool ButtonColors::decodeColorOverridableLockStatesAndLoadVerticalHeaderLocks()
{
    uint32_t bitMask = 0x00000001;
    uint32_t validColorsFlagsBits = 0x07770777;

    QTableWidget *table;
    bool columnHasOverrideColorsLoaded = false;
    uint32_t lockFlags = m_internalSettings->buttonOverrideColorsLockStates();
    for (uint32_t i = 0, rowIndex = 0; i < 32; i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            if (i < 16) {
                table = m_ui->overrideColorTableActive;
            } else {
                if (i == 16)
                    rowIndex = 0; // reset the rows to 0 for referencing the inactive colours
                table = m_ui->overrideColorTableInactive;
            }
            QTableWidgetItem *item = table->verticalHeaderItem(rowIndex);
            if (item) {
                if (lockFlags & bitMask) { // if the current bit in lockFlags is 1
                    setTableVerticalHeaderSectionCheckedState(table, rowIndex, true);
                } else {
                    setTableVerticalHeaderSectionCheckedState(table, rowIndex, false);
                }
            }
            rowIndex++;
        }
    }
    return columnHasOverrideColorsLoaded;
}

void ButtonColors::loadButtonPaletteColorsIcons()
{
    if (m_ui->tabWidget->currentIndex() == 0)
        loadButtonPaletteColorsIconsMain(true);
    else if (m_ui->tabWidget->currentIndex() == 1)
        loadButtonPaletteColorsIconsMain(false);
}

void ButtonColors::loadButtonPaletteColorsIconsMain(bool active)
{
    auto uiButtonIconColors = active ? m_ui->buttonIconColorsActive : m_ui->buttonIconColorsInactive;
    auto uiCloseButtonIconColor = active ? m_ui->closeButtonIconColorActive : m_ui->closeButtonIconColorInactive;
    auto uiButtonBackgroundColors = active ? m_ui->buttonBackgroundColorsActive : m_ui->buttonBackgroundColorsInactive;
    auto uiTranslucentButtonBackgrounds = active ? m_ui->translucentButtonBackgroundsActive : m_ui->translucentButtonBackgroundsInactive;
    auto uiNegativeCloseBackgroundHoverPress = active ? m_ui->negativeCloseBackgroundHoverPressActive : m_ui->negativeCloseBackgroundHoverPressInactive;
    auto uiSetBlackWhiteIconOnPoorContrast = active ? m_ui->blackWhiteIconOnPoorContrastActive : m_ui->blackWhiteIconOnPoorContrastInactive;
    auto uiSetAdjustBackgroundColorOnPoorContrast =
        active ? m_ui->adjustBackgroundColorOnPoorContrastActive : m_ui->adjustBackgroundColorOnPoorContrastInactive;
    auto uiSetTranclucentButtonBackgroundsOpacity =
        active ? m_ui->translucentButtonBackgroundsOpacityActive : m_ui->translucentButtonBackgroundsOpacityInactive;

    InternalSettingsPtr temporaryColorSettings =
        InternalSettingsPtr(new InternalSettings); // temporary settings that reflect the UI, only to instantly update the colours displayed in the UI
    temporaryColorSettings->load();

    // temporaryColorSettings->setButtonIconColors(m_ui->buttonIconColors->currentIndex());
    temporaryColorSettings->setCloseButtonIconColor(active, convertCloseButtonIconColorUiToSettingsIndex(active, uiCloseButtonIconColor->currentIndex()));
    // temporaryColorSettings->setButtonBackgroundColors(m_ui->buttonBackgroundColors->currentIndex());
    temporaryColorSettings->setTranslucentButtonBackgrounds(active, uiTranslucentButtonBackgrounds->isChecked());
    temporaryColorSettings->setNegativeCloseBackgroundHoverPress(active, uiNegativeCloseBackgroundHoverPress->isChecked());
    // temporaryColorSettings->setLockButtonColorsActive(m_ui->lockButtonColorsActive->isChecked());
    temporaryColorSettings->setBlackWhiteIconOnPoorContrast(active, uiSetBlackWhiteIconOnPoorContrast->isChecked());
    temporaryColorSettings->setAdjustBackgroundColorOnPoorContrast(active, uiSetAdjustBackgroundColorOnPoorContrast->isChecked());
    temporaryColorSettings->setTranslucentButtonBackgroundsOpacity(active, uiSetTranclucentButtonBackgroundsOpacity->value() / 100.0f);

    DecorationPalette decorationPalette(QApplication::palette(), temporaryColorSettings, false, true);

    QColor titlebarTextActive;
    QColor titlebarTextInactive;
    QColor titlebarBackgroundActive;
    QColor titlebarBackgroundInactive;
    // get the titlebar text colour
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    bool colorSchemeHasHeaderColor = KColorScheme::isColorSetSupported(config, KColorScheme::Header);

    // get the alpha values from the system colour scheme
    if (colorSchemeHasHeaderColor) {
        KColorScheme activeHeader = KColorScheme(QPalette::Active, KColorScheme::Header, config);
        titlebarTextActive = activeHeader.foreground().color();
        titlebarBackgroundActive = activeHeader.background().color();

        KColorScheme inactiveHeader = KColorScheme(QPalette::Inactive, KColorScheme::Header, config);
        titlebarTextInactive = inactiveHeader.foreground().color();
        titlebarBackgroundInactive = inactiveHeader.background().color();
    } else {
        KConfigGroup wmConfig(config, QStringLiteral("WM"));
        if (wmConfig.exists()) {
            titlebarTextActive = wmConfig.readEntry("activeForeground", QColorConstants::Black);
            titlebarTextInactive = wmConfig.readEntry("inactiveForeground", QColorConstants::Black);
            titlebarBackgroundActive = wmConfig.readEntry("activeBackground", QColorConstants::Black);
            titlebarBackgroundInactive = wmConfig.readEntry("inactiveBackground", QColorConstants::Black);
        }
    }

    DecorationButtonBehaviour buttonBehaviour;
    buttonBehaviour.reconfigure(temporaryColorSettings);
    auto getGroup = [](DecorationButtonPalette *palette, bool active) {
        return active ? palette->active() : palette->inactive();
    };
    DecorationButtonPalette closeButtonPalette(KDecoration2::DecorationButtonType::Close);
    DecorationButtonPalette maximizeButtonPalette(KDecoration2::DecorationButtonType::Maximize);
    DecorationButtonPalette minimizeButtonPalette(KDecoration2::DecorationButtonType::Minimize);
    DecorationButtonPalette otherButtonPalette(KDecoration2::DecorationButtonType::Custom);

    QList<DecorationButtonPalette *> otherCloseButtonList{&otherButtonPalette, &closeButtonPalette};
    otherCloseButtonList = sortButtonsAsPerKwinConfig(otherCloseButtonList);
    QList<DecorationButtonPalette *> otherTrafficLightsButtonList{&otherButtonPalette, &closeButtonPalette, &maximizeButtonPalette, &minimizeButtonPalette};
    otherTrafficLightsButtonList = sortButtonsAsPerKwinConfig(otherTrafficLightsButtonList);
    QList<DecorationButtonPalette *> trafficLightsButtonList{&closeButtonPalette, &maximizeButtonPalette, &minimizeButtonPalette};
    trafficLightsButtonList = sortButtonsAsPerKwinConfig(trafficLightsButtonList);

    qreal size = 32;

    QRectF oneByThree00(QPointF(0, 0), QPointF(size, size / 3));
    QRectF oneByThree01(QPointF(0, size / 3), QPointF(size, size * 2 / 3));
    QRectF oneByThree02(QPointF(0, size * 2 / 3), QPointF(size, size));

    QRectF twoByThree[2][3];
    twoByThree[0][0] = QRectF(QPointF(0, 0), QPointF(size / 2, size / 3));
    twoByThree[0][1] = QRectF(QPointF(0, size / 3), QPointF(size / 2, size * 2 / 3));
    twoByThree[0][2] = QRectF(QPointF(0, size * 2 / 3), QPointF(size / 2, size));
    twoByThree[1][0] = QRectF(QPointF(size / 2, 0), QPointF(size, size / 3));
    twoByThree[1][1] = QRectF(QPointF(size / 2, size / 3), QPointF(size, size * 2 / 3));
    twoByThree[1][2] = QRectF(QPointF(size / 2, size * 2 / 3), QPointF(size, size));

    QRectF threeByThree[3][3];
    threeByThree[0][0] = QRectF(QPointF(0, 0), QPointF(size / 3, size / 3));
    threeByThree[0][1] = QRectF(QPointF(0, size / 3), QPointF(size / 3, size * 2 / 3));
    threeByThree[0][2] = QRectF(QPointF(0, size * 2 / 3), QPointF(size / 3, size));
    threeByThree[1][0] = QRectF(QPointF(size / 3, 0), QPointF(size * 2 / 3, size / 3));
    threeByThree[1][1] = QRectF(QPointF(size / 3, size / 3), QPointF(size * 2 / 3, size * 2 / 3));
    threeByThree[1][2] = QRectF(QPointF(size / 3, size * 2 / 3), QPointF(size * 2 / 3, size));
    threeByThree[2][0] = QRectF(QPointF(size * 2 / 3, 0), QPointF(size, size / 3));
    threeByThree[2][1] = QRectF(QPointF(size * 2 / 3, size / 3), QPointF(size, size * 2 / 3));
    threeByThree[2][2] = QRectF(QPointF(size * 2 / 3, size * 2 / 3), QPointF(size, size));

    QRectF fourByThree[4][3];
    fourByThree[0][0] = QRectF(QPointF(0, 0), QPointF(size / 4, size / 3));
    fourByThree[0][1] = QRectF(QPointF(0, size / 3), QPointF(size / 4, size * 2 / 3));
    fourByThree[0][2] = QRectF(QPointF(0, size * 2 / 3), QPointF(size / 4, size));
    fourByThree[1][0] = QRectF(QPointF(size / 4, 0), QPointF(size / 2, size / 3));
    fourByThree[1][1] = QRectF(QPointF(size / 4, size / 3), QPointF(size / 2, size * 2 / 3));
    fourByThree[1][2] = QRectF(QPointF(size / 4, size * 2 / 3), QPointF(size / 2, size));
    fourByThree[2][0] = QRectF(QPointF(size / 2, 0), QPointF(size * 3 / 4, size / 3));
    fourByThree[2][1] = QRectF(QPointF(size / 2, size / 3), QPointF(size * 3 / 4, size * 2 / 3));
    fourByThree[2][2] = QRectF(QPointF(size / 2, size * 2 / 3), QPointF(size * 3 / 4, size));
    fourByThree[3][0] = QRectF(QPointF(size * 3 / 4, 0), QPointF(size, size / 3));
    fourByThree[3][1] = QRectF(QPointF(size * 3 / 4, size / 3), QPointF(size, size * 2 / 3));
    fourByThree[3][2] = QRectF(QPointF(size * 3 / 4, size * 2 / 3), QPointF(size, size));

    QPixmap pixmap(size, size);
    std::unique_ptr<QPainter> painter = std::make_unique<QPainter>(&pixmap);
    painter->setPen(Qt::NoPen);

    // background colors
    temporaryColorSettings->setButtonBackgroundColors(active, InternalSettings::EnumButtonBackgroundColors::TitlebarText);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    };

    uiButtonBackgroundColors->setIconSize(QSize(size, size));
    pixmap.fill(Qt::transparent);
    painter->setBrush(getGroup(&otherButtonPalette, active)->backgroundPress.isValid() ? getGroup(&otherButtonPalette, active)->backgroundPress
                                                                                       : Qt::transparent);
    painter->drawRect(oneByThree00);
    painter->setBrush(getGroup(&otherButtonPalette, active)->backgroundHover.isValid() ? getGroup(&otherButtonPalette, active)->backgroundHover
                                                                                       : Qt::transparent);
    painter->drawRect(oneByThree01);
    painter->setBrush(getGroup(&otherButtonPalette, active)->backgroundNormal.isValid() ? getGroup(&otherButtonPalette, active)->backgroundNormal
                                                                                        : Qt::transparent);
    painter->drawRect(oneByThree02);
    QIcon titlebarText(pixmap);
    uiButtonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::TitlebarText, titlebarText);

    temporaryColorSettings->setButtonBackgroundColors(active, InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->backgroundPress.isValid() ? getGroup(otherCloseButtonList[i], active)->backgroundPress
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->backgroundHover.isValid() ? getGroup(otherCloseButtonList[i], active)->backgroundHover
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->backgroundNormal.isValid()
                                  ? getGroup(otherCloseButtonList[i], active)->backgroundNormal
                                  : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon backgroundTitlebarTextNegativeClose(pixmap);
    uiButtonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose, backgroundTitlebarTextNegativeClose);

    temporaryColorSettings->setButtonBackgroundColors(active, InternalSettings::EnumButtonBackgroundColors::Accent);
    otherButtonPalette.reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);

    pixmap.fill(Qt::transparent);
    painter->setBrush(getGroup(&otherButtonPalette, active)->backgroundPress.isValid() ? getGroup(&otherButtonPalette, active)->backgroundPress
                                                                                       : Qt::transparent);
    painter->drawRect(oneByThree00);
    painter->setBrush(getGroup(&otherButtonPalette, active)->backgroundHover.isValid() ? getGroup(&otherButtonPalette, active)->backgroundHover
                                                                                       : Qt::transparent);
    painter->drawRect(oneByThree01);
    painter->setBrush(getGroup(&otherButtonPalette, active)->backgroundNormal.isValid() ? getGroup(&otherButtonPalette, active)->backgroundNormal
                                                                                        : Qt::transparent);
    painter->drawRect(oneByThree02);
    QIcon backgroundAccent(pixmap);
    uiButtonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::Accent, backgroundAccent);

    temporaryColorSettings->setButtonBackgroundColors(active, InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->backgroundPress.isValid() ? getGroup(otherCloseButtonList[i], active)->backgroundPress
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->backgroundHover.isValid() ? getGroup(otherCloseButtonList[i], active)->backgroundHover
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->backgroundNormal.isValid()
                                  ? getGroup(otherCloseButtonList[i], active)->backgroundNormal
                                  : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon backgroundAccentNegativeClose(pixmap);
    uiButtonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose, backgroundAccentNegativeClose);

    temporaryColorSettings->setButtonBackgroundColors(active, InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights);

    for (auto &buttonPalette : otherTrafficLightsButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    }

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 4; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(getGroup(otherTrafficLightsButtonList[i], active)->backgroundPress.isValid()
                                  ? getGroup(otherTrafficLightsButtonList[i], active)->backgroundPress
                                  : Qt::transparent);
            painter->drawRect(fourByThree[i][0]);
            painter->setBrush(getGroup(otherTrafficLightsButtonList[i], active)->backgroundHover.isValid()
                                  ? getGroup(otherTrafficLightsButtonList[i], active)->backgroundHover
                                  : Qt::transparent);
            painter->drawRect(fourByThree[i][1]);
            painter->setBrush(getGroup(otherTrafficLightsButtonList[i], active)->backgroundNormal.isValid()
                                  ? getGroup(otherTrafficLightsButtonList[i], active)->backgroundNormal
                                  : Qt::transparent);
            painter->drawRect(fourByThree[i][2]);
        }
    }
    QIcon backgroundAccentTrafficLightsClose(pixmap);
    uiButtonBackgroundColors->setItemIcon(InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights, backgroundAccentTrafficLightsClose);

    // icon colors ---------------------------------------------------------------------------------
    uiButtonIconColors->setIconSize(QSize(24, 24));
    temporaryColorSettings->setButtonBackgroundColors(active, uiButtonBackgroundColors->currentIndex());

    temporaryColorSettings->setButtonIconColors(active, InternalSettings::EnumButtonIconColors::TitlebarText);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundPress.isValid() ? getGroup(otherCloseButtonList[i], active)->foregroundPress
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundHover.isValid() ? getGroup(otherCloseButtonList[i], active)->foregroundHover
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundNormal.isValid()
                                  ? getGroup(otherCloseButtonList[i], active)->foregroundNormal
                                  : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon iconTitlebarText(pixmap);
    uiButtonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::TitlebarText, iconTitlebarText);

    temporaryColorSettings->setButtonIconColors(active, InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundPress.isValid() ? getGroup(otherCloseButtonList[i], active)->foregroundPress
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundHover.isValid() ? getGroup(otherCloseButtonList[i], active)->foregroundHover
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundNormal.isValid()
                                  ? getGroup(otherCloseButtonList[i], active)->foregroundNormal
                                  : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon iconTitlebarTextNegativeClose(pixmap);
    uiButtonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose, iconTitlebarTextNegativeClose);

    temporaryColorSettings->setButtonIconColors(active, InternalSettings::EnumButtonIconColors::Accent);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundPress.isValid() ? getGroup(otherCloseButtonList[i], active)->foregroundPress
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundHover.isValid() ? getGroup(otherCloseButtonList[i], active)->foregroundHover
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundNormal.isValid()
                                  ? getGroup(otherCloseButtonList[i], active)->foregroundNormal
                                  : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon iconAccent(pixmap);
    uiButtonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::Accent, iconAccent);

    temporaryColorSettings->setButtonIconColors(active, InternalSettings::EnumButtonIconColors::AccentNegativeClose);
    for (auto &buttonPalette : otherCloseButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    };

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 2; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundPress.isValid() ? getGroup(otherCloseButtonList[i], active)->foregroundPress
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][0]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundHover.isValid() ? getGroup(otherCloseButtonList[i], active)->foregroundHover
                                                                                                   : Qt::transparent);
            painter->drawRect(twoByThree[i][1]);
            painter->setBrush(getGroup(otherCloseButtonList[i], active)->foregroundNormal.isValid()
                                  ? getGroup(otherCloseButtonList[i], active)->foregroundNormal
                                  : Qt::transparent);
            painter->drawRect(twoByThree[i][2]);
        }
    }
    QIcon iconAccentNegativeClose(pixmap);
    uiButtonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::AccentNegativeClose, iconAccentNegativeClose);

    temporaryColorSettings->setButtonIconColors(active, InternalSettings::EnumButtonIconColors::AccentTrafficLights);
    for (auto &buttonPalette : otherTrafficLightsButtonList) {
        buttonPalette->reconfigure(temporaryColorSettings,
                                   &buttonBehaviour,
                                   &decorationPalette,
                                   titlebarTextActive,
                                   titlebarBackgroundActive,
                                   titlebarTextInactive,
                                   titlebarBackgroundInactive,
                                   true,
                                   active);
    }

    pixmap.fill(Qt::transparent);
    for (int i = 0; i < 4; i++) {
        if (i < otherTrafficLightsButtonList.count()) {
            painter->setBrush(getGroup(otherTrafficLightsButtonList[i], active)->foregroundPress.isValid()
                                  ? getGroup(otherTrafficLightsButtonList[i], active)->foregroundPress
                                  : Qt::transparent);
            painter->drawRect(fourByThree[i][0]);
            painter->setBrush(getGroup(otherTrafficLightsButtonList[i], active)->foregroundHover.isValid()
                                  ? getGroup(otherTrafficLightsButtonList[i], active)->foregroundHover
                                  : Qt::transparent);
            painter->drawRect(fourByThree[i][1]);
            painter->setBrush(getGroup(otherTrafficLightsButtonList[i], active)->foregroundNormal.isValid()
                                  ? getGroup(otherTrafficLightsButtonList[i], active)->foregroundNormal
                                  : Qt::transparent);
            painter->drawRect(fourByThree[i][2]);
        }
    }
    QIcon iconAccentTrafficLights(pixmap);
    uiButtonIconColors->setItemIcon(InternalSettings::EnumButtonIconColors::AccentTrafficLights, iconAccentTrafficLights);

    // closeButtonIconColor icons ----------------------------------------------------

    uiCloseButtonIconColor->setIconSize(QSize(16, 16));
    temporaryColorSettings->setButtonIconColors(active, uiButtonIconColors->currentIndex());

    int uiItemIndex = convertCloseButtonIconColorSettingsToUiIndex(active, InternalSettings::EnumCloseButtonIconColor::AsSelected);
    if (uiItemIndex >= 0) {
        temporaryColorSettings->setCloseButtonIconColor(active, InternalSettings::EnumCloseButtonIconColor::AsSelected);
        if (uiButtonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            for (auto &buttonPalette : trafficLightsButtonList) {
                buttonPalette->reconfigure(temporaryColorSettings,
                                           &buttonBehaviour,
                                           &decorationPalette,
                                           titlebarTextActive,
                                           titlebarBackgroundActive,
                                           titlebarTextInactive,
                                           titlebarBackgroundInactive,
                                           true,
                                           active);
            }
            pixmap.fill(Qt::transparent);
            for (int i = 0; i < 3; i++) {
                if (i < trafficLightsButtonList.count()) {
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundPress.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundPress
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][0]);
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundHover.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundHover
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][1]);
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundNormal.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundNormal
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][2]);
                }
            }
        } else {
            closeButtonPalette.reconfigure(temporaryColorSettings,
                                           &buttonBehaviour,
                                           &decorationPalette,
                                           titlebarTextActive,
                                           titlebarBackgroundActive,
                                           titlebarTextInactive,
                                           titlebarBackgroundInactive,
                                           true,
                                           active);
            pixmap.fill(Qt::transparent);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundPress.isValid() ? getGroup(&closeButtonPalette, active)->foregroundPress
                                                                                               : Qt::transparent);
            painter->drawRect(oneByThree00);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundHover.isValid() ? getGroup(&closeButtonPalette, active)->foregroundHover
                                                                                               : Qt::transparent);
            painter->drawRect(oneByThree01);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundNormal.isValid() ? getGroup(&closeButtonPalette, active)->foregroundNormal
                                                                                                : Qt::transparent);
            painter->drawRect(oneByThree02);
        }
        QIcon icon(pixmap);
        uiCloseButtonIconColor->setItemIcon(uiItemIndex, icon);
    }

    uiItemIndex = convertCloseButtonIconColorSettingsToUiIndex(active, InternalSettings::EnumCloseButtonIconColor::NegativeWhenHoverPress);
    if (uiItemIndex >= 0) {
        temporaryColorSettings->setCloseButtonIconColor(active, InternalSettings::EnumCloseButtonIconColor::NegativeWhenHoverPress);
        if (uiButtonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            for (auto &buttonPalette : trafficLightsButtonList) {
                buttonPalette->reconfigure(temporaryColorSettings,
                                           &buttonBehaviour,
                                           &decorationPalette,
                                           titlebarTextActive,
                                           titlebarBackgroundActive,
                                           titlebarTextInactive,
                                           titlebarBackgroundInactive,
                                           true,
                                           active);
            }
            pixmap.fill(Qt::transparent);
            for (int i = 0; i < 3; i++) {
                if (i < trafficLightsButtonList.count()) {
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundPress.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundPress
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][0]);
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundHover.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundHover
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][1]);
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundNormal.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundNormal
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][2]);
                }
            }
        } else {
            closeButtonPalette.reconfigure(temporaryColorSettings,
                                           &buttonBehaviour,
                                           &decorationPalette,
                                           titlebarTextActive,
                                           titlebarBackgroundActive,
                                           titlebarTextInactive,
                                           titlebarBackgroundInactive,
                                           true,
                                           active);
            pixmap.fill(Qt::transparent);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundPress.isValid() ? getGroup(&closeButtonPalette, active)->foregroundPress
                                                                                               : Qt::transparent);
            painter->drawRect(oneByThree00);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundHover.isValid() ? getGroup(&closeButtonPalette, active)->foregroundHover
                                                                                               : Qt::transparent);
            painter->drawRect(oneByThree01);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundNormal.isValid() ? getGroup(&closeButtonPalette, active)->foregroundNormal
                                                                                                : Qt::transparent);
            painter->drawRect(oneByThree02);
        }
        QIcon icon(pixmap);
        uiCloseButtonIconColor->setItemIcon(uiItemIndex, icon);
    }

    uiItemIndex = convertCloseButtonIconColorSettingsToUiIndex(active, InternalSettings::EnumCloseButtonIconColor::White);
    if (uiItemIndex >= 0) {
        temporaryColorSettings->setCloseButtonIconColor(active, InternalSettings::EnumCloseButtonIconColor::White);
        if (uiButtonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            for (auto &buttonPalette : trafficLightsButtonList) {
                buttonPalette->reconfigure(temporaryColorSettings,
                                           &buttonBehaviour,
                                           &decorationPalette,
                                           titlebarTextActive,
                                           titlebarBackgroundActive,
                                           titlebarTextInactive,
                                           titlebarBackgroundInactive,
                                           true,
                                           active);
            }
            pixmap.fill(Qt::transparent);
            for (int i = 0; i < 3; i++) {
                if (i < trafficLightsButtonList.count()) {
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundPress.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundPress
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][0]);
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundHover.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundHover
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][1]);
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundNormal.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundNormal
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][2]);
                }
            }
        } else {
            closeButtonPalette.reconfigure(temporaryColorSettings,
                                           &buttonBehaviour,
                                           &decorationPalette,
                                           titlebarTextActive,
                                           titlebarBackgroundActive,
                                           titlebarTextInactive,
                                           titlebarBackgroundInactive,
                                           true,
                                           active);
            pixmap.fill(Qt::transparent);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundPress.isValid() ? getGroup(&closeButtonPalette, active)->foregroundPress
                                                                                               : Qt::transparent);
            painter->drawRect(oneByThree00);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundHover.isValid() ? getGroup(&closeButtonPalette, active)->foregroundHover
                                                                                               : Qt::transparent);
            painter->drawRect(oneByThree01);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundNormal.isValid() ? getGroup(&closeButtonPalette, active)->foregroundNormal
                                                                                                : Qt::transparent);
            painter->drawRect(oneByThree02);
        }
        QIcon icon(pixmap);
        uiCloseButtonIconColor->setItemIcon(uiItemIndex, icon);
    }

    uiItemIndex = convertCloseButtonIconColorSettingsToUiIndex(active, InternalSettings::EnumCloseButtonIconColor::WhiteWhenHoverPress);
    if (uiItemIndex >= 0) {
        temporaryColorSettings->setCloseButtonIconColor(active, InternalSettings::EnumCloseButtonIconColor::WhiteWhenHoverPress);

        if (uiButtonIconColors->currentIndex() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
            for (auto &buttonPalette : trafficLightsButtonList) {
                buttonPalette->reconfigure(temporaryColorSettings,
                                           &buttonBehaviour,
                                           &decorationPalette,
                                           titlebarTextActive,
                                           titlebarBackgroundActive,
                                           titlebarTextInactive,
                                           titlebarBackgroundInactive,
                                           true,
                                           active);
            }
            pixmap.fill(Qt::transparent);
            for (int i = 0; i < 3; i++) {
                if (i < trafficLightsButtonList.count()) {
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundPress.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundPress
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][0]);
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundHover.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundHover
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][1]);
                    painter->setBrush(getGroup(trafficLightsButtonList[i], active)->foregroundNormal.isValid()
                                          ? getGroup(trafficLightsButtonList[i], active)->foregroundNormal
                                          : Qt::transparent);
                    painter->drawRect(threeByThree[i][2]);
                }
            }
        } else {
            closeButtonPalette.reconfigure(temporaryColorSettings,
                                           &buttonBehaviour,
                                           &decorationPalette,
                                           titlebarTextActive,
                                           titlebarBackgroundActive,
                                           titlebarTextInactive,
                                           titlebarBackgroundInactive,
                                           true,
                                           active);
            pixmap.fill(Qt::transparent);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundPress.isValid() ? getGroup(&closeButtonPalette, active)->foregroundPress
                                                                                               : Qt::transparent);
            painter->drawRect(oneByThree00);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundHover.isValid() ? getGroup(&closeButtonPalette, active)->foregroundHover
                                                                                               : Qt::transparent);
            painter->drawRect(oneByThree01);
            painter->setBrush(getGroup(&closeButtonPalette, active)->foregroundNormal.isValid() ? getGroup(&closeButtonPalette, active)->foregroundNormal
                                                                                                : Qt::transparent);
            painter->drawRect(oneByThree02);
        }
        QIcon icon(pixmap);
        uiCloseButtonIconColor->setItemIcon(uiItemIndex, icon);
    }
}

void ButtonColors::getButtonsOrderFromKwinConfig()
{
    QMap<KDecoration2::DecorationButtonType, QChar> buttonNames;
    // list modified from https://invent.kde.org/plasma/kwin/-/blob/master/src/decorations/settings.cpp
    buttonNames[KDecoration2::DecorationButtonType::Menu] = QChar('M');
    buttonNames[KDecoration2::DecorationButtonType::ApplicationMenu] = QChar('N');
    buttonNames[KDecoration2::DecorationButtonType::OnAllDesktops] = QChar('S');
    buttonNames[KDecoration2::DecorationButtonType::KeepAbove] = QChar('F');
    buttonNames[KDecoration2::DecorationButtonType::KeepBelow] = QChar('B');
    buttonNames[KDecoration2::DecorationButtonType::Shade] = QChar('L');
    buttonNames[KDecoration2::DecorationButtonType::ContextHelp] = QChar('H');
    buttonNames[KDecoration2::DecorationButtonType::Minimize] = QChar('I');
    buttonNames[KDecoration2::DecorationButtonType::Maximize] = QChar('A');
    buttonNames[KDecoration2::DecorationButtonType::Close] = QChar('X');

    QString buttonsOnLeft;
    QString buttonsOnRight;

    // very hacky way to do this -- better would be to find a way to get the settings from <KDecoration2/DecorationSettings>
    //  read kwin button border setting
    KSharedConfig::Ptr kwinConfig = KSharedConfig::openConfig(QStringLiteral("kwinrc"));
    if (kwinConfig && kwinConfig->hasGroup(QStringLiteral("org.kde.kdecoration2"))) {
        KConfigGroup kdecoration2Group = kwinConfig->group(QStringLiteral("org.kde.kdecoration2"));

        if (!kdecoration2Group.hasKey(QStringLiteral("ButtonsOnLeft"))) {
            buttonsOnLeft = QStringLiteral("MS");
        } else {
            buttonsOnLeft = kdecoration2Group.readEntry(QStringLiteral("ButtonsOnLeft"));
        }

        if (!kdecoration2Group.hasKey(QStringLiteral("ButtonsOnRight"))) {
            buttonsOnRight = QStringLiteral("HIAX");
        } else {
            buttonsOnRight = kdecoration2Group.readEntry(QStringLiteral("ButtonsOnRight"));
        }

    } else {
        buttonsOnLeft = QStringLiteral("MS");
        buttonsOnRight = QStringLiteral("HIAX");
    }

    QString visibleButtons = buttonsOnLeft + buttonsOnRight;

    m_visibleButtonsOrder.clear();
    for (QChar *it = visibleButtons.begin(); it != visibleButtons.end(); it++) {
        auto key = buttonNames.key(*it, KDecoration2::DecorationButtonType::Custom);
        if (key != KDecoration2::DecorationButtonType::Custom)
            m_visibleButtonsOrder.append(key);
    }

    m_hiddenButtons.clear();
    // add hidden buttons to m_hiddenButtons
    for (auto it = buttonNames.begin(); it != buttonNames.end(); it++) {
        if (!visibleButtons.contains(*it)) {
            m_hiddenButtons.append(buttonNames.key(*it));
        }
    }

    // Place a custom button type in the average position of these "other" button types
    QList<int> otherButtonIndexes{
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::Menu),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::ApplicationMenu),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::OnAllDesktops),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::ContextHelp),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::KeepAbove),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::KeepBelow),
        m_visibleButtonsOrder.indexOf(KDecoration2::DecorationButtonType::Shade),
    };

    // remove the -1s (index not found)
    QMutableListIterator<int> i(otherButtonIndexes);
    while (i.hasNext()) {
        if (i.next() == -1)
            i.remove();
    }

    int indexOfCustom;
    if (otherButtonIndexes.count()) {
        int sum = 0;
        for (int i = 0; i < otherButtonIndexes.count(); i++) {
            sum += otherButtonIndexes[i] + 1;
        }
        indexOfCustom = (sum / otherButtonIndexes.count()) - 1; // indexOfCustom is now at the median index position of otherButtonIndexes
    } else {
        indexOfCustom = 0;
    }

    // Want to give Close/Maximize/Minimize buttons priority over the custom button to be at either the left or right edges
    QMap<int, KDecoration2::DecorationButtonType> leftEdgePriorityButtons; // a list of Close/Maximize/Minimize if at left edge
    QMap<int, KDecoration2::DecorationButtonType> rightEdgePriorityButtons; // a list of Close/Maximize/Minimize if at right edge

    // find leftEdgePriorityButtons
    for (int i = 0; i < 3; i++) {
        if (buttonsOnLeft.indexOf(QChar('X')) == i) {
            leftEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Close);
        } else if (buttonsOnLeft.indexOf(QChar('A')) == i) {
            leftEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Maximize);
        } else if (buttonsOnLeft.indexOf(QChar('I')) == i) {
            leftEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Minimize);
        }
    }

    // find rightEdgePrioritybuttons
    for (int i = m_visibleButtonsOrder.count() - 1; i >= m_visibleButtonsOrder.count() - 3; i--) {
        if (buttonsOnRight.lastIndexOf(QChar('X')) == i) { // lastIndexOf in-case a weirdo adds more than one button of the same type
            rightEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Close);
        } else if (buttonsOnRight.lastIndexOf(QChar('A')) == i) {
            rightEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Maximize);
        } else if (buttonsOnRight.lastIndexOf(QChar('I')) == i) {
            rightEdgePriorityButtons.insert(i, KDecoration2::DecorationButtonType::Minimize);
        }
    }

    // if custom is at an edge, make sure priority (min/max/close) button has a priority over custom button for the edge
    if (indexOfCustom >= 0 && indexOfCustom <= (leftEdgePriorityButtons.count() - 1)
        && leftEdgePriorityButtons.count()) { // if custom is to go at start but a leftEdgePriority button is there
        indexOfCustom = m_visibleButtonsOrder.indexOf(leftEdgePriorityButtons.value(leftEdgePriorityButtons.count() - 1)) + 1;
    } else if (indexOfCustom <= (m_visibleButtonsOrder.count() - 1) && indexOfCustom >= (m_visibleButtonsOrder.count() - rightEdgePriorityButtons.count())
               && rightEdgePriorityButtons.count()) { // if custom is to go at end but a right EdgePriority button is there
        indexOfCustom = m_visibleButtonsOrder.indexOf(rightEdgePriorityButtons.value(0));
    }

    m_allCustomizableButtonsOrder = m_visibleButtonsOrder + m_hiddenButtons;

    m_visibleButtonsOrder.insert(indexOfCustom,
                                 KDecoration2::DecorationButtonType::Custom); // dummy Custom button inserted for illustrating colour palettes in icons
}

QList<DecorationButtonPalette *> ButtonColors::sortButtonsAsPerKwinConfig(QList<DecorationButtonPalette *> inputlist)
{
    QList<DecorationButtonPalette *> outputlist;

    for (int i = 0; i < m_visibleButtonsOrder.count(); i++) {
        for (int j = inputlist.count() - 1; j >= 0; j--) { // iterate loop in reverse order as want to delete elements
            if (m_visibleButtonsOrder[i] == (inputlist[j])->buttonType()) {
                outputlist.append(inputlist[j]);
                inputlist.removeAt(j);
            }
        }
    }
    return outputlist;
}

void ButtonColors::loadHorizontalHeaderIcons()
{
    for (auto i = 0; i < m_allCustomizableButtonsOrder.count(); i++) { // set the horizontal header icons
        setHorizontalHeaderSectionIcon(m_allCustomizableButtonsOrder[i], m_ui->overrideColorTableActive, i);
        setHorizontalHeaderSectionIcon(m_allCustomizableButtonsOrder[i], m_ui->overrideColorTableInactive, i);
    }
}

void ButtonColors::updateLockIcons()
{
    m_ui->lockButtonColorsActive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockButtonColorsInactive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));

    int numRows = m_overridableButtonColorStatesStrings.count();
    for (int rowIndex = 0; rowIndex < numRows; rowIndex++) {
        auto verticalHeaderItemActive = m_ui->overrideColorTableActive->verticalHeaderItem(rowIndex);
        auto verticalHeaderItemInactive = m_ui->overrideColorTableInactive->verticalHeaderItem(rowIndex);
        if (verticalHeaderItemActive->checkState() == Qt::CheckState::Checked) {
            verticalHeaderItemActive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Locked));
        } else {
            verticalHeaderItemActive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Unlocked));
        }
        if (verticalHeaderItemInactive->checkState() == Qt::CheckState::Checked) {
            verticalHeaderItemInactive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Locked));
        } else {
            verticalHeaderItemInactive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Unlocked));
        }
    }
}

void ButtonColors::setHorizontalHeaderSectionIcon(KDecoration2::DecorationButtonType type, QTableWidget *table, int section)
{
    QIcon icon;

    if (type == DecorationButtonType::Menu) {
        icon = windowIcon();
    } else {
        bool renderSystemIcon = false;
        QString iconName;
        if (m_internalSettings->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme) {
            QString iconNameChecked;
            SystemIconTheme::systemIconNames(type, iconName, iconNameChecked);
            if (!iconName.isEmpty()) {
                renderSystemIcon = true;
            }
        }

        qreal dpr = devicePixelRatioF();
        QPixmap pixmap(16.0 * dpr, 16.0 * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);
        std::unique_ptr<QPainter> painter = std::make_unique<QPainter>(&pixmap);
        painter->setPen(QApplication::palette().windowText().color());
        painter->setRenderHints(QPainter::RenderHint::Antialiasing);

        if (renderSystemIcon) {
            SystemIconTheme systemIconRenderer(painter.get(), 16, iconName, m_internalSettings, dpr);
            systemIconRenderer.renderIcon();
        } else {
            auto [iconRenderer, localRenderingWidth](RenderDecorationButtonIcon::factory(m_internalSettings, painter.get(), true, true, dpr));

            int centringOffset = (localRenderingWidth - 16) / 2;
            painter->setViewport(0, 0, 16, 16);
            painter->setWindow(centringOffset, centringOffset, 16, 16);

            QPen pen = painter->pen();
            pen.setWidthF(PenWidth::Symbol * dpr);
            pen.setCosmetic(true);
            painter->setPen(pen);
            iconRenderer->renderIcon(type, false);
        }

        icon = QIcon(pixmap);
    }

    QTableWidgetItem *item = table->horizontalHeaderItem(section);
    if (!item)
        return;
    item->setIcon(icon);
}

bool ButtonColors::event(QEvent *ev)
{
    if (ev->type() == QEvent::ApplicationPaletteChange) {
        // overwrite handling of palette change
        if (!m_loaded)
            load();
        else {
            loadButtonPaletteColorsIcons();
            loadHorizontalHeaderIcons();
        }
        return QWidget::event(ev);
    }

    // TODO:detect icon change

    // Make sure the rest of events are handled
    return QWidget::event(ev);
}
}
